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
