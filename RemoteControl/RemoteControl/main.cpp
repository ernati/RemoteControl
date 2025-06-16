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

// ������ ���ν���: â�� �޽����� ó���մϴ�. - â ����, Ÿ�̸� ����, ȭ�� ĸ�� �� ������Ʈ
    // DispatchMessage �Լ����� ȣ���.
    // Ŭ���� ���� static CALLBACK ��� �Լ��� �����ϸ�, �� �Լ��� Ŭ������ �ν��Ͻ�(��, this ������)�� ���� ����.
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    //1. uMsg�� ���� ó��
    switch (uMsg) {
    case WM_CREATE:
        // Ÿ�̸� ����: ���� �ֱ�� ȭ�� ĸ�� �� ������Ʈ
        // SetTimer �Լ�: Ÿ�̸Ӹ� �����ϰ�, 3��° �Ķ����( CAPTURE_INTERVAL )�� ����� ������ ������ â�� �޽��� ť�� WM_TIMER �޽����� �߻���Ŵ.
        SetTimer(hwnd, 1, pCaptureScreen->GetCaptureInterval(), NULL);
        return 0;

    case WM_TIMER:
        // 1. ������ �ð����� ȭ�����

        if (RecvMode->m_hBitmap != NULL) {
            //printf("ȭ���� ����մϴ�!\n");
            InvalidateRect(hwnd, NULL, TRUE);
        }
        return 0;

    case WM_PAINT: {
        //1. BeginPaint - Window�� ���� ��ȿȭ��( �ٽ� �׷��� �ϴ� ) ������Ʈ ������ Ȯ���Ͽ�, �� ������ ������ PAINTSTRUCT ����ü�� ����.
        // ����, �ش� ������ �׸��� ���� DC �ڵ��� ��ȯ.
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        // 2. â Ŭ���̾�Ʈ ������ ��ǥ�� �˻�.
        // ���� �� ��ǥ(0, 0), ������ �Ʒ� ��ǥ(â�� �ʺ�, â�� ����)
        RECT rect;
        GetClientRect(hwnd, &rect);
        int winWidth = rect.right - rect.left;
        int winHeight = rect.bottom - rect.top;

        if (RecvMode->m_hBitmap != NULL) {

            //RecvMode->GetMutex();

            // 3. ȹ���� ���� DC�� ������ �Ӽ��� ������ DC�� �޸𸮿� �����ϰ�, �� �ڵ��� ȹ��. - �̹��� ó���� �޸� DC���� ����.
            // SelectObject - DC�� Ư�� GDI ��ü( ���⼭�� ��Ʈ�� )�� �����ϵ��� �Ͽ�, ���� �׸��� �۾��� �ش� ��ü�� �Ӽ��� ����ϵ��� ��.
            HDC hMemDC = CreateCompatibleDC(hdc);
            if (hMemDC == NULL) {
                printf("CreateCompatibleDC Fail!\n");
            }

            HBITMAP hOldBmp = (HBITMAP)SelectObject(hMemDC, RecvMode->m_hBitmap); // hOldBmp - ������ DC�� ���õǾ� �ִ� ��Ʈ�� ��ü�� �ڵ��� ����
            if (hOldBmp == NULL) {
                printf("SelectObject Fail!\n");
            }

            // 4. GetObject - ��Ʈ�� ������ ����( BITMAP ����ü )�� ����
            BITMAP bm;
            GetObject(RecvMode->m_hBitmap, sizeof(BITMAP), &bm);

            // 5. Src DC( Memory DC )�κ��� �ȼ� ������( ���� ������ ����� ĸ�ĵ� ȭ�� )�� Dest DC( ��ü ȭ�鿡 ���� Device Context )�� ������ ( SRCCOPY - �ܼ� ���� )
            // ��� �簢���� ũ�⿡ �°� Ȯ�� �Ǵ� ����Ͽ� ����
            StretchBlt(hdc, 0, 0, winWidth, winHeight, hMemDC, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);

            // 6. ���� ��ü ���� �� �޸� DC ���� - �޸� ���� ����
            SelectObject(hMemDC, hOldBmp);
            DeleteDC(hMemDC);

            //ReleaseMutex
            //RecvMode->ReleaseMutex_Custom();
        }

        //7. EndPaint - BeginPaint �Լ��κ��� ��ȯ�� DC �ڵ��� ����
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
    // �ܼ� �Ҵ� �� ǥ�� ��� ������
    AllocConsole();
    FILE* pCout;
    freopen_s(&pCout, "CONOUT$", "w", stdout);
    printf("�ܼ� â�� �Ҵ�Ǿ����ϴ�.\n");
    
    
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

    // 2. ������ Ŭ���� ���
    const wchar_t CLASS_NAME[] = L"ResponsiveScreenCaptureWindowClass";

    // 2.1 ������ Ŭ���� ����ü �ʱ�ȭ �� ���
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = g_printScreen->m_hInstance;
    wc.lpszClassName = CLASS_NAME;

    // 2.2 ������ Ŭ���� ��� ���� �� ���� �޽��� ��� �� ����
    if (!RegisterClass(&wc)) {
        std::cerr << "������ Ŭ���� ��� ����" << std::endl;
        return -1;
    }

    // 3. â ����
    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"Responsive Screen Capture Display",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, g_printScreen->m_nWidth, g_printScreen->m_nHeight,
        NULL,
        NULL,
        g_printScreen->m_hInstance,
        g_printScreen->m_captureScreen         // ������ �Ķ����
    );

    if (!hwnd) {
        std::cerr << "â ���� ����" << std::endl;
        return -1;
    }

    // 4. â ǥ��
    ShowWindow(hwnd, SW_SHOW);

    // 6. �޽��� ����
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;  // �����尡 ����� �� ��ȯ�� ��
}


