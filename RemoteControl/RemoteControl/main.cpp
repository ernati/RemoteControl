// RemoteControl Client Application
// main.cpp는 구현 파일이므로 #pragma once가 필요하지 않습니다.

#include <CCaptureScreenAndSendBitMap.h>
#include <CPrintScreen.h>
#include <CRemoteControlRecvMode.h>
#include <CRemoteControlSendMode.h>

#include <iostream>
#include <string>
#include <cstdlib>
#include <memory>  // std::unique_ptr를 위해 추가

#define LOCAL_SERVER "127.0.0.1"
#define INNER_SERVER "192.168.45.15"
#define SERVICE_SERVER "59.14.59.1"

// 전역 포인터들 - 추후 RAII 패턴으로 개선 필요
CRemoteControlRecvMode* RecvMode = nullptr;
CCaptureScreenAndSendBitMap* pCaptureScreen = nullptr;
CPrintScreen* g_printScreen = nullptr;
CRemoteControlSendMode* server = nullptr;

// 윈도우 그리기 처리 함수 분리
void HandleWindowPaint(HWND hwnd) {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);

    // 창 클라이언트 영역의 좌표를 검사
    RECT rect;
    GetClientRect(hwnd, &rect);
    int winWidth = rect.right - rect.left;
    int winHeight = rect.bottom - rect.top;

    if (RecvMode && RecvMode->m_hBitmap != NULL) {
        // 메모리 DC 생성 및 비트맵 선택
        HDC hMemDC = CreateCompatibleDC(hdc);
        if (hMemDC == NULL) {
            printf("CreateCompatibleDC 실패!\n");
            EndPaint(hwnd, &ps);
            return;
        }

        HBITMAP hOldBmp = (HBITMAP)SelectObject(hMemDC, RecvMode->m_hBitmap);
        if (hOldBmp == NULL) {
            printf("SelectObject 실패!\n");
            DeleteDC(hMemDC);
            EndPaint(hwnd, &ps);
            return;
        }

        // 비트맵 정보 획득
        BITMAP bm;
        GetObject(RecvMode->m_hBitmap, sizeof(BITMAP), &bm);

        // 화면에 비트맵 그리기 (크기 조정)
        StretchBlt(hdc, 0, 0, winWidth, winHeight, hMemDC, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);

        // 리소스 정리
        SelectObject(hMemDC, hOldBmp);
        DeleteDC(hMemDC);
    }

    EndPaint(hwnd, &ps);
}

// 윈도우 프로시저: 창의 메시지를 처리합니다.
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE:
        // 타이머 설정: 주기적 화면 캡처 및 업데이트
        if (pCaptureScreen) {
            SetTimer(hwnd, 1, pCaptureScreen->GetCaptureInterval(), NULL);
        }
        return 0;

    case WM_TIMER:
        // 주기적 시간마다 화면갱신
        if (RecvMode && RecvMode->m_hBitmap != NULL) {
            InvalidateRect(hwnd, NULL, TRUE);
        }
        return 0;

    case WM_PAINT:
        HandleWindowPaint(hwnd);
        return 0;

    case WM_DESTROY:
        KillTimer(hwnd, 1);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

