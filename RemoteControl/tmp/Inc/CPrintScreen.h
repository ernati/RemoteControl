#pragma once
#include <WinSock2.h>
#include <cstdio>
#pragma comment(lib, "ws2_32")
#include <CCaptureScreenAndSendBitMap.h>

class CPrintScreen
{
//methods
public:
	CPrintScreen();
    CPrintScreen(HINSTANCE hInstance);
	~CPrintScreen();

	void setCaptureScreen(CCaptureScreenAndSendBitMap* captureScreen) { m_captureScreen = captureScreen; }
	void sethInstance(HINSTANCE hInstance) { m_hInstance = hInstance; }
private:


//variables
public:
	HINSTANCE m_hInstance;
	HWND m_hWnd;
	int m_nWidth;
	int m_nHeight;

	CCaptureScreenAndSendBitMap* m_captureScreen = NULL;

private:
};




