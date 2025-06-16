#pragma once
#include <CCaptureScreenAndSendBitMap.h>
#include <CPrintScreen.h>
#include <CRemoteControlRecvMode.h>

#include <CRemoteControlSendMode.h>

#include <iostream>
#include <string>
#include <cstdlib>

#define LOCAL_SERVER "127.0.0.1"
#define INNER_SERVER "192.168.45.15"
#define SERVICE_SERVER "59.14.59.1"

CRemoteControlRecvMode* RecvMode = nullptr;
CCaptureScreenAndSendBitMap* pCaptureScreen = nullptr;
CPrintScreen* g_printScreen = nullptr;
CRemoteControlSendMode* server = nullptr;

// 윈도우 프로시저: 창의 메시지를 처리합니다. - 창 생성, 타이머 설정, 화면 캡쳐 및 업데이트
    // DispatchMessage 함수에서 호출됨.
    // 클래스 내에 static CALLBACK 멤버 함수를 선언하면, 이 함수는 클래스의 인스턴스(즉, this 포인터)를 갖지 않음.
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    //1. uMsg에 따라 처리
    switch (uMsg) {
    case WM_CREATE:
        // 타이머 설정: 일정 주기로 화면 캡쳐 및 업데이트
        // SetTimer 함수: 타이머를 설정하고, 3번째 파라미터( CAPTURE_INTERVAL )가 경과할 때마다 지정된 창의 메시지 큐에 WM_TIMER 메시지를 발생시킴.
        SetTimer(hwnd, 1, pCaptureScreen->GetCaptureInterval(), NULL);
        return 0;

    case WM_TIMER:
        // 1. 설정된 시간마다 화면출력

        if (RecvMode->m_hBitmap != NULL) {
            //printf("화면을 출력합니다!\n");
            InvalidateRect(hwnd, NULL, TRUE);
        }
        return 0;

    case WM_PAINT: {
        //1. BeginPaint - Window에 의해 무효화된( 다시 그려야 하는 ) 업데이트 영역을 확인하여, 그 영역의 정보를 PAINTSTRUCT 구조체에 저장.
        // 이후, 해당 영역을 그리기 위한 DC 핸들을 반환.
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        // 2. 창 클라이언트 영역의 좌표를 검색.
        // 왼쪽 위 좌표(0, 0), 오른쪽 아래 좌표(창의 너비, 창의 높이)
        RECT rect;
        GetClientRect(hwnd, &rect);
        int winWidth = rect.right - rect.left;
        int winHeight = rect.bottom - rect.top;

        if (RecvMode->m_hBitmap != NULL) {

            //RecvMode->GetMutex();

            // 3. 획득한 기존 DC와 동일한 속성을 가지는 DC를 메모리에 생성하고, 그 핸들을 획득. - 이미지 처리는 메모리 DC에서 수행.
            // SelectObject - DC가 특정 GDI 객체( 여기서는 비트맵 )을 선택하도록 하여, 이후 그리기 작업이 해당 객체의 속성을 사용하도록 함.
            HDC hMemDC = CreateCompatibleDC(hdc);
            if (hMemDC == NULL) {
                printf("CreateCompatibleDC Fail!\n");
            }

            HBITMAP hOldBmp = (HBITMAP)SelectObject(hMemDC, RecvMode->m_hBitmap); // hOldBmp - 이전에 DC에 선택되어 있던 비트맵 객체의 핸들을 저장
            if (hOldBmp == NULL) {
                printf("SelectObject Fail!\n");
            }

            // 4. GetObject - 비트맵 정보를 버퍼( BITMAP 구조체 )에 저장
            BITMAP bm;
            GetObject(RecvMode->m_hBitmap, sizeof(BITMAP), &bm);

            // 5. Src DC( Memory DC )로부터 픽셀 데이터( 전역 변수에 저장된 캡쳐된 화면 )을 Dest DC( 전체 화면에 대한 Device Context )에 복사함 ( SRCCOPY - 단순 복사 )
            // 대상 사각형의 크기에 맞게 확대 또는 축소하여 복사
            StretchBlt(hdc, 0, 0, winWidth, winHeight, hMemDC, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);

            // 6. 선택 객체 원복 후 메모리 DC 삭제 - 메모리 누수 방지
            SelectObject(hMemDC, hOldBmp);
            DeleteDC(hMemDC);

            //ReleaseMutex
            //RecvMode->ReleaseMutex_Custom();
        }

        //7. EndPaint - BeginPaint 함수로부터 반환된 DC 핸들을 해제
        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_DESTROY:
        KillTimer(hwnd, 1);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

DWORD WINAPI ThreadFunctionRecv(LPVOID lpParam)
{
    // 콘솔 할당 및 표준 출력 재지정
    AllocConsole();
    FILE* pCout;
    freopen_s(&pCout, "CONOUT$", "w", stdout);
    printf("콘솔 창이 할당되었습니다.\n");
    
    
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

    // 2. 윈도우 클래스 등록
    const wchar_t CLASS_NAME[] = L"ResponsiveScreenCaptureWindowClass";

    // 2.1 윈도우 클래스 구조체 초기화 및 등록
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = g_printScreen->m_hInstance;
    wc.lpszClassName = CLASS_NAME;

    // 2.2 윈도우 클래스 등록 실패 시 에러 메시지 출력 후 종료
    if (!RegisterClass(&wc)) {
        std::cerr << "윈도우 클래스 등록 실패" << std::endl;
        return -1;
    }

    // 3. 창 생성
    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"Responsive Screen Capture Display",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, g_printScreen->m_nWidth, g_printScreen->m_nHeight,
        NULL,
        NULL,
        g_printScreen->m_hInstance,
        g_printScreen->m_captureScreen         // 전달할 파라미터
    );

    if (!hwnd) {
        std::cerr << "창 생성 실패" << std::endl;
        return -1;
    }

    // 4. 창 표시
    ShowWindow(hwnd, SW_SHOW);

    // 6. 메시지 루프
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;  // 스레드가 종료될 때 반환할 값
}


//Capture를 위한 ThreadFunction
DWORD WINAPI ThreadFunctionSend(LPVOID lpParam)
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
        }

        //Sleep(100);
    }

    return 0;  // 스레드가 종료될 때 반환할 값
}

