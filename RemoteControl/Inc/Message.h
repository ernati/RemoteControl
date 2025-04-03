#pragma once

#include <WinSock2.h>
#pragma comment(lib, "ws2_32")

// Message ����ü: BITMAPINFOHEADER�� �ȼ� ������ ũ�⸦ �����ϰ�, �� �ڿ� �ȼ� �����Ͱ� ���ӵ�
#pragma pack(push, 1)
struct Message {
    BITMAPINFOHEADER bmiHeader; // ��Ʈ�� ���� ��� (����, ����, ��Ʈ�� ��) - 40����Ʈ
    DWORD pixelDataSize;             // �ȼ� ������ ũ�� (����Ʈ ����) - 4����Ʈ
    char pixelData[15000000];// �� �ڿ� �ȼ� �����Ͱ� �̾����ϴ�. - ������
};
#pragma pack(pop)

#define SIZE_OF_BITMAPINFOHEADER 40
#define SIZE_OF_DWORD 4

