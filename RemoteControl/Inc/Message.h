#pragma once

#include <WinSock2.h>
#pragma comment(lib, "ws2_32")

// Message ����ü: BITMAPINFOHEADER�� �ȼ� ������ ũ�⸦ �����ϰ�, �� �ڿ� �ȼ� �����Ͱ� ���ӵ�
#pragma pack(push, 1)
struct Message {
    BITMAPINFOHEADER bmiHeader; // ��Ʈ�� ���� ��� (����, ����, ��Ʈ�� ��)
    DWORD dataSize;             // �ȼ� ������ ũ�� (����Ʈ ����)
    // �� �ڿ� �ȼ� �����Ͱ� �̾����ϴ�.
};
#pragma pack(pop)