DWORD WINAPI ThreadFunctionRecv(LPVOID lpParam)
{
    // 콘솔 할당 및 표준 출력 리다이렉션
    AllocConsole();
    FILE* pCout;
    freopen_s(&pCout, "CONOUT$", "w", stdout);
    printf("콘솔 창이 할당되었습니다.\n");
    
    
    // lpParam을 사용하여 전달된 데이터를 처리할 수 있습니다.

    HINSTANCE hInstance = GetModuleHandle(NULL);

    // 지역적으로 객체 생성 (전역 포인터 대신 사용)
    auto localCaptureScreen = std::make_unique<CCaptureScreenAndSendBitMap>();
    auto localPrintScreen = std::make_unique<CPrintScreen>(hInstance);

    // 안전한 포인터 할당을 위한 체크
    if (!localCaptureScreen || !localPrintScreen) {
        std::cerr << "객체 생성 실패" << std::endl;
        return -1;
    }

    // 전역 포인터에 할당 (기존 코드와 호환성을 위해)
    pCaptureScreen = localCaptureScreen.get();
    g_printScreen = localPrintScreen.get();

    g_printScreen->setCaptureScreen(pCaptureScreen);
    g_printScreen->sethInstance(hInstance);


    //1. null check
    if (!g_printScreen->m_hInstance) {
        std::cerr << "m_hInstance is nullptr" << std::endl;
        return -1;
    }

    // 2. 윈도우 클래스 등록
    const wchar_t CLASS_NAME[] = L"ResponsiveScreenCaptureWindowClass";

    // 2.1 윈도우 클래스 구조체 초기화 및 등록
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = g_printScreen->m_hInstance;
    wc.lpszClassName = CLASS_NAME;

    // 2.2 윈도우 클래스 등록 실패 시 오류 메시지 출력 후 종료
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
        g_printScreen->m_captureScreen         // 사용자 파라미터
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


//Capture에 관한 ThreadFunction
DWORD WINAPI ThreadFunctionSend(LPVOID lpParam)
{
    // lpParam을 사용하여 전달된 데이터를 처리할 수 있습니다.

    HINSTANCE hInstance = GetModuleHandle(NULL);

    // 지역적으로 객체 생성 (전역 포인터 대신 사용)
    auto localCaptureScreen = std::make_unique<CCaptureScreenAndSendBitMap>();
    auto localPrintScreen = std::make_unique<CPrintScreen>(hInstance);

    // 안전한 포인터 할당을 위한 체크
    if (!localCaptureScreen || !localPrintScreen) {
        std::cerr << "객체 생성 실패" << std::endl;
        return -1;
    }

    // 전역 포인터에 할당 (기존 코드와 호환성을 위해)
    pCaptureScreen = localCaptureScreen.get();
    g_printScreen = localPrintScreen.get();

    g_printScreen->setCaptureScreen(pCaptureScreen);
    g_printScreen->sethInstance(hInstance);


    //1. null check
    if (!g_printScreen->m_hInstance) {
        std::cerr << "m_hInstance is nullptr" << std::endl;
        return -1;
    }

    //2. 주기적 시간 마다 화면 캡처
    // 반복 실행 간격 (예: 100ms)
    while (true) {
        //2.1 화면 캡처
        pCaptureScreen->CaptureScreen();
        if (pCaptureScreen->CheckSuccessCapture()) {
            // 2. 새로 캡처된 이미지가 있다면 갱신
            pCaptureScreen->UpdateNewBitmap();

            //2.2 ThreadClient 의 BITMAP 주소값 을 이동
            if (server) {  // null 체크 추가
                server->m_hBitmap = pCaptureScreen->GetnewBitmap();
            }
        }

        //Sleep(100);  // CPU 사용률 개선을 위해 Sleep 주석 해제 고려
    }

    return 0;  // 스레드가 종료될 때 반환할 값
}

// ���� ���
void PrintHelp(const char* progName) {
    printf("Usage:\n");
    printf("  %s <network> <authId> <authPw> recv <myId> <targetId>\n", progName);
    printf("    <network>   : loopback | inner | service\n");
    printf("    <authId>    : your user ID\n");
    printf("    <authPw>    : your password\n");
    printf("    <myId>      : this client's numeric ID (recv mode)\n");
    printf("    <targetId>  : ID of the send-mode client to connect to\n\n");
    printf("  %s <network> <authId> <authPw> send <myId>\n", progName);
    printf("    <myId>      : this client's numeric ID (send mode)\n");
}


int main(int argc, char* argv[]) {
    if (argc < 6) {
        PrintHelp(argv[0]);
        return -1;
    }

    // 1) ��Ʈ��ũ ����
    const char* serverIp = nullptr;
    std::string net(argv[1]);
    if (net == "loopback") {
        serverIp = LOCAL_SERVER;
    }
    else if (net == "inner") {
        serverIp = INNER_SERVER;
    }
    else if (net == "service") {
        serverIp = SERVICE_SERVER;
    }
    else {
        printf("Unknown network '%s'\n\n", argv[1]);
        PrintHelp(argv[0]);
        return -1;
    }

    // 2) ���� ����
    const char* authId = argv[2];
    const char* authPw = argv[3];

    // 3. ���� ȭ�� ������ ���� �Ķ����
    std::string mode(argv[4]);
    int nRet = 0;

    if (mode == "recv") {
        if (argc < 7) {
            PrintHelp(argv[0]);
            return -1;
        }
        
        uint32_t myId = static_cast<uint32_t>(std::stoul(argv[5]));
        uint32_t targetId = static_cast<uint32_t>(std::stoul(argv[6]));

        // 메모리 관리 개선: unique_ptr 사용
        auto recvModePtr = std::make_unique<CRemoteControlRecvMode>(authId, authPw, myId, targetId);
        RecvMode = recvModePtr.get();  // 기존 코드와 호환성을 위해

        // 캡처에 관한 Thread 생성
        HANDLE hThread = CreateThread(
            NULL,          // 기본 보안 속성
            0,             // 기본 스택 크기
            ThreadFunctionRecv, // 스레드 함수
            NULL,         // 스레드 함수에 전달할 매개변수
            0,             // 생성 즉시 실행
            NULL           // 스레드 ID (필요 시 사용)
        );

        if (hThread == NULL) {
            printf("ThreadFunctionRecv 생성 실패\n");
            return -1;
        }

        nRet = RecvMode->StartClient(25000, serverIp);
        if (nRet < 0) {
            printf("RecvMode.StartClient() failed, nRet:%d\n", nRet);
        }

        // 스레드 정리
        if (hThread) {
            WaitForSingleObject(hThread, INFINITE);
            CloseHandle(hThread);
        }

    }
    else if (mode == "send") {
        // send ���� ID, targetId �� �� �ʿ�
        if (argc < 6) {
            PrintHelp(argv[0]);
            return -1;
        }
        uint32_t myId = static_cast<uint32_t>(std::stoul(argv[5]));

        // 메모리 관리 개선: unique_ptr 사용
        auto serverPtr = std::make_unique<CRemoteControlSendMode>(authId, authPw, myId);
        server = serverPtr.get();  // 기존 코드와 호환성을 위해

        // Window에 관한 Thread 생성
        HANDLE hThread = CreateThread(
            NULL,          // 기본 보안 속성
            0,             // 기본 스택 크기
            ThreadFunctionSend, // 스레드 함수
            NULL,         // 스레드 함수에 전달할 매개변수
            0,             // 생성 즉시 실행
            NULL           // 스레드 ID (필요 시 사용)
        );

        if (hThread == NULL) {
            printf("ThreadFunctionSend 생성 실패\n");
            return -1;
        }

        int nRet = 0;

        nRet = server->StartClient(25000, serverIp);
        if (nRet < 0) {
            printf("server.StartClient() failed, nRet:%d\n", nRet);
        }

        // 스레드 정리
        if (hThread) {
            WaitForSingleObject(hThread, INFINITE);
            CloseHandle(hThread);
        }
    }
    else {
        PrintHelp(argv[0]);
        return -2;
    }

	return nRet;
}


