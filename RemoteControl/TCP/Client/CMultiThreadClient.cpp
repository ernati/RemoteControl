#pragma once

#include <CMultiThreadClient.h>

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
	////서버로의 데이터 전송을 맡을 스레드 생성
	//m_hSendThread = ::CreateThread(
	//	NULL,
	//	0,
	//	SendData,
	//	(LPVOID)this,
	//	0,
	//	&dwSendThreadID
	//);

	// 서버로부터의 데이터 수신
	printf("[Client] RecvData Start!\n");
	int recvByte = 0;
	while (true)
	{
		GetMutex();
		
		recvByte = recv(m_CommunicationSocket, m_recvBuffer, sizeof(m_recvBuffer), 0);
		if (recvByte == SOCKET_ERROR)
		{
			printf("recv() failed\n");
			break;
		}
		else if (recvByte == 0)
		{
			printf("서버와의 연결이 끊어졌습니다.\n");
			break;
		}

		//ReleaseMutex
		// mutex 해제
		ReleaseMutex_Custom();

		printf("Recv BitMap Data!\n");

		//받은 데이터 처리
		m_hBitmap = ReconstructBitmapFromMessage(m_recvBuffer, recvByte);
	}

	return 0;
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


HBITMAP CMultiThreadClient::ReconstructBitmapFromMessage(const char* pMessageBuffer, DWORD messageSize)
{
	if (!pMessageBuffer || messageSize < sizeof(Message))
		return NULL;

	// Message 구조체를 읽어들임
	const Message* pMsg = reinterpret_cast<const Message*>(pMessageBuffer);
	if (messageSize < sizeof(Message) )
		return NULL;

	// BITMAPINFO 구조체 생성 (비트맵 헤더만 필요)
	BITMAPINFO bmi;
	ZeroMemory(&bmi, sizeof(BITMAPINFO));
	bmi.bmiHeader = pMsg->bmiHeader;

	// CreateDIBSection을 사용해 새 비트맵 생성
	HDC hdcScreen = GetDC(NULL);
	void* pBits = nullptr;
	HBITMAP hNewBitmap = CreateDIBSection(hdcScreen, &bmi, DIB_RGB_COLORS, &pBits, NULL, 0);
	ReleaseDC(NULL, hdcScreen);

	if (!hNewBitmap || !pBits)
		return NULL;

	// Message 버퍼에서 픽셀 데이터를 복사
	memcpy(pBits, pMessageBuffer + SIZE_OF_BITMAPINFOHEADER + SIZE_OF_DWORD, pMsg->pixelDataSize);

	return hNewBitmap;
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

