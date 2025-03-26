#pragma once

#include <WinSock2.h>
#include <cstdio>
#pragma comment(lib, "ws2_32") //windows.h와 해당 코드는 공존 불가능
#include <iostream>


class CCaptureScreenAndSendBitMap
{

// Methods
public:
	CCaptureScreenAndSendBitMap();
	CCaptureScreenAndSendBitMap(UINT);
	~CCaptureScreenAndSendBitMap();
	HBITMAP CaptureScreen();
	void UpdateNewBitmap();

	HBITMAP GetnewBitmap() { return m_hnewBitmap; }
	HBITMAP GetoldBitmap() { return m_holdBitmap; }
	bool CheckSuccessCapture() { return m_holdBitmap != NULL; }
	bool CheckReadyForPrintScreen() { return m_hnewBitmap != NULL; }
	UINT GetCaptureInterval() { return CAPTURE_INTERVAL; }

private:

// Variables
public:

private:
	HBITMAP m_hnewBitmap;
	HBITMAP m_holdBitmap;
	UINT CAPTURE_INTERVAL;

};