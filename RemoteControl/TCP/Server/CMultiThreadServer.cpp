#pragma once

#include <CMultiThreadServer.h>

CMultiThreadServer::CMultiThreadServer()
{
	ZeroMemory(&m_wsa, sizeof(m_wsa));

	m_listenSocket = INVALID_SOCKET;
	ZeroMemory(&m_listenaddr, sizeof(m_listenaddr));

	m_CommunicationSocket = INVALID_SOCKET;
	ZeroMemory(&m_Clientaddr, sizeof(m_Clientaddr));
	m_port = 0;

	ZeroMemory(m_recvBuffer, sizeof(m_recvBuffer));
	ZeroMemory(m_sendBuffer, sizeof(m_sendBuffer));

	ZeroMemory(&message, sizeof(message));
	m_hBitmap = NULL;

	m_hSendThread = NULL;
	dwSendThreadID = 0;

	m_hRecvThread = NULL;
	dwRecvThreadID = 0;

}

CMultiThreadServer::~CMultiThreadServer()
{
	if (m_listenSocket != INVALID_SOCKET)
		closesocket(m_listenSocket);

	if (m_CommunicationSocket != INVALID_SOCKET)
		closesocket(m_CommunicationSocket);

	WSACleanup();
}



/*
*	public method
*/



int CMultiThreadServer::StartServer(int port)
{
	m_port = port;

	if (!InitWinsock())
	{
		printf("InitWinsock() failed\n");
		return -1;
	}

	if (!CreateListenSocket())
	{
		printf("CreateListenSocket() failed\n");
		return -2;
	}

	if (!BindSocket())
	{
		printf("BindSocket() failed\n");
		return -3;
	}

	if (!ListenSocket())
	{
		printf("ListenSocket() failed\n");
		return -4;
	}

	//클라이언트와의 통신
	Communication();

	return 0;
}

int CMultiThreadServer::Communication()
{
	int addrLen = sizeof(m_Clientaddr);

	while (INVALID_SOCKET != ( m_CommunicationSocket = accept(m_listenSocket, (SOCKADDR*)&m_Clientaddr, &addrLen)))
	{
		//클라이언트로의 데이터 전송을 맡을 스레드 생성
		m_hSendThread = ::CreateThread(
			NULL,
			0,
			SendData,
			(LPVOID)this,
			0,
			&dwSendThreadID
		);
		
		////클라이언트로부터 데이터 수신을 맡을 스레드 생성
		//m_hRecvThread = ::CreateThread(
		//	NULL,
		//	0,
		//	RecvData,
		//	(LPVOID)this,
		//	0,
		//	&dwRecvThreadID
		//);
	}

	return 0;
}

void CMultiThreadServer::CloseCommunication()
{
	if (m_hSendThread != NULL)
	{
		TerminateThread(m_hSendThread, 0);
		CloseHandle(m_hSendThread);
	}

	if (m_hRecvThread != NULL)
	{
		TerminateThread(m_hRecvThread, 0);
		CloseHandle(m_hRecvThread);
	}

	closesocket(m_CommunicationSocket);
}


/*
*	private method
*/



bool CMultiThreadServer::InitWinsock()
{
	if (0 != WSAStartup(MAKEWORD(2, 2), &m_wsa))
	{
		printf("WSAStartup() failed\n");
		return false;
	}

	return true;
}

bool CMultiThreadServer::CreateListenSocket()
{
	m_listenSocket = socket(AF_INET, SOCK_STREAM, 0); //TCP
	if (INVALID_SOCKET == m_listenSocket)
	{
		printf("socket() failed\n");
		return false;
	}

	return true;
}

bool CMultiThreadServer::BindSocket()
{
	m_listenaddr.sin_family = AF_INET;
	m_listenaddr.sin_port = htons(m_port);
	m_listenaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (SOCKET_ERROR == bind(m_listenSocket, (SOCKADDR*)&m_listenaddr, sizeof(m_listenaddr)))
	{
		printf("bind() failed\n");
		return false;
	}

	return true;
}

bool CMultiThreadServer::ListenSocket()
{

	if (SOCKET_ERROR == listen(m_listenSocket, SOMAXCONN))
	{
		printf("listen() failed\n");
		return false;
	}

	printf("[Server] Start Listening...\n");

	return true;
}


int sendn(SOCKET sock, const char* buffer, int totalBytes) {
	int totalSent = 0;
	while (totalSent < totalBytes) {
		int n = send(sock, buffer + totalSent, totalBytes - totalSent, 0);
		if (n == SOCKET_ERROR) {
			// 에러 처리: 필요하면 WSAGetLastError()로 에러 코드 확인
			return SOCKET_ERROR;
		}
		totalSent += n;
	}
	return totalSent;
}



DWORD WINAPI SendData(LPVOID lpParam)
{
	CMultiThreadServer* pServer = (CMultiThreadServer*)lpParam;
	SOCKET hSocket = pServer->GetCommunicationSocket();
	char* pSendBuffer = pServer->GetSendBuffer();
	char* pszTmp = NULL;

	printf("[Server] SendData Start!\n");

	DWORD dwSendSize = 0;
	
	while (1)
	{
		//1. Bitmap 데이터가 NULL이 아닐경우, 데이터를 전송한다.
		if (pServer->GetBitMap() != NULL) {
			//1.1 멤버에 담긴 BITMAP 데이터를 획득
			BITMAP BmpCaptured;
			if (!GetObject(pServer->GetBitMap(), sizeof(BmpCaptured), &BmpCaptured)) {
			    continue;
			}
			size_t bitsSize = static_cast<size_t>(BmpCaptured.bmHeight) * BmpCaptured.bmWidthBytes;
			uint8_t* bits = new uint8_t[bitsSize];
			if (GetBitmapBits(pServer->GetBitMap(), static_cast<LONG>(bitsSize), bits) == 0) {
			    delete[] bits;
			    continue;
			}
			
			//1.2 BITMAP의 메타 데이터가 담긴 Message 버퍼 생성
			size_t msgSize = 0;
			uint8_t* msgBuf = createMessageBuffer_tmp(BmpCaptured, bits, bitsSize, msgSize);
			Message* msg = reinterpret_cast<Message*>(msgBuf);

			//1.3 사용자가 입력한 문자열을 서버에 전송한다.
			sendn(hSocket, (const char*)msgBuf, msgSize);

			//1.4 메모리 해제
			delete[] bits;
			delete[] msgBuf;
		}

		Sleep(100);
	}

	return 0;
}

DWORD WINAPI RecvData(LPVOID lpParam)
{
	CMultiThreadServer* pServer = (CMultiThreadServer*)lpParam;
	SOCKET hSocket = pServer->GetCommunicationSocket();
	char* pRecvBuffer = pServer->GetRecvBuffer();

	printf("[Server] RecvData Start!\n");

	while (1)
	{
		//3.3 서버로부터 데이터를 수신한다.
		::recv(hSocket, pRecvBuffer, sizeof(pRecvBuffer), 0);
		if (strcmp(pRecvBuffer, "EXIT") == 0)	break;

		//3.4 서버로부터 수신한 데이터를 출력한다.
		printf("Client : %s\n", pRecvBuffer);
	}

	return 0;
}