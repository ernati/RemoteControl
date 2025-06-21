#pragma once
#include <WinSock2.h>
#include <cstdio>
#pragma comment(lib, "ws2_32")

#include "Message.h"
#include "CommonAPI.h"

class CRemoteControlSendMode
{
	//method
public:
	CRemoteControlSendMode();
	CRemoteControlSendMode(const char* authId,
		const char* authPw,
		uint32_t myId);
	virtual ~CRemoteControlSendMode();

	// 1) Initialize, connect, handshake
	int StartClient(int port, const char* serverIp = "127.0.0.1");

	//2. Ŭ���̾�Ʈ���� ���
	int Communication();

	//N. Get
	SOCKET GetSocket() { return m_socket; }
	char* GetSendBuffer() { return m_sendBuffer; }
	char* GetRecvBuffer() { return m_recvBuffer; }
	HBITMAP GetBitMap() { return m_hBitmap; }

private:
	//1. ���� ���� - ���� ����, ���ε�, ����
	//1.1 ���� ���̺귯�� �ʱ�ȭ
	bool InitWinsock();
	bool CreateSocket();
	bool ConnectServer(const char* ip, int port);
	bool PerformHandshake();
	
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

	SOCKET m_socket;
	SOCKADDR_IN m_serverAddr;

	int m_port;

	//Ŭ���̾�Ʈ���� ��� ����
	char m_sendBuffer[1024];
	char m_recvBuffer[1024];

	//�޽��� ������
	Message m_message;

	uint32_t    m_myId;

	// ������
	char    m_authId[32];
	char    m_authPw[32];

};

bool recvn_sendMode(SOCKET sock, void* buf, size_t len);
int sendn_sendMode(SOCKET sock, const char* buffer, int totalBytes);

DWORD WINAPI SendData_SendMode(LPVOID lpParam);

DWORD WINAPI RecvData_SendMode(LPVOID lpParam);