#pragma once
#include <Message.h>
#include <WinSock2.h>
#include <cstdio>
#pragma comment(lib, "ws2_32")



class CMultiThreadServer
{
	//method
public:
	CMultiThreadServer();
	virtual ~CMultiThreadServer();

	//1. ���� ���� - ���� ����, ���ε�, ����
	int StartServer(int port);

	//2. Ŭ���̾�Ʈ���� ���
	int Communication();

	//N. Get
	SOCKET GetCommunicationSocket() { return m_CommunicationSocket; }
	char* GetSendBuffer() { return m_sendBuffer; }
	char* GetRecvBuffer() { return m_recvBuffer; }
	HBITMAP GetBitMap() { return m_hBitmap; }

	char* CreateBitmapMessage(HBITMAP hBitmap, DWORD& outMessageSize);

private:
	//1. ���� ���� - ���� ����, ���ε�, ����
	//1.1 ���� ���̺귯�� �ʱ�ȭ
	bool InitWinsock();
	//1.2 ���� ����
	bool CreateListenSocket();
	//1.3 ���� ���ε�
	bool BindSocket();
	//1.4 ���� ����
	bool ListenSocket();

	
	//2. Ŭ���̾�Ʈ���� ���
	//2.1 Ŭ���̾�Ʈ�� ������ ����
	//2.2 Ŭ���̾�Ʈ�κ��� ������ ����

	//3. ��� ����
	void CloseCommunication();


	//member
public:
	HBITMAP m_hBitmap;

private:
	//����
	WSADATA m_wsa;

	//���� ���� ����
	SOCKET m_listenSocket;
	SOCKADDR_IN m_listenaddr;

	//Ŭ���̾�Ʈ���� ��� ����
	SOCKET m_CommunicationSocket;
	SOCKADDR_IN m_Clientaddr;
	int m_port;

	//Ŭ���̾�Ʈ���� ��� ����
	char m_sendBuffer[102400];
	char m_recvBuffer[102400];

	//�޽��� ������
	Message message;

	//Ŭ���̾�Ʈ���� ������ ������ ���� ������ �ڵ�
	HANDLE m_hSendThread;
	DWORD dwSendThreadID;

	//Ŭ���̾�Ʈ�κ��� ������ ������ ���� ������ �ڵ�
	HANDLE m_hRecvThread;
	DWORD dwRecvThreadID;

};


DWORD WINAPI SendData(LPVOID lpParam);

DWORD WINAPI RecvData(LPVOID lpParam);