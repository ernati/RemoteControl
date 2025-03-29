#pragma once
#include <Message.h>
#include <WinSock2.h>
#include <cstdio>
#pragma comment(lib, "ws2_32")

class CMultiThreadClient
{
	//method
public:
	CMultiThreadClient();
	virtual ~CMultiThreadClient();

	//1. Ŭ���̾�Ʈ ���� - ���� ����, Ŀ��Ʈ
	int StartClient(int port);

	//2. �������� ���
	int Communication();

	//Get
	SOCKET GetCommunicationSocket() { return m_CommunicationSocket; }
	char* GetSendBuffer() { return m_sendBuffer; }

	HBITMAP ReconstructBitmapFromMessage(const char* pMessageBuffer, DWORD messageSize);

private:
	//1. Ŭ���̾�Ʈ ���� - ���� ����, Ŀ��Ʈ
	//1.1 ���� ���̺귯�� �ʱ�ȭ
	bool InitWinsock();
	//1.2 ���� ����
	bool CreateCommunicationSocket();
	//1.3 ���� Ŀ��Ʈ
	bool ConnectServer();


public:
	HBITMAP m_hBitmap;

private:
	//member
	//����
	WSADATA m_wsa;

	//�������� ��� ����
	SOCKET m_CommunicationSocket;
	SOCKADDR_IN m_Serveraddr;
	int m_port;

	//�������� ��� ����
	char m_sendBuffer[102400];
	char m_recvBuffer[102400];

	//�޽��� ������
	Message message;

	//�������� ������ ������ ���� ������ �ڵ�
	HANDLE m_hSendThread;
	DWORD dwSendThreadID;

	////�����κ��� ������ ������ ���� ������ �ڵ�
	//HANDLE m_hRecvThread;
	//DWORD dwRecvThreadID;
};

DWORD WINAPI SendData(LPVOID lpParam);

