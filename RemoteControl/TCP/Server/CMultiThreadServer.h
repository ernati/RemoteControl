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

	//1. 서버 시작 - 소켓 생성, 바인딩, 리슨
	int StartServer(int port);

	//2. 클라이언트와의 통신
	int Communication();

	//N. Get
	SOCKET GetCommunicationSocket() { return m_CommunicationSocket; }
	char* GetSendBuffer() { return m_sendBuffer; }
	char* GetRecvBuffer() { return m_recvBuffer; }
	HBITMAP GetBitMap() { return m_hBitmap; }

	char* CreateBitmapMessage(HBITMAP hBitmap, DWORD& outMessageSize);

private:
	//1. 서버 시작 - 소켓 생성, 바인딩, 리슨
	//1.1 윈속 라이브러리 초기화
	bool InitWinsock();
	//1.2 소켓 생성
	bool CreateListenSocket();
	//1.3 소켓 바인딩
	bool BindSocket();
	//1.4 소켓 리슨
	bool ListenSocket();

	
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

	//서버 연결 소켓
	SOCKET m_listenSocket;
	SOCKADDR_IN m_listenaddr;

	//클라이언트와의 통신 소켓
	SOCKET m_CommunicationSocket;
	SOCKADDR_IN m_Clientaddr;
	int m_port;

	//클라이언트와의 통신 버퍼
	char m_sendBuffer[102400];
	char m_recvBuffer[102400];

	//메시지 데이터
	Message message;

	//클라이언트로의 데이터 전송을 맡을 스레드 핸들
	HANDLE m_hSendThread;
	DWORD dwSendThreadID;

	//클라이언트로부터 데이터 수신을 맡을 스레드 핸들
	HANDLE m_hRecvThread;
	DWORD dwRecvThreadID;

};


DWORD WINAPI SendData(LPVOID lpParam);

DWORD WINAPI RecvData(LPVOID lpParam);