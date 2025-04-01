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
	////�������� ������ ������ ���� ������ ����
	//m_hSendThread = ::CreateThread(
	//	NULL,
	//	0,
	//	SendData,
	//	(LPVOID)this,
	//	0,
	//	&dwSendThreadID
	//);

	// �����κ����� ������ ����
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
			printf("�������� ������ ���������ϴ�.\n");
			break;
		}

		//ReleaseMutex
		// mutex ����
		ReleaseMutex_Custom();

		printf("Recv BitMap Data!\n");

		//���� ������ ó��
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

	// Message ����ü�� �о����
	const Message* pMsg = reinterpret_cast<const Message*>(pMessageBuffer);
	if (messageSize < sizeof(Message) )
		return NULL;

	// BITMAPINFO ����ü ���� (��Ʈ�� ����� �ʿ�)
	BITMAPINFO bmi;
	ZeroMemory(&bmi, sizeof(BITMAPINFO));
	bmi.bmiHeader = pMsg->bmiHeader;

	// CreateDIBSection�� ����� �� ��Ʈ�� ����
	HDC hdcScreen = GetDC(NULL);
	void* pBits = nullptr;
	HBITMAP hNewBitmap = CreateDIBSection(hdcScreen, &bmi, DIB_RGB_COLORS, &pBits, NULL, 0);
	ReleaseDC(NULL, hdcScreen);

	if (!hNewBitmap || !pBits)
		return NULL;

	// Message ���ۿ��� �ȼ� �����͸� ����
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

