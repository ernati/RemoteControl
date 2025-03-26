#include <CCaptureScreenAndSendBitMap.h>

CCaptureScreenAndSendBitMap::CCaptureScreenAndSendBitMap()
{
    m_hnewBitmap = NULL;
	m_holdBitmap = NULL;
	CAPTURE_INTERVAL = 100;
}	

CCaptureScreenAndSendBitMap::CCaptureScreenAndSendBitMap(UINT capture_interval)
{
    m_hnewBitmap = NULL;
	m_holdBitmap = NULL;
	CAPTURE_INTERVAL = capture_interval;
}

CCaptureScreenAndSendBitMap::~CCaptureScreenAndSendBitMap()
{
	if (m_hnewBitmap) {
		DeleteObject(m_hnewBitmap);
	}
	if (m_holdBitmap) {
		DeleteObject(m_holdBitmap);
	}
}

HBITMAP CCaptureScreenAndSendBitMap::CaptureScreen()
{
    // 1. 데스크탑의 DC( 전체 화면에 대한 Device Context ) 획득
    HDC hScreenDC = GetDC(NULL);

    // 2. 획득한 기존 DC와 동일한 속성을 가지는 DC를 메모리에 생성하고, 그 핸들을 획득. - 이미지 처리는 메모리 DC에서 수행.
    HDC hMemoryDC = CreateCompatibleDC(hScreenDC);

    // 3. 기본 디스플레이 모니터의 화면 너비 및 높이(픽셀) 획득.
    int width = GetSystemMetrics(SM_CXSCREEN);
    int height = GetSystemMetrics(SM_CYSCREEN);

    // 4. 기본 디스플레이 모니터의 화면 너비 및 높이를 가지는 비트맵 생성.
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, width, height);
    if (!hBitmap) {
        DeleteDC(hMemoryDC);
        ReleaseDC(NULL, hScreenDC);
        return NULL;
    }

    // 5. SelectObject - DC가 특정 GDI 객체( 여기서는 비트맵 )을 선택하도록 하여, 이후 그리기 작업이 해당 객체의 속성을 사용하도록 함.
    SelectObject(hMemoryDC, hBitmap);

    // 6. Src DC( hScreenDC )로부터 픽셀 데이터( 여기서는 데스크탑 화면 )을 Dest DC( hMemoryDC )에 복사함 ( SRCCOPY - 단순 복사 )
    BitBlt(hMemoryDC, 0, 0, width, height, hScreenDC, 0, 0, SRCCOPY);

    // 7. 리소스 해제
    DeleteDC(hMemoryDC);
    ReleaseDC(NULL, hScreenDC);

    if (m_holdBitmap) {
        DeleteObject(m_holdBitmap);
		m_holdBitmap = NULL;
    }
    m_holdBitmap = hBitmap;

    return hBitmap;
}

void CCaptureScreenAndSendBitMap::UpdateNewBitmap()
{
	if (m_hnewBitmap) {
		DeleteObject(m_hnewBitmap);
		m_hnewBitmap = NULL;
	}

	m_hnewBitmap = m_holdBitmap;

	//if (m_holdBitmap) {               //Deleteobject 함수는 m_holdBitmap이 가리키는 비트맵을 삭제하고 m_holdBitmap을 NULL로 설정한다.
	//    DeleteObject(m_holdBitmap);   //즉, 해당 코드가 존재하면, m_hnewBitmap과 m_holdBitmap이 동일한 비트맵을 가리키게 되는 상태에서
	//    m_holdBitmap = NULL;          // m_holdBitmap을 삭제하게 되므로, m_hnewBitmap도 삭제되어버린다.
    //}

    return;
}