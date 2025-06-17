#pragma once

#include "CRemoteControlRecvMode.h"
#include <vector>
#include <cstdio>
#include <cstring>


#define LOCAL_SERVER "127.0.0.1"
#define INNER_SERVER "192.168.45.15"
#define SERVICE_SERVER "59.14.59.1"

CRemoteControlRecvMode::CRemoteControlRecvMode()
{
	ZeroMemory(&m_Serveraddr, sizeof(m_Serveraddr));
	ZeroMemory(&message, sizeof(message));
}

CRemoteControlRecvMode::CRemoteControlRecvMode(const char* authId,
	const char* authPw, uint32_t myId, uint32_t targetId)
	: m_CommunicationSocket(INVALID_SOCKET)
	, m_myId(myId)
	, m_targetId(targetId)
	, m_hBitmap(NULL)
{
	ZeroMemory(&m_Serveraddr, sizeof(m_Serveraddr));
	ZeroMemory(&message, sizeof(message));

	strncpy_s(m_authId, authId, _TRUNCATE);
	strncpy_s(m_authPw, authPw, _TRUNCATE);
}

CRemoteControlRecvMode::~CRemoteControlRecvMode()
{
	if (m_CommunicationSocket != INVALID_SOCKET)
		closesocket(m_CommunicationSocket);
	WSACleanup();
}

/*
* public method
*/
int CRemoteControlRecvMode::StartClient(int port, const char* serverIp)
{

	if (!InitWinsock())
	{
		printf("InitWinsock() failed\n");
		return -1;
	}

	if (!CreateCommunicationSocket() )
	{
		printf("CreateCommunicationSocket() failed\n");
		return -2;
	}

	if (!ConnectServer(serverIp, port))
	{
		printf("ConnectServer() failed\n");
		return -3;
	}

	if (!PerformHandshake()) {
		printf("Handshake with server failed\n");
		return -4;
	}
	return Communication();
}

int CRemoteControlRecvMode::Communication()
{
	printf("[Client] RecvData Start!\n");

	while (true)
	{
		// 1) ReceiveBitmapMessage ���ο��� ���+���̷ε� ���š�HBITMAP ����
		HBITMAP hBmp = ReceiveBitmapMessage(m_CommunicationSocket);
		if (!hBmp) {
			break;
		}

		// ������ HBITMAP�� ���� ������ ����
		m_hBitmap = hBmp;

	}

	return 0;
}

// sock�κ��� ��Ȯ�� len ����Ʈ�� ���� ������ �ݺ� ȣ��
bool recvn(SOCKET sock, void* buf, size_t len) {
	uint8_t* ptr = reinterpret_cast<uint8_t*>(buf);
	size_t   received = 0;
	while (received < len) {
		int r = recv(sock, reinterpret_cast<char*>(ptr + received), int(len - received), 0);
		if (r <= 0) return false;  // ���� Ȥ�� ���� ����
		received += r;
	}
	return true;
}



/*
* private method
*/
bool CRemoteControlRecvMode::InitWinsock()
{
	if (WSAStartup(MAKEWORD(2, 2), &m_wsa) != 0)
	{
		printf("WSAStartup() failed\n");
		return false;
	}

	return true;
}

bool CRemoteControlRecvMode::CreateCommunicationSocket()
{
	m_CommunicationSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_CommunicationSocket == INVALID_SOCKET)
	{
		printf("socket() failed\n");
		return false;
	}

	return true;
}

