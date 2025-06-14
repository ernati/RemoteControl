#pragma once
#include <Message.h>
#include <cstdio>

class CRemoteControlRecvMode
{
	//method
public:
	CRemoteControlRecvMode();
	CRemoteControlRecvMode(uint32_t myId, uint32_t targetId);
	virtual ~CRemoteControlRecvMode();

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

	bool PerformHandshake();


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
	char m_sendBuffer[1024];
	char m_recvBuffer[1024];

	char m_BitMapBuffer[16000000];

	//�޽��� ������
	Message message;

	uint32_t m_myId;
	uint32_t m_targetId;
};

DWORD WINAPI SendData(LPVOID lpParam);

bool recvn(SOCKET sock, void* buf, size_t len);

void GetMutex();
void ReleaseMutex();

HBITMAP ReceiveBitmapMessage(SOCKET sock);

