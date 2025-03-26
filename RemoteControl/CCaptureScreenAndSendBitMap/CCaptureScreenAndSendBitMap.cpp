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
    // 1. ����ũž�� DC( ��ü ȭ�鿡 ���� Device Context ) ȹ��
    HDC hScreenDC = GetDC(NULL);

    // 2. ȹ���� ���� DC�� ������ �Ӽ��� ������ DC�� �޸𸮿� �����ϰ�, �� �ڵ��� ȹ��. - �̹��� ó���� �޸� DC���� ����.
    HDC hMemoryDC = CreateCompatibleDC(hScreenDC);

    // 3. �⺻ ���÷��� ������� ȭ�� �ʺ� �� ����(�ȼ�) ȹ��.
    int width = GetSystemMetrics(SM_CXSCREEN);
    int height = GetSystemMetrics(SM_CYSCREEN);

    // 4. �⺻ ���÷��� ������� ȭ�� �ʺ� �� ���̸� ������ ��Ʈ�� ����.
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, width, height);
    if (!hBitmap) {
        DeleteDC(hMemoryDC);
        ReleaseDC(NULL, hScreenDC);
        return NULL;
    }

    // 5. SelectObject - DC�� Ư�� GDI ��ü( ���⼭�� ��Ʈ�� )�� �����ϵ��� �Ͽ�, ���� �׸��� �۾��� �ش� ��ü�� �Ӽ��� ����ϵ��� ��.
    SelectObject(hMemoryDC, hBitmap);

    // 6. Src DC( hScreenDC )�κ��� �ȼ� ������( ���⼭�� ����ũž ȭ�� )�� Dest DC( hMemoryDC )�� ������ ( SRCCOPY - �ܼ� ���� )
    BitBlt(hMemoryDC, 0, 0, width, height, hScreenDC, 0, 0, SRCCOPY);

    // 7. ���ҽ� ����
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

	//if (m_holdBitmap) {               //Deleteobject �Լ��� m_holdBitmap�� ����Ű�� ��Ʈ���� �����ϰ� m_holdBitmap�� NULL�� �����Ѵ�.
	//    DeleteObject(m_holdBitmap);   //��, �ش� �ڵ尡 �����ϸ�, m_hnewBitmap�� m_holdBitmap�� ������ ��Ʈ���� ����Ű�� �Ǵ� ���¿���
	//    m_holdBitmap = NULL;          // m_holdBitmap�� �����ϰ� �ǹǷ�, m_hnewBitmap�� �����Ǿ������.
    //}

    return;
}