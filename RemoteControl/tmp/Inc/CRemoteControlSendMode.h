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

	//2. 클라이언트와의 통신
	int Communication();

	//N. Get
	SOCKET GetSocket() { return m_socket; }
	char* GetSendBuffer() { return m_sendBuffer; }
	char* GetRecvBuffer() { return m_recvBuffer; }
	HBITMAP GetBitMap() { return m_hBitmap; }

private:
	//1. 서버 시작 - 소켓 생성, 바인딩, 리슨
	//1.1 윈속 라이브러리 초기화
	bool InitWinsock();
	bool CreateSocket();
	bool ConnectServer(const char* ip, int port);
	bool PerformHandshake();
	
	//2. 클라이언트와의 통신
	//2.1 클라이언트로 데이터 전송
	//2.2 클라이언트로부터 데이터 수신

	//3. 통신 종료
	void CloseCommunication();



	//member
public:
	HBITMAP m_hBitmap;

private:
	//윈속
	WSADATA m_wsa;

	SOCKET m_socket;
	SOCKADDR_IN m_serverAddr;

	int m_port;

	//클라이언트와의 통신 버퍼
	char m_sendBuffer[1024];
	char m_recvBuffer[1024];

	//메시지 데이터
	Message m_message;

	uint32_t    m_myId;

	// 인증용
	char    m_authId[32];
	char    m_authPw[32];

};

bool recvn_sendMode(SOCKET sock, void* buf, size_t len);
int sendn_sendMode(SOCKET sock, const char* buffer, int totalBytes);

DWORD WINAPI SendData_SendMode(LPVOID lpParam);

DWORD WINAPI RecvData_SendMode(LPVOID lpParam);