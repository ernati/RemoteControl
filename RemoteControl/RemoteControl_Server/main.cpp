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