//Capture�� ���� ThreadFunction
DWORD WINAPI ThreadFunctionSend(LPVOID lpParam)
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
        }

        //Sleep(100);
    }

    return 0;  // �����尡 ����� �� ��ȯ�� ��
}

// ���� ���
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
        // recv ���� myId �ϳ��� �ʿ�
        if (argc < 3) {
            PrintHelp(argv[0]);
            return -1;
        }
        uint32_t myId = static_cast<uint32_t>(std::stoul(argv[2]));

        RecvMode = new CRemoteControlRecvMode();

        // ĸó�� ���� Thread ����
        HANDLE hThread = CreateThread(
            NULL,          // �⺻ ���� �Ӽ�
            0,             // �⺻ ���� ũ��
            ThreadFunctionRecv, // ������ �Լ�
            NULL,         // ������ �Լ��� ������ �Ű�����
            0,             // ���� ��� ����
            NULL           // ������ ID (�ʿ� �� ����)
        );

        nRet = RecvMode->StartClient(25000, SERVICE_SERVER);
        if (nRet < 0) {
            printf("RecvMode.StartClient() failed, nRet:%d\n", nRet);
        }

    }
    else if (mode == "send") {
        // send ���� ID, targetId �� �� �ʿ�
        if (argc < 4) {
            PrintHelp(argv[0]);
            return -1;
        }
        uint32_t myId = static_cast<uint32_t>(std::stoul(argv[2]));
        uint32_t targetId = static_cast<uint32_t>(std::stoul(argv[3]));


        server = new CRemoteControlSendMode();

        // Window�� ���� Thread ����
        HANDLE hThread = CreateThread(
            NULL,          // �⺻ ���� �Ӽ�
            0,             // �⺻ ���� ũ��
            ThreadFunctionSend, // ������ �Լ�
            NULL,         // ������ �Լ��� ������ �Ű�����
            0,             // ���� ��� ����
            NULL           // ������ ID (�ʿ� �� ����)
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


