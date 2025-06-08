#pragma once

#include <CMultiThreadClient.h>
#include <vector>

CMultiThreadClient::CMultiThreadClient()
{
	ZeroMemory(&m_wsa, sizeof(m_wsa));

	m_CommunicationSocket = INVALID_SOCKET;
	ZeroMemory(&m_Serveraddr, sizeof(m_Serveraddr));
	ZeroMemory(&message, sizeof(message));
	m_port = 0;

	m_hBitmap = NULL;

	hMutex = CreateMutex(NULL, FALSE, L"Global\\MyGlobalMutex");
	if (hMutex == NULL) {
		printf("CreateMutex 실패, 오류 코드 : %d\n", GetLastError());
		return;
	}
	printf("전역 mutex가 생성되었습니다.\n");
}

CMultiThreadClient::~CMultiThreadClient()
{
	if (m_CommunicationSocket != INVALID_SOCKET)
		closesocket(m_CommunicationSocket);

	WSACleanup();
}

/*
* public method
*/
int CMultiThreadClient::StartClient(int port)
{
	m_port = port;

	if (!InitWinsock())
	{
		printf("InitWinsock() failed\n");
		return -1;
	}

	if (!CreateCommunicationSocket())
	{
		printf("CreateCommunicationSocket() failed\n");
		return -2;
	}

	if (!ConnectServer())
	{
		printf("ConnectServer() failed\n");
		return -3;
	}

	//통신
	Communication();

	return 0;
}

int CMultiThreadClient::Communication()
{
	printf("[Client] RecvData Start!\n");

	while (true)
	{
		// 1) ReceiveBitmapMessage 내부에서 헤더+페이로드 수신→HBITMAP 복원
		HBITMAP hBmp = ReceiveBitmapMessage(m_CommunicationSocket);
		if (!hBmp) {
			break;
		}

		//// 동기화
		GetMutex();

		// 복원된 HBITMAP을 공유 변수에 저장
		m_hBitmap = hBmp;

		ReleaseMutex_Custom();
	}

	return 0;
}

// sock로부터 정확히 len 바이트를 받을 때까지 반복 호출
bool recvn(SOCKET sock, void* buf, size_t len) {
	uint8_t* ptr = reinterpret_cast<uint8_t*>(buf);
	size_t   received = 0;
	while (received < len) {
		int r = recv(sock, reinterpret_cast<char*>(ptr + received), int(len - received), 0);
		if (r <= 0) return false;  // 에러 혹은 연결 종료
		received += r;
	}
	return true;
}



/*
* private method
*/
bool CMultiThreadClient::InitWinsock()
{
	if (WSAStartup(MAKEWORD(2, 2), &m_wsa) != 0)
	{
		printf("WSAStartup() failed\n");
		return false;
	}

	return true;
}

bool CMultiThreadClient::CreateCommunicationSocket()
{
	m_CommunicationSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_CommunicationSocket == INVALID_SOCKET)
	{
		printf("socket() failed\n");
		return false;
	}

	return true;
}

bool CMultiThreadClient::ConnectServer()
{
	m_Serveraddr.sin_family = AF_INET;
	m_Serveraddr.sin_port = htons(m_port);
	m_Serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	if (connect(m_CommunicationSocket, (SOCKADDR*)&m_Serveraddr, sizeof(m_Serveraddr)) == SOCKET_ERROR)
	{
		printf("connect() failed\n");
		return false;
	}
	return true;
}


