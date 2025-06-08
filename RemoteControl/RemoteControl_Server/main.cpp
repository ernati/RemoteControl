#pragma once
#include <CCaptureScreenAndSendBitMap.h>
#include <CPrintScreen.h>
#include <CMultiThreadServer.h>
#include <iostream>

CCaptureScreenAndSendBitMap* pCaptureScreen = nullptr;
CPrintScreen* g_printScreen = nullptr;
CMultiThreadServer* server = nullptr;

//Capture를 위한 ThreadFunction
DWORD WINAPI ThreadFunction(LPVOID lpParam)
{
    // lpParam을 사용하여 전달된 데이터를 처리할 수 있습니다.

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

    //2. 일정 시간 마다 화면 캡처
    // 반복 간격 설정 (예: 100ms)
    while (true) {
        //2.1 화면 캡처
        pCaptureScreen->CaptureScreen();
        if (pCaptureScreen->CheckSuccessCapture()) {
            // 2. 이전 캡쳐된 이미지가 있다면 해제
            pCaptureScreen->UpdateNewBitmap();

            //2.2 ThreadClient 내 BITMAP 변수로 값 이동
            server->m_hBitmap = pCaptureScreen->GetnewBitmap();

            //// 2.3 BITMAP 정보 출력
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
            //// 예상 출력
            //== = Captured BITMAP Info == =
            //    Width       : 2560
            //    Height : 1440
            //    Planes : 1
            //    BitCount : 32
            //    WidthBytes : 10240
            //    NumberOfBits : 117964800
            //    Reserved : 0000000000000000
            //============================

            ////2.4 Message 구조체 생성
            //// === 여기에 TEST 코드 삽입 ===
            //if (server->m_hBitmap) {
            //    // 1) 원본 BITMAP 정보 얻기
            //    BITMAP origBmp;
            //    if (!GetObject(server->m_hBitmap, sizeof(origBmp), &origBmp)) {
            //        std::cerr << "GetObject 실패: " << GetLastError() << "\n";
            //        continue;
            //    }
            //    size_t bitsSize = static_cast<size_t>(origBmp.bmHeight) * origBmp.bmWidthBytes;
            //    uint8_t* bits = new uint8_t[bitsSize];
            //    if (GetBitmapBits(server->m_hBitmap, static_cast<LONG>(bitsSize), bits) == 0) {
            //        std::cerr << "GetBitmapBits 실패\n";
            //        delete[] bits;
            //        continue;
            //    }

            //    // 2) Message 버퍼 생성
            //    size_t msgSize = 0;
            //    uint8_t* msgBuf = createMessageBuffer_tmp(origBmp, bits, bitsSize, msgSize);
            //    Message* msg = reinterpret_cast<Message*>(msgBuf);

            //    // 3) 체크섬 검증
            //    // 네트워크 바이트 오더 → 호스트 바이트 오더
            //    uint32_t recvCrc = ntohl(msg->checksum);
            //    // 계산을 위해 원본 checksum 필드를 0으로 세팅
            //    std::memset(msgBuf + offsetof(Message, checksum), 0, sizeof(msg->checksum));
            //    const uint8_t* csStart = msgBuf + sizeof(msg->magic);
            //    size_t        csLen = msgSize - sizeof(msg->magic);
            //    uint32_t      calcCrc = calculateCRC32(csStart, csLen);

            //    if (recvCrc != calcCrc) {
            //        std::cerr << "체크섬 불일치! 수신: " << recvCrc
            //            << " 계산: " << calcCrc << "\n";
            //    }
            //    else {
            //        std::cout << "체크섬 정상\n";

            //        // 4) Message 헤더 vs 원본 BITMAP 비교
            //        bool match = true;
            //        if (ntohl(msg->width) != static_cast<uint32_t>(origBmp.bmWidth))    match = false;
            //        if (ntohl(msg->height) != static_cast<uint32_t>(origBmp.bmHeight))   match = false;
            //        if (ntohs(msg->planes) != origBmp.bmPlanes)                         match = false;
            //        if (ntohs(msg->bitCount) != origBmp.bmBitsPixel)                     match = false;
            //        if (ntohl(msg->widthBytes) != origBmp.bmWidthBytes)                  match = false;
            //        if (ntohll(msg->payloadSize) != bitsSize)                            match = false;

            //        if (match) {
            //            std::cout << "Message 헤더와 원본 BITMAP 정보 일치\n";
            //        }
            //        else {
            //            std::cerr << "Message 헤더와 BITMAP 정보 불일치\n";
            //        }
            //    }

            //    // 5) 메모리 해제
            //    delete[] bits;
            //    delete[] msgBuf;
            //}
        }

        Sleep(100);
    }

    return 0;  // 스레드가 종료될 때 반환할 값
}

// main 함수: 콘솔 애플리케이션 형태의 진입점
int main() {

    server = new CMultiThreadServer();

    // Window를 위한 Thread 생성
    HANDLE hThread = CreateThread(
        NULL,          // 기본 보안 속성
        0,             // 기본 스택 크기
        ThreadFunction, // 스레드 함수
        NULL,         // 스레드 함수에 전달할 매개변수
        0,             // 생성 즉시 실행
        NULL           // 스레드 ID (필요 시 저장)
    );

    int nRet = 0;

	nRet = server->StartServer(9000);
	if (nRet < 0) {
		printf("server.StartServer() failed, nRet:%d\n", nRet);
	}

	return nRet;
}
