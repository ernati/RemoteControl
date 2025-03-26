#pragma once
#include <CCaptureScreenAndSendBitMap.h>
#include <CPrintScreen.h>
#include <CMultiThreadServer.h>
#include <iostream>

CCaptureScreenAndSendBitMap* pCaptureScreen = nullptr;
CPrintScreen* g_printScreen = nullptr;

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
        if (wParam == 1) {
            // 1. ������ �ð����� ȭ�� ĸ��
            pCaptureScreen->CaptureScreen();
            if (pCaptureScreen->CheckSuccessCapture()) {
                // 2. ���� ĸ�ĵ� �̹����� �ִٸ� ����
                pCaptureScreen->UpdateNewBitmap();

                // 3. ĸ���� �̹����� ���� ȭ�� ���� ��û, �ι�° �Ķ���Ͱ� NULL�̹Ƿ� ��ü ȭ�� ����
                // ���� ���� : InvalidateRect �Լ� - ������ â�� Ŭ���̾�Ʈ ���� �� �Ϻ�(�Ǵ� ��ü)�� ��ȿ ����(update region)���� ǥ���մϴ�.
                // ��ȿ ������ �����Ǹ�, Windows�� �޽��� ������ ���� ���¿� �������� �� �ڵ����� WM_PAINT �޽����� �ش� â�� �޽��� ť�� �Խ�.
                InvalidateRect(hwnd, NULL, TRUE);
            }
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

        if (pCaptureScreen->CheckReadyForPrintScreen()) {
            // 3. ȹ���� ���� DC�� ������ �Ӽ��� ������ DC�� �޸𸮿� �����ϰ�, �� �ڵ��� ȹ��. - �̹��� ó���� �޸� DC���� ����.
            // SelectObject - DC�� Ư�� GDI ��ü( ���⼭�� ��Ʈ�� )�� �����ϵ��� �Ͽ�, ���� �׸��� �۾��� �ش� ��ü�� �Ӽ��� ����ϵ��� ��.
            HDC hMemDC = CreateCompatibleDC(hdc);
            HBITMAP hOldBmp = (HBITMAP)SelectObject(hMemDC, pCaptureScreen->GetnewBitmap()); // hOldBmp - ������ DC�� ���õǾ� �ִ� ��Ʈ�� ��ü�� �ڵ��� ����

            // 4. GetObject - ��Ʈ�� ������ ����( BITMAP ����ü )�� ����
            BITMAP bm;
            GetObject(pCaptureScreen->GetnewBitmap(), sizeof(BITMAP), &bm);

            // 5. Src DC( Memory DC )�κ��� �ȼ� ������( ���� ������ ����� ĸ�ĵ� ȭ�� )�� Dest DC( ��ü ȭ�鿡 ���� Device Context )�� ������ ( SRCCOPY - �ܼ� ���� )
            // ��� �簢���� ũ�⿡ �°� Ȯ�� �Ǵ� ����Ͽ� ����
            StretchBlt(hdc, 0, 0, winWidth, winHeight, hMemDC, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);

            // 6. ���� ��ü ���� �� �޸� DC ���� - �޸� ���� ����
            SelectObject(hMemDC, hOldBmp);
            DeleteDC(hMemDC);
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

// main �Լ�: �ܼ� ���ø����̼� ������ ������
int main() {
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
	CMultiThreadServer server;

	nRet = server.StartServer(9000);
	if (nRet < 0) {
		printf("server.StartServer() failed, nRet:%d\n", nRet);
	}

	return nRet;
}