HBITMAP ReceiveBitmapMessage(SOCKET sock) {
	// 1) 헤더만큼 먼저 받기 (payload[1] 제외)
	const size_t headerSize = sizeof(Message) - sizeof(uint8_t);
	std::vector<uint8_t> headerBuf(headerSize);
	if (!recvn(sock, headerBuf.data(), headerSize)) {
		return NULL;
	}

	// 2) payloadSize 파싱
	Message* hdr = reinterpret_cast<Message*>(headerBuf.data());
	if (ntohl(hdr->magic) != 0x424D4350) {
		return NULL;
	}
	uint64_t payloadSize = ntohll(hdr->payloadSize);

	// 3) 헤더 + 페이로드 전체 버퍼 할당 & 헤더 복사
	size_t totalSize = headerSize + payloadSize;
	uint8_t* fullBuf = new uint8_t[totalSize];
	memcpy(fullBuf, headerBuf.data(), headerSize);

	// 4) 나머지 페이로드 수신
	if (!recvn(sock, fullBuf + headerSize, payloadSize)) {
		delete[] fullBuf;
		return NULL;
	}

	// 5) 체크섬 검증
	// recv된 checksum 추출
	uint32_t recvCrc;
	std::memcpy(&recvCrc, fullBuf + offsetof(Message, checksum), sizeof(uint32_t));
	recvCrc = ntohl(recvCrc);

	// checksum 필드 0으로 클리어
	std::memset(fullBuf + offsetof(Message, checksum), 0, sizeof(uint32_t));

	// 계산 대상: magic 직후부터 끝까지
	uint8_t* csStart = fullBuf + sizeof(uint32_t);
	size_t   csLen = totalSize - sizeof(uint32_t);
	uint32_t calcCrc = calculateCRC32(csStart, csLen);

	if (recvCrc != calcCrc) {
		delete[] fullBuf;
		return NULL;
	}

	// 6) 헤더 정보 꺼내기
	Message* msg = reinterpret_cast<Message*>(fullBuf);
	uint32_t width = ntohl(msg->width);
	uint32_t height = ntohl(msg->height);
	uint16_t bitCount = ntohs(msg->bitCount);
	uint32_t scanLine = ntohl(msg->widthBytes);
	uint8_t* pixels = msg->payload;

	// 7) DIBSection으로 HBITMAP 생성
	BITMAPINFO bmi;
	ZeroMemory(&bmi, sizeof(bmi));
	BITMAPINFOHEADER& bih = bmi.bmiHeader;
	bih.biSize = sizeof(bih);
	bih.biWidth = width;
	bih.biHeight = -static_cast<LONG>(height); // top-down
	bih.biPlanes = 1;
	bih.biBitCount = bitCount;
	bih.biCompression = BI_RGB;

	void* dibBits = nullptr;
	HDC    hdc = GetDC(NULL);
	HBITMAP hBmp = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &dibBits, NULL, 0);
	ReleaseDC(NULL, hdc);

	if (hBmp && dibBits) {
		// 픽셀 데이터 복사
		std::memcpy(dibBits, pixels, payloadSize);
	}
	else {
	}

	delete[] fullBuf;
	return hBmp;
}



DWORD WINAPI SendData(LPVOID lpParam)
{
	CMultiThreadClient* pClient = (CMultiThreadClient*)lpParam;
	SOCKET hSocket = pClient->GetCommunicationSocket();
	char* sendBuffer = pClient->GetSendBuffer();

	printf("[Client] SendData Start!\n");

	while (true)
	{
		ZeroMemory(sendBuffer, sizeof(sendBuffer));
		printf("보낼 데이터: ");
		gets_s(sendBuffer, sizeof(sendBuffer));

		if (send(hSocket, sendBuffer, strlen(sendBuffer), 0) == SOCKET_ERROR)
		{
			printf("send() failed\n");
			break;
		}
	}

	return 0;
}



void CMultiThreadClient::GetMutex()
{
	DWORD dwWaitResult = WaitForSingleObject(hMutex, INFINITE);
	switch (dwWaitResult) {
	case WAIT_OBJECT_0:
		printf("mutex 소유권 획득 성공. 임계 구역에 진입합니다.\n");
		// 여기서 임계 구역 내 작업 수행
		// 예제: 2초 동안 대기 (임계 구역 내 작업을 시뮬레이션)
		Sleep(2000);
		break;
	case WAIT_ABANDONED:
		printf("mutex가 포기되었습니다.\n");
		break;
	default:
		printf("WaitForSingleObject 실패, 오류 코드: %d\n", GetLastError());
		break;
	}
}

void CMultiThreadClient::ReleaseMutex_Custom()
{
	if (!ReleaseMutex(hMutex)) {
		printf("ReleaseMutex 실패, 오류 코드: %d\n", GetLastError());
	}
	else {
		printf("mutex가 해제되었습니다.\n");
	}
}