bool CRemoteControlRecvMode::ConnectServer(const char* ip, int port) {
	m_Serveraddr.sin_family = AF_INET;
	m_Serveraddr.sin_port = htons(port);
	m_Serveraddr.sin_addr.s_addr = inet_addr(ip);

	int ret = connect(m_CommunicationSocket, (SOCKADDR*)&m_Serveraddr, sizeof(m_Serveraddr));
	if (ret == SOCKET_ERROR) {
		int err = WSAGetLastError();

		// ���� �޽��� ���ڿ��� ���� ����
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

bool CRemoteControlRecvMode::PerformHandshake()
{
	// 1) Unified header ����
	FullRequestHeader req{};
	strncpy_s(req.authId, m_authId, _TRUNCATE);
	strncpy_s(req.authPw, m_authPw, _TRUNCATE);
	req.mode = 'r';
	req.myId = htonl(m_myId);
	req.targetId = htonl(m_targetId);

	// 2) ����+�ڵ����ũ ��û ����
	if (send(m_CommunicationSocket, (char*)&req, sizeof(req), 0) != sizeof(req)) {
		printf("Failed to send handshake request\n");
		return false;
	}

	//// 3) ���� ���� ����
	//ConnResponse authResp{};
	//if (!recvn(m_CommunicationSocket, &authResp, sizeof(authResp))) return false;
	//if (authResp.success != 1) {
	//	printf("Auth failed: %s\n", authResp.info);
	//	return false;
	//}
	//printf("Auth OK: %s\n", authResp.info);

	// 4. �ڵ����ũ ���� ����
	ConnResponse resp{};
	if (!recvn(m_CommunicationSocket, (void*) & resp, sizeof(resp))) {
		printf("Failed to receive handshake response\n");
		return false;
	}

	if (resp.success != 1) {
		printf("Server error: %s\n", resp.info);
		return false;
	}

	printf("Handshake success: %s\n", resp.info);
	return true;
}


HBITMAP ReceiveBitmapMessage(SOCKET sock) {
	// 1) �����ŭ ���� �ޱ� (payload[1] ����)
	const size_t headerSize = sizeof(Message) - sizeof(uint8_t);
	std::vector<uint8_t> headerBuf(headerSize);
	if (!recvn(sock, headerBuf.data(), headerSize)) {
		return NULL;
	}

	// 2) payloadSize �Ľ�
	Message* hdr = reinterpret_cast<Message*>(headerBuf.data());
	if (ntohl(hdr->magic) != 0x424D4350) {
		return NULL;
	}
	uint64_t payloadSize = ntohll(hdr->payloadSize);

	// 3) ��� + ���̷ε� ��ü ���� �Ҵ� & ��� ����
	size_t totalSize = headerSize + payloadSize;
	uint8_t* fullBuf = new uint8_t[totalSize];
	memcpy(fullBuf, headerBuf.data(), headerSize);

	// 4) ������ ���̷ε� ����
	if (!recvn(sock, fullBuf + headerSize, payloadSize)) {
		delete[] fullBuf;
		return NULL;
	}

	// 5) üũ�� ����
	// recv�� checksum ����
	uint32_t recvCrc;
	std::memcpy(&recvCrc, fullBuf + offsetof(Message, checksum), sizeof(uint32_t));
	recvCrc = ntohl(recvCrc);

	// checksum �ʵ� 0���� Ŭ����
	std::memset(fullBuf + offsetof(Message, checksum), 0, sizeof(uint32_t));

	// ��� ���: magic ���ĺ��� ������
	uint8_t* csStart = fullBuf + sizeof(uint32_t);
	size_t   csLen = totalSize - sizeof(uint32_t);
	uint32_t calcCrc = calculateCRC32(csStart, csLen);

	if (recvCrc != calcCrc) {
		delete[] fullBuf;
		return NULL;
	}

	// 6) ��� ���� ������
	Message* msg = reinterpret_cast<Message*>(fullBuf);
	uint32_t width = ntohl(msg->width);
	uint32_t height = ntohl(msg->height);
	uint16_t bitCount = ntohs(msg->bitCount);
	uint32_t scanLine = ntohl(msg->widthBytes);
	uint8_t* pixels = msg->payload;

	// 7) DIBSection���� HBITMAP ����
	BITMAPINFO bmi;
	ZeroMemory(&bmi, sizeof(bmi));
	BITMAPINFOHEADER& bih = bmi.bmiHeader;
	bih.biSize = sizeof(bih);
	bih.biWidth = width;
	bih.biHeight = -static_cast<LONG>(height); // top-down
	bih.biPlanes = 1;
	bih.biBitCount = bitCount;
	bih.biCompression = BI_RGB;

	void* dibBits = nullptr;
	HDC    hdc = GetDC(NULL);
	HBITMAP hBmp = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &dibBits, NULL, 0);
	ReleaseDC(NULL, hdc);

	if (hBmp && dibBits) {
		// �ȼ� ������ ����
		std::memcpy(dibBits, pixels, payloadSize);
	}
	else {
	}

	delete[] fullBuf;
	return hBmp;
}



DWORD WINAPI SendData(LPVOID lpParam)
{
	CRemoteControlRecvMode* pClient = (CRemoteControlRecvMode*)lpParam;
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



