#pragma once
#include <Message.h>
#include <cstdio>

class CRemoteControlRecvMode
{
	//method
public:
	CRemoteControlRecvMode();
	// 새 생성자: authId, authPw, myId, targetId
	CRemoteControlRecvMode(const char* authId,
		const char* authPw,
		uint32_t myId,
		uint32_t targetId);
	virtual ~CRemoteControlRecvMode();

	//1. 클라이언트 시작 - 소켓 생성, 커넥트
	int StartClient(int port, const char* serverIp = "127.0.0.1");

	//2. 서버와의 통신
	int Communication();

	//Get
	SOCKET GetCommunicationSocket() { return m_CommunicationSocket; }
	char* GetSendBuffer() { return m_sendBuffer; }

	HBITMAP ReconstructBitmapFromMessage(const char* pMessageBuffer, DWORD messageSize);

private:
	//1. 클라이언트 시작 - 소켓 생성, 커넥트
	//1.1 윈속 라이브러리 초기화
	bool InitWinsock();
	//1.2 소켓 생성
	bool CreateCommunicationSocket();
	//1.3 서버 커넥트
	bool ConnectServer(const char* ip, int port);

	bool PerformHandshake();


public:
	HBITMAP m_hBitmap;

private:
	//member
	//윈속
	WSADATA m_wsa;

	//서버와의 통신 소켓
	SOCKET m_CommunicationSocket;
	SOCKADDR_IN m_Serveraddr;

	//서버와의 통신 버퍼
	char m_sendBuffer[1024];
	char m_recvBuffer[1024];

	char m_BitMapBuffer[16000000];

	//메시지 데이터
	Message message;

	uint32_t m_myId;
	uint32_t m_targetId;

	char       m_authId[32];
	char       m_authPw[32];
};

DWORD WINAPI SendData(LPVOID lpParam);

bool recvn(SOCKET sock, void* buf, size_t len);

void GetMutex();
void ReleaseMutex();

HBITMAP ReceiveBitmapMessage(SOCKET sock);

