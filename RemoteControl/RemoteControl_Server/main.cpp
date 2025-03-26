#pragma once
#include <CCaptureScreenAndSendBitMap.h>
#include <CPrintScreen.h>
#include <CMultiThreadServer.h>
#include <iostream>

CCaptureScreenAndSendBitMap* pCaptureScreen = nullptr;
CPrintScreen* g_printScreen = nullptr;

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
        if (wParam == 1) {
            // 1. 설정된 시간마다 화면 캡쳐
            pCaptureScreen->CaptureScreen();
            if (pCaptureScreen->CheckSuccessCapture()) {
                // 2. 이전 캡쳐된 이미지가 있다면 해제
                pCaptureScreen->UpdateNewBitmap();

                // 3. 캡쳐한 이미지를 통해 화면 갱신 요청, 두번째 파라미터가 NULL이므로 전체 화면 갱신
                // 세부 정보 : InvalidateRect 함수 - 지정한 창의 클라이언트 영역 중 일부(또는 전체)를 무효 영역(update region)으로 표시합니다.
                // 무효 영역이 설정되면, Windows는 메시지 루프가 유휴 상태에 도달했을 때 자동으로 WM_PAINT 메시지를 해당 창의 메시지 큐에 게시.
                InvalidateRect(hwnd, NULL, TRUE);
            }
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

        if (pCaptureScreen->CheckReadyForPrintScreen()) {
            // 3. 획득한 기존 DC와 동일한 속성을 가지는 DC를 메모리에 생성하고, 그 핸들을 획득. - 이미지 처리는 메모리 DC에서 수행.
            // SelectObject - DC가 특정 GDI 객체( 여기서는 비트맵 )을 선택하도록 하여, 이후 그리기 작업이 해당 객체의 속성을 사용하도록 함.
            HDC hMemDC = CreateCompatibleDC(hdc);
            HBITMAP hOldBmp = (HBITMAP)SelectObject(hMemDC, pCaptureScreen->GetnewBitmap()); // hOldBmp - 이전에 DC에 선택되어 있던 비트맵 객체의 핸들을 저장

            // 4. GetObject - 비트맵 정보를 버퍼( BITMAP 구조체 )에 저장
            BITMAP bm;
            GetObject(pCaptureScreen->GetnewBitmap(), sizeof(BITMAP), &bm);

            // 5. Src DC( Memory DC )로부터 픽셀 데이터( 전역 변수에 저장된 캡쳐된 화면 )을 Dest DC( 전체 화면에 대한 Device Context )에 복사함 ( SRCCOPY - 단순 복사 )
            // 대상 사각형의 크기에 맞게 확대 또는 축소하여 복사
            StretchBlt(hdc, 0, 0, winWidth, winHeight, hMemDC, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);

            // 6. 선택 객체 원복 후 메모리 DC 삭제 - 메모리 누수 방지
            SelectObject(hMemDC, hOldBmp);
            DeleteDC(hMemDC);
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

// main 함수: 콘솔 애플리케이션 형태의 진입점
int main() {
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
	CMultiThreadServer server;

	nRet = server.StartServer(9000);
	if (nRet < 0) {
		printf("server.StartServer() failed, nRet:%d\n", nRet);
	}

	return nRet;
}
