#pragma once

#include <CSingleThreadClient.h>

CSingleThreadClient::CSingleThreadClient()
{
	ZeroMemory(&m_wsa, sizeof(m_wsa));

	m_CommunicationSocket = INVALID_SOCKET;
	ZeroMemory(&m_Serveraddr, sizeof(m_Serveraddr));
	m_port = 0;
}

CSingleThreadClient::~CSingleThreadClient()
{
	if (m_CommunicationSocket != INVALID_SOCKET)
		closesocket(m_CommunicationSocket);

	WSACleanup();
}

/*
* public method
*/
int CSingleThreadClient::StartClient(int port)
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

int CSingleThreadClient::Communication()
{
	char sendBuffer[1024];
	char recvBuffer[1024];

	//서버로의 데이터 전송을 맡을 스레드 생성
	m_hSendThread = ::CreateThread(
		NULL,
		0,
		SendData,
		(LPVOID)this,
		0,
		&dwSendThreadID
	);

	// 서버로부터의 데이터 수신
	printf("[Client] RecvData Start!\n");
	int recvByte = 0;
	while (true)
	{
		recvByte = recv(m_CommunicationSocket, recvBuffer, sizeof(recvBuffer), 0);
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

		printf("서버로부터 받은 데이터: %s\n", recvBuffer);
	}

	return 0;
}


/*
* private method
*/
bool CSingleThreadClient::InitWinsock()
{
	if (WSAStartup(MAKEWORD(2, 2), &m_wsa) != 0)
	{
		printf("WSAStartup() failed\n");
		return false;
	}

	return true;
}

bool CSingleThreadClient::CreateCommunicationSocket()
{
	m_CommunicationSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_CommunicationSocket == INVALID_SOCKET)
	{
		printf("socket() failed\n");
		return false;
	}

	return true;
}

bool CSingleThreadClient::ConnectServer()
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

DWORD WINAPI SendData(LPVOID lpParam)
{
	CSingleThreadClient* pClient = (CSingleThreadClient*)lpParam;
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