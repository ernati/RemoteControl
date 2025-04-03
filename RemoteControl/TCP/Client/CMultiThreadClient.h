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

	//1. 클라이언트 시작 - 소켓 생성, 커넥트
	int StartClient(int port);

	//2. 서버와의 통신
	int Communication();

	//Get
	SOCKET GetCommunicationSocket() { return m_CommunicationSocket; }
	char* GetSendBuffer() { return m_sendBuffer; }

	HBITMAP ReconstructBitmapFromMessage(const char* pMessageBuffer, DWORD messageSize);

	void GetMutex();
	void ReleaseMutex_Custom();

private:
	//1. 클라이언트 시작 - 소켓 생성, 커넥트
	//1.1 윈속 라이브러리 초기화
	bool InitWinsock();
	//1.2 소켓 생성
	bool CreateCommunicationSocket();
	//1.3 서버 커넥트
	bool ConnectServer();

	int recvn(SOCKET sock, char* buffer, int totalBytes);


public:
	HBITMAP m_hBitmap;
	HANDLE hMutex;


private:
	//member
	//윈속
	WSADATA m_wsa;

	//서버와의 통신 소켓
	SOCKET m_CommunicationSocket;
	SOCKADDR_IN m_Serveraddr;
	int m_port;

	//서버와의 통신 버퍼
	char m_sendBuffer[1024];
	char m_recvBuffer[1024];

	char m_BitMapBuffer[16000000];

	//메시지 데이터
	Message message;

	//서버로의 데이터 전송을 맡을 스레드 핸들
	HANDLE m_hSendThread;
	DWORD dwSendThreadID;

	////서버로부터 데이터 수신을 맡을 스레드 핸들
	//HANDLE m_hRecvThread;
	//DWORD dwRecvThreadID;
};

DWORD WINAPI SendData(LPVOID lpParam);


void GetMutex();
void ReleaseMutex();

