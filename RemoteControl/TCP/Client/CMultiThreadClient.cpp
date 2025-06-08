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
		printf("CreateMutex ����, ���� �ڵ� : %d\n", GetLastError());
		return;
	}
	printf("���� mutex�� �����Ǿ����ϴ�.\n");
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

	//���
	Communication();

	return 0;
}

int CMultiThreadClient::Communication()
{
	printf("[Client] RecvData Start!\n");

	while (true)
	{
		// 1) ReceiveBitmapMessage ���ο��� ���+���̷ε� ���š�HBITMAP ����
		HBITMAP hBmp = ReceiveBitmapMessage(m_CommunicationSocket);
		if (!hBmp) {
			break;
		}

		//// ����ȭ
		GetMutex();

		// ������ HBITMAP�� ���� ������ ����
		m_hBitmap = hBmp;

		ReleaseMutex_Custom();
	}

	return 0;
}

// sock�κ��� ��Ȯ�� len ����Ʈ�� ���� ������ �ݺ� ȣ��
bool recvn(SOCKET sock, void* buf, size_t len) {
	uint8_t* ptr = reinterpret_cast<uint8_t*>(buf);
	size_t   received = 0;
	while (received < len) {
		int r = recv(sock, reinterpret_cast<char*>(ptr + received), int(len - received), 0);
		if (r <= 0) return false;  // ���� Ȥ�� ���� ����
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
	// 1) �����ŭ ���� �ޱ� (payload[1] ����)
	const size_t headerSize = sizeof(Message) - sizeof(uint8_t);
	std::vector<uint8_t> headerBuf(headerSize);
	if (!recvn(sock, headerBuf.data(), headerSize)) {
		return NULL;
	}

	// 2) payloadSize �Ľ�
	Message* hdr = reinterpret_cast<Message*>(headerBuf.data());
	if (ntohl(hdr->magic) != 0x424D4350) {
		return NULL;
	}
	uint64_t payloadSize = ntohll(hdr->payloadSize);

	// 3) ��� + ���̷ε� ��ü ���� �Ҵ� & ��� ����
	size_t totalSize = headerSize + payloadSize;
	uint8_t* fullBuf = new uint8_t[totalSize];
	memcpy(fullBuf, headerBuf.data(), headerSize);

	// 4) ������ ���̷ε� ����
	if (!recvn(sock, fullBuf + headerSize, payloadSize)) {
		delete[] fullBuf;
		return NULL;
	}

	// 5) üũ�� ����
	// recv�� checksum ����
	uint32_t recvCrc;
	std::memcpy(&recvCrc, fullBuf + offsetof(Message, checksum), sizeof(uint32_t));
	recvCrc = ntohl(recvCrc);

	// checksum �ʵ� 0���� Ŭ����
	std::memset(fullBuf + offsetof(Message, checksum), 0, sizeof(uint32_t));

	// ��� ���: magic ���ĺ��� ������
	uint8_t* csStart = fullBuf + sizeof(uint32_t);
	size_t   csLen = totalSize - sizeof(uint32_t);
	uint32_t calcCrc = calculateCRC32(csStart, csLen);

	if (recvCrc != calcCrc) {
		delete[] fullBuf;
		return NULL;
	}

	// 6) ��� ���� ������
	Message* msg = reinterpret_cast<Message*>(fullBuf);
	uint32_t width = ntohl(msg->width);
	uint32_t height = ntohl(msg->height);
	uint16_t bitCount = ntohs(msg->bitCount);
	uint32_t scanLine = ntohl(msg->widthBytes);
	uint8_t* pixels = msg->payload;

	// 7) DIBSection���� HBITMAP ����
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
		// �ȼ� ������ ����
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
		printf("���� ������: ");
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
		printf("mutex ������ ȹ�� ����. �Ӱ� ������ �����մϴ�.\n");
		// ���⼭ �Ӱ� ���� �� �۾� ����
		// ����: 2�� ���� ��� (�Ӱ� ���� �� �۾��� �ùķ��̼�)
		Sleep(2000);
		break;
	case WAIT_ABANDONED:
		printf("mutex�� ����Ǿ����ϴ�.\n");
		break;
	default:
		printf("WaitForSingleObject ����, ���� �ڵ�: %d\n", GetLastError());
		break;
	}
}

void CMultiThreadClient::ReleaseMutex_Custom()
{
	if (!ReleaseMutex(hMutex)) {
		printf("ReleaseMutex ����, ���� �ڵ�: %d\n", GetLastError());
	}
	else {
		printf("mutex�� �����Ǿ����ϴ�.\n");
	}
}

