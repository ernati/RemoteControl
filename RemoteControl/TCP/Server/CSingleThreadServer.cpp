#pragma once

#include <CSingleThreadServer.h>

CSingleThreadServer::CSingleThreadServer()
{
	ZeroMemory(&m_wsa, sizeof(m_wsa));

	m_listenSocket = INVALID_SOCKET;
	ZeroMemory(&m_listenaddr, sizeof(m_listenaddr));

	m_CommunicationSocket = INVALID_SOCKET;
	ZeroMemory(&m_Clientaddr, sizeof(m_Clientaddr));
	m_port = 0;

	ZeroMemory(m_recvBuffer, sizeof(m_recvBuffer));
	ZeroMemory(m_sendBuffer, sizeof(m_sendBuffer));

	m_hSendThread = NULL;
	dwSendThreadID = 0;

	m_hRecvThread = NULL;
	dwRecvThreadID = 0;
}

CSingleThreadServer::~CSingleThreadServer()
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



int CSingleThreadServer::StartServer(int port)
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

	//Ŭ���̾�Ʈ���� ���
	Communication();

	return 0;
}

int CSingleThreadServer::Communication()
{
	int addrLen = sizeof(m_Clientaddr);
	m_CommunicationSocket = accept(m_listenSocket, (SOCKADDR*)&m_Clientaddr, &addrLen);
	if (INVALID_SOCKET == m_CommunicationSocket)
	{
		printf("accept() failed\n");
		return -5;
	}

	while (INVALID_SOCKET != ( m_CommunicationSocket = accept(m_listenSocket, (SOCKADDR*)&m_Clientaddr, &addrLen)))
	{
		//Ŭ���̾�Ʈ���� ������ ������ ���� ������ ����
		m_hSendThread = ::CreateThread(
			NULL,
			0,
			SendData,
			(LPVOID)this,
			0,
			&dwSendThreadID
		);
		
		//Ŭ���̾�Ʈ�κ��� ������ ������ ���� ������ ����
		m_hRecvThread = ::CreateThread(
			NULL,
			0,
			RecvData,
			(LPVOID)this,
			0,
			&dwRecvThreadID
		);
	}

	return 0;
}

void CSingleThreadServer::CloseCommunication()
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


bool CSingleThreadServer::InitWinsock()
{
	if (0 != WSAStartup(MAKEWORD(2, 2), &m_wsa))
	{
		printf("WSAStartup() failed\n");
		return false;
	}

	return true;
}

bool CSingleThreadServer::CreateListenSocket()
{
	m_listenSocket = socket(AF_INET, SOCK_STREAM, 0); //TCP
	if (INVALID_SOCKET == m_listenSocket)
	{
		printf("socket() failed\n");
		return false;
	}

	return true;
}

bool CSingleThreadServer::BindSocket()
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

bool CSingleThreadServer::ListenSocket()
{
	if (SOCKET_ERROR == listen(m_listenSocket, SOMAXCONN))
	{
		printf("listen() failed\n");
		return false;
	}

	return true;
}


DWORD WINAPI SendData(LPVOID lpParam)
{
	CSingleThreadServer* pServer = (CSingleThreadServer*)lpParam;
	SOCKET hSocket = pServer->GetCommunicationSocket();
	char* pSendBuffer = pServer->GetSendBuffer();

	printf("[Server] SendData Start!\n");
	
	while (1)
	{
		//3.1 ����ڷκ��� ���ڿ��� �Է¹޴´�.
		printf("���� ������: ");
		scanf_s("%s", pSendBuffer);
		if (strcmp(pSendBuffer, "EXIT") == 0)	break;

		//3.2 ����ڰ� �Է��� ���ڿ��� ������ �����Ѵ�.
		::send(hSocket, pSendBuffer, strlen(pSendBuffer) + 1, 0);
	}

	return 0;
}

DWORD WINAPI RecvData(LPVOID lpParam)
{
	CSingleThreadServer* pServer = (CSingleThreadServer*)lpParam;
	SOCKET hSocket = pServer->GetCommunicationSocket();
	char* pRecvBuffer = pServer->GetRecvBuffer();

	printf("[Server] RecvData Start!\n");

	while (1)
	{
		//3.3 �����κ��� �����͸� �����Ѵ�.
		::recv(hSocket, pRecvBuffer, sizeof(pRecvBuffer), 0);
		if (strcmp(pRecvBuffer, "EXIT") == 0)	break;

		//3.4 �����κ��� ������ �����͸� ����Ѵ�.
		printf("Client : %s\n", pRecvBuffer);
	}

	return 0;
}