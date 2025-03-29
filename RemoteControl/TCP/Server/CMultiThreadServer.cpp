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

	//Ŭ���̾�Ʈ���� ���
	Communication();

	return 0;
}

int CMultiThreadServer::Communication()
{
	int addrLen = sizeof(m_Clientaddr);

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
		
		////Ŭ���̾�Ʈ�κ��� ������ ������ ���� ������ ����
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


char* CMultiThreadServer::CreateBitmapMessage(HBITMAP hBitmap, DWORD& outMessageSize)
{
	if (!hBitmap)
		return nullptr;

	// ĸ�ĵ� ��Ʈ���� �⺻ ������ ����ϴ�.
	BITMAP bm = { 0 };
	if (GetObject(hBitmap, sizeof(bm), &bm) == 0)
		return nullptr;

	// BITMAPINFOHEADER�� �����մϴ�.
	BITMAPINFOHEADER bih = { 0 };
	bih.biSize = sizeof(BITMAPINFOHEADER);
	bih.biWidth = bm.bmWidth;
	bih.biHeight = bm.bmHeight;  // �ϴܿ��� ���� ������ �ƴ϶�� �ʿ信 ���� ���� ����
	bih.biPlanes = 1;
	bih.biBitCount = bm.bmBitsPixel;  // ��: 24 �Ǵ� 32
	bih.biCompression = BI_RGB;
	bih.biSizeImage = 0;  // BI_RGB�� ��� 0�� �� �� �����Ƿ� �Ʒ����� ���

	// ����, GetDIBits�� ȣ���Ͽ� �ʿ��� �̹��� ������ ũ�⸦ ����
	HDC hdcScreen = GetDC(NULL);
	if (0 == GetDIBits(hdcScreen, hBitmap, 0, bm.bmHeight, NULL, reinterpret_cast<BITMAPINFO*>(&bih), DIB_RGB_COLORS))
	{
		ReleaseDC(NULL, hdcScreen);
		return nullptr;
	}
	ReleaseDC(NULL, hdcScreen);

	// biSizeImage�� 0�̸� ���� ��� (�� ��ĵ������ 4����Ʈ ��迡 ������ ����)
	DWORD bytesPerLine = ((bm.bmWidth * bih.biBitCount + 31) / 32) * 4;
	if (bih.biSizeImage == 0)
		bih.biSizeImage = bytesPerLine * bm.bmHeight;

	DWORD dataSize = bih.biSizeImage;

	// �ȼ� �����͸� ���� ���۸� �Ҵ�
	char* pPixelBuffer = new char[dataSize];
	if (!pPixelBuffer)
		return nullptr;

	hdcScreen = GetDC(NULL);
	if (0 == GetDIBits(hdcScreen, hBitmap, 0, bm.bmHeight, pPixelBuffer, reinterpret_cast<BITMAPINFO*>(&bih), DIB_RGB_COLORS))
	{
		ReleaseDC(NULL, hdcScreen);
		delete[] pPixelBuffer;
		return nullptr;
	}
	ReleaseDC(NULL, hdcScreen);

	// Message ���� ��ü ũ��: Message ����ü ũ�� + �ȼ� ������ ũ��
	outMessageSize = sizeof(Message) + dataSize;

	// Message ����ü�� ä��
	ZeroMemory(m_sendBuffer, sizeof(m_sendBuffer));
	Message* pMsg = reinterpret_cast<Message*>(m_sendBuffer);
	pMsg->bmiHeader = bih;
	pMsg->dataSize = dataSize;

	// �ȼ� �����͸� Message ����ü �ڿ� ����
	memcpy(m_sendBuffer + sizeof(Message), pPixelBuffer, dataSize);

	delete[] pPixelBuffer;
	return m_sendBuffer;
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
		////3.1 ����ڷκ��� ���ڿ��� �Է¹޴´�.
		//printf("���� ������: ");
		//scanf_s("%s", pSendBuffer);
		//if (strcmp(pSendBuffer, "EXIT") == 0)	break;

		//Bitmap �����Ͱ� NULL�� �ƴҰ��, ������ �����͸� ���ۿ� ��´�.
		if (pServer->GetBitMap() != NULL) {

			pServer->CreateBitmapMessage(pServer->GetBitMap(), dwSendSize);

			printf("Send BitMap Data!\n");

			//3.2 ����ڰ� �Է��� ���ڿ��� ������ �����Ѵ�.
			::send(hSocket, pSendBuffer, strlen(pSendBuffer) + 1, 0);
		}

		
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
		//3.3 �����κ��� �����͸� �����Ѵ�.
		::recv(hSocket, pRecvBuffer, sizeof(pRecvBuffer), 0);
		if (strcmp(pRecvBuffer, "EXIT") == 0)	break;

		//3.4 �����κ��� ������ �����͸� ����Ѵ�.
		printf("Client : %s\n", pRecvBuffer);
	}

	return 0;
}