// 사용법 출력
void PrintHelp(const char* progName) {
    printf("Usage:\n");
    printf("  %s recv <myId> <targetId>\n", progName);
    printf("    <myId>      : this client's ID (recv mode)\n");
    printf("    <targetId>  : ID of the send-mode client you want to connect to\n");
    printf("  %s send <myId>\n", progName);
    printf("    <myId>      : this client's ID (send mode)\n");
}


int main(int argc, char* argv[]) {
    if (argc < 2) {
        PrintHelp(argv[0]);
        return -1;
    }

    std::string mode(argv[1]);
    int nRet = 0;

    if (mode == "recv") {
        // recv 모드는 myId 하나만 필요
        if (argc < 3) {
            PrintHelp(argv[0]);
            return -1;
        }
        uint32_t myId = static_cast<uint32_t>(std::stoul(argv[2]));

        RecvMode = new CRemoteControlRecvMode();

        // 캡처를 위한 Thread 생성
        HANDLE hThread = CreateThread(
            NULL,          // 기본 보안 속성
            0,             // 기본 스택 크기
            ThreadFunctionRecv, // 스레드 함수
            NULL,         // 스레드 함수에 전달할 매개변수
            0,             // 생성 즉시 실행
            NULL           // 스레드 ID (필요 시 저장)
        );

        nRet = RecvMode->StartClient(25000, SERVICE_SERVER);
        if (nRet < 0) {
            printf("RecvMode.StartClient() failed, nRet:%d\n", nRet);
        }

    }
    else if (mode == "send") {
        // send 모드는 ID, targetId 둘 다 필요
        if (argc < 4) {
            PrintHelp(argv[0]);
            return -1;
        }
        uint32_t myId = static_cast<uint32_t>(std::stoul(argv[2]));
        uint32_t targetId = static_cast<uint32_t>(std::stoul(argv[3]));


        server = new CRemoteControlSendMode();

        // Window를 위한 Thread 생성
        HANDLE hThread = CreateThread(
            NULL,          // 기본 보안 속성
            0,             // 기본 스택 크기
            ThreadFunctionSend, // 스레드 함수
            NULL,         // 스레드 함수에 전달할 매개변수
            0,             // 생성 즉시 실행
            NULL           // 스레드 ID (필요 시 저장)
        );

        int nRet = 0;

        nRet = server->StartClient(25000, SERVICE_SERVER);
        if (nRet < 0) {
            printf("server.StartServer() failed, nRet:%d\n", nRet);
        }
    }
    else {
        PrintHelp(argv[0]);
        return -2;
    }

	return nRet;
}


