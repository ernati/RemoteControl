#pragma once

#include <WinSock2.h>
#pragma comment(lib, "ws2_32")

// Message 구조체: BITMAPINFOHEADER와 픽셀 데이터 크기를 저장하고, 그 뒤에 픽셀 데이터가 연속됨
#pragma pack(push, 1)
struct Message {
    BITMAPINFOHEADER bmiHeader; // 비트맵 정보 헤더 (가로, 세로, 비트수 등) - 40바이트
    DWORD pixelDataSize;             // 픽셀 데이터 크기 (바이트 단위) - 4바이트
    char pixelData[15000000];// 그 뒤에 픽셀 데이터가 이어집니다. - 나머지
};
#pragma pack(pop)

#define SIZE_OF_BITMAPINFOHEADER 40
#define SIZE_OF_DWORD 4

