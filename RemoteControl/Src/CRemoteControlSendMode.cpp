#pragma once

#include <CRemoteControlSendMode.h>

CRemoteControlSendMode::CRemoteControlSendMode()
{
	ZeroMemory(&m_wsa, sizeof(m_wsa));

	m_port = 0;

	ZeroMemory(m_recvBuffer, sizeof(m_recvBuffer));
	ZeroMemory(m_sendBuffer, sizeof(m_sendBuffer));

	m_socket = INVALID_SOCKET;

	m_myId = 0;

}

CRemoteControlSendMode::~CRemoteControlSendMode() {
	if (m_socket != INVALID_SOCKET)
		closesocket(m_socket);
	WSACleanup();
}

/*
*	public method
*/



int CRemoteControlSendMode::StartClient(int port, const char* serverIp) {
	m_port = port;
	if (!InitWinsock()) {
		printf("WSAStartup failed\n"); return -1;
	}
	if (!CreateSocket()) {
		printf("socket() failed\n"); return -2;
	}
	if (!ConnectServer(serverIp, port)) {
		printf("connect() failed\n"); return -3;
	}
	if (!PerformHandshake()) {
		printf("Handshake failed\n"); return -4;
	}
	return Communication();
}

/*
*	private method
*/



bool CRemoteControlSendMode::InitWinsock()
{
	if (0 != WSAStartup(MAKEWORD(2, 2), &m_wsa))
	{
		printf("WSAStartup() failed\n");
		return false;
	}

	return true;
}

bool CRemoteControlSendMode::CreateSocket()
{
	m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); //TCP
	if (INVALID_SOCKET == m_socket)
	{
		printf("socket() failed\n");
		return false;
	}

	return true;
}

bool CRemoteControlSendMode::ConnectServer(const char* ip, int port) {
	m_serverAddr.sin_family = AF_INET;
	m_serverAddr.sin_port = htons(port);
	m_serverAddr.sin_addr.s_addr = inet_addr(ip);

	int ret = connect(m_socket, (SOCKADDR*)&m_serverAddr, sizeof(m_serverAddr));
	if (ret == SOCKET_ERROR) {
		int err = WSAGetLastError();

		// 에러 메시지 문자열을 얻어올 버퍼
		char msgBuf[512] = { 0 };
		FormatMessageA(
			FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			err,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			msgBuf,
			(DWORD)sizeof(msgBuf),
			NULL
		);

		printf("connect() failed. Error %d: %s\n", err, msgBuf);
		return false;
	}

	return true;
}

bool recvn_sendMode(SOCKET sock, void* buf, size_t len) {
	uint8_t* ptr = (uint8_t*)buf;
	size_t received = 0;
	while (received < len) {
		int r = recv(sock, (char*)(ptr + received), int(len - received), 0);
		if (r <= 0) return false;
		received += r;
	}
	return true;
}

int sendn_sendMode(SOCKET sock, const char* buffer, int totalBytes) {
	int totalSent = 0;
	while (totalSent < totalBytes) {
		int n = send(sock, buffer + totalSent, totalBytes - totalSent, 0);
		if (n == SOCKET_ERROR) {
			// 에러 처리: 필요하면 WSAGetLastError()로 에러 코드 확인
			return SOCKET_ERROR;
		}
		totalSent += n;
	}
	return totalSent;
}

bool CRemoteControlSendMode::PerformHandshake() {
	ConnRequestHeader req{};
	req.mode = 's';
	req.myId = htonl(m_myId);
	req.target = 0;
	if (send(m_socket, (char*)&req, sizeof(req), 0) != sizeof(req)) {
		return false;
	}
	ConnResponse resp{};
	if (!recvn_sendMode(m_socket, &resp, sizeof(resp))) return false;
	if (resp.success != 1) {
		printf("Server: %s\n", resp.info);
		return false;
	}
	printf("Handshake OK: %s\n", resp.info);
	return true;
}


int CRemoteControlSendMode::Communication() {
	printf("[Client] SendData Start\n");

	// 1) SendData_SendMode 쓰레드 생성
	DWORD tid;
	HANDLE hThread = ::CreateThread(
		NULL, 0,
		SendData_SendMode,
		(LPVOID)this,
		0, &tid
	);
	if (!hThread) {
		printf("Failed to create send thread\n");
		return -1;
	}

	// 2) 쓰레드가 종료될 때까지 대기
	::WaitForSingleObject(hThread, INFINITE);
	::CloseHandle(hThread);

	// 3) 종료 직전, 소켓을 깔끔히 닫아 서버가 닫힘을 인지하게 함
	::shutdown(m_socket, SD_BOTH);
	::closesocket(m_socket);
	m_socket = INVALID_SOCKET;

	printf("[Client] Communication ended, socket closed\n");
	return 0;
}


DWORD WINAPI SendData_SendMode(LPVOID lpParam)
{
	CRemoteControlSendMode* pServer = (CRemoteControlSendMode*)lpParam;
	SOCKET hSocket = pServer->GetSocket();
	char* pSendBuffer = pServer->GetSendBuffer();
	char* pszTmp = NULL;

	printf("[Server] SendData_SendMode Start!\n");

	DWORD dwSendSize = 0;
	
	while (1)
	{
		//1. Bitmap 데이터가 NULL이 아닐경우, 데이터를 전송한다.
		if (pServer->GetBitMap() != NULL) {
			//1.1 멤버에 담긴 BITMAP 데이터를 획득
			BITMAP BmpCaptured;
			if (!GetObject(pServer->GetBitMap(), sizeof(BmpCaptured), &BmpCaptured)) {
			    continue;
			}
			size_t bitsSize = static_cast<size_t>(BmpCaptured.bmHeight) * BmpCaptured.bmWidthBytes;
			uint8_t* bits = new uint8_t[bitsSize];
			if (GetBitmapBits(pServer->GetBitMap(), static_cast<LONG>(bitsSize), bits) == 0) {
			    delete[] bits;
			    continue;
			}
			
			//1.2 BITMAP의 메타 데이터가 담긴 Message 버퍼 생성
			size_t msgSize = 0;
			uint8_t* msgBuf = createMessageBuffer_tmp(BmpCaptured, bits, bitsSize, msgSize);
			Message* msg = reinterpret_cast<Message*>(msgBuf);

			//1.3 사용자가 입력한 문자열을 서버에 전송한다.
			sendn_sendMode(hSocket, (const char*)msgBuf, msgSize);

			//1.4 메모리 해제
			delete[] bits;
			delete[] msgBuf;
		}

		Sleep(100);
	}

	return 0;
}

DWORD WINAPI RecvData_SendMode(LPVOID lpParam)
{
	CRemoteControlSendMode* pServer = (CRemoteControlSendMode*)lpParam;
	SOCKET hSocket = pServer->GetSocket();
	char* pRecvBuffer = pServer->GetRecvBuffer();

	printf("[Server] RecvData_SendMode Start!\n");

	while (1)
	{
		//3.3 서버로부터 데이터를 수신한다.
		::recv(hSocket, pRecvBuffer, sizeof(pRecvBuffer), 0);
		if (strcmp(pRecvBuffer, "EXIT") == 0)	break;

		//3.4 서버로부터 수신한 데이터를 출력한다.
		printf("Client : %s\n", pRecvBuffer);
	}

	return 0;
}