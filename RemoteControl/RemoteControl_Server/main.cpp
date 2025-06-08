#pragma once
#include <CCaptureScreenAndSendBitMap.h>
#include <CPrintScreen.h>
#include <CMultiThreadServer.h>
#include <iostream>

CCaptureScreenAndSendBitMap* pCaptureScreen = nullptr;
CPrintScreen* g_printScreen = nullptr;
CMultiThreadServer* server = nullptr;

//Capture�� ���� ThreadFunction
DWORD WINAPI ThreadFunction(LPVOID lpParam)
{
    // lpParam�� ����Ͽ� ���޵� �����͸� ó���� �� �ֽ��ϴ�.

    HINSTANCE hInstance = GetModuleHandle(NULL);

    pCaptureScreen = new CCaptureScreenAndSendBitMap();
    g_printScreen = new CPrintScreen(hInstance);

    g_printScreen->setCaptureScreen(pCaptureScreen);
    g_printScreen->sethInstance(hInstance);


    //1. null check
    if (!g_printScreen->m_hInstance) {
        std::cerr << "m_captureScreen is nullptr" << std::endl;
        return false;
    }

    //2. ���� �ð� ���� ȭ�� ĸó
    // �ݺ� ���� ���� (��: 100ms)
    while (true) {
        //2.1 ȭ�� ĸó
        pCaptureScreen->CaptureScreen();
        if (pCaptureScreen->CheckSuccessCapture()) {
            // 2. ���� ĸ�ĵ� �̹����� �ִٸ� ����
            pCaptureScreen->UpdateNewBitmap();

            //2.2 ThreadClient �� BITMAP ������ �� �̵�
            server->m_hBitmap = pCaptureScreen->GetnewBitmap();

            //// 2.3 BITMAP ���� ���
            //if (server->m_hBitmap) {
            //    BITMAP bmpInfo;
            //    if (GetObject(server->m_hBitmap, sizeof(BITMAP), &bmpInfo)) {
            //        std::cout << "=== Captured BITMAP Info ===\n";
            //        std::cout << "Width       : " << bmpInfo.bmWidth << "\n";
            //        std::cout << "Height      : " << bmpInfo.bmHeight << "\n";
            //        std::cout << "Planes      : " << bmpInfo.bmPlanes << "\n";
            //        std::cout << "BitCount    : " << bmpInfo.bmBitsPixel << "\n";
            //        std::cout << "WidthBytes  : " << bmpInfo.bmWidthBytes << "\n";
            //        std::cout << "NumberOfBits: " << bmpInfo.bmHeight * bmpInfo.bmWidthBytes * 8 << "\n";
            //        std::cout << "Reserved    : " << bmpInfo.bmBits << "\n";
            //        std::cout << "============================\n";
            //    }
            //    else {
            //        std::cerr << "GetObject failed: " << GetLastError() << std::endl;
            //    }
            //}
            //// ���� ���
            //== = Captured BITMAP Info == =
            //    Width       : 2560
            //    Height : 1440
            //    Planes : 1
            //    BitCount : 32
            //    WidthBytes : 10240
            //    NumberOfBits : 117964800
            //    Reserved : 0000000000000000
            //============================

            ////2.4 Message ����ü ����
            //// === ���⿡ TEST �ڵ� ���� ===
            //if (server->m_hBitmap) {
            //    // 1) ���� BITMAP ���� ���
            //    BITMAP origBmp;
            //    if (!GetObject(server->m_hBitmap, sizeof(origBmp), &origBmp)) {
            //        std::cerr << "GetObject ����: " << GetLastError() << "\n";
            //        continue;
            //    }
            //    size_t bitsSize = static_cast<size_t>(origBmp.bmHeight) * origBmp.bmWidthBytes;
            //    uint8_t* bits = new uint8_t[bitsSize];
            //    if (GetBitmapBits(server->m_hBitmap, static_cast<LONG>(bitsSize), bits) == 0) {
            //        std::cerr << "GetBitmapBits ����\n";
            //        delete[] bits;
            //        continue;
            //    }

            //    // 2) Message ���� ����
            //    size_t msgSize = 0;
            //    uint8_t* msgBuf = createMessageBuffer_tmp(origBmp, bits, bitsSize, msgSize);
            //    Message* msg = reinterpret_cast<Message*>(msgBuf);

            //    // 3) üũ�� ����
            //    // ��Ʈ��ũ ����Ʈ ���� �� ȣ��Ʈ ����Ʈ ����
            //    uint32_t recvCrc = ntohl(msg->checksum);
            //    // ����� ���� ���� checksum �ʵ带 0���� ����
            //    std::memset(msgBuf + offsetof(Message, checksum), 0, sizeof(msg->checksum));
            //    const uint8_t* csStart = msgBuf + sizeof(msg->magic);
            //    size_t        csLen = msgSize - sizeof(msg->magic);
            //    uint32_t      calcCrc = calculateCRC32(csStart, csLen);

            //    if (recvCrc != calcCrc) {
            //        std::cerr << "üũ�� ����ġ! ����: " << recvCrc
            //            << " ���: " << calcCrc << "\n";
            //    }
            //    else {
            //        std::cout << "üũ�� ����\n";

            //        // 4) Message ��� vs ���� BITMAP ��
            //        bool match = true;
            //        if (ntohl(msg->width) != static_cast<uint32_t>(origBmp.bmWidth))    match = false;
            //        if (ntohl(msg->height) != static_cast<uint32_t>(origBmp.bmHeight))   match = false;
            //        if (ntohs(msg->planes) != origBmp.bmPlanes)                         match = false;
            //        if (ntohs(msg->bitCount) != origBmp.bmBitsPixel)                     match = false;
            //        if (ntohl(msg->widthBytes) != origBmp.bmWidthBytes)                  match = false;
            //        if (ntohll(msg->payloadSize) != bitsSize)                            match = false;

            //        if (match) {
            //            std::cout << "Message ����� ���� BITMAP ���� ��ġ\n";
            //        }
            //        else {
            //            std::cerr << "Message ����� BITMAP ���� ����ġ\n";
            //        }
            //    }

            //    // 5) �޸� ����
            //    delete[] bits;
            //    delete[] msgBuf;
            //}
        }

        Sleep(100);
    }

    return 0;  // �����尡 ����� �� ��ȯ�� ��
}

// main �Լ�: �ܼ� ���ø����̼� ������ ������
int main() {

    server = new CMultiThreadServer();

    // Window�� ���� Thread ����
    HANDLE hThread = CreateThread(
        NULL,          // �⺻ ���� �Ӽ�
        0,             // �⺻ ���� ũ��
        ThreadFunction, // ������ �Լ�
        NULL,         // ������ �Լ��� ������ �Ű�����
        0,             // ���� ��� ����
        NULL           // ������ ID (�ʿ� �� ����)
    );

    int nRet = 0;

	nRet = server->StartServer(9000);
	if (nRet < 0) {
		printf("server.StartServer() failed, nRet:%d\n", nRet);
	}

	return nRet;
}
