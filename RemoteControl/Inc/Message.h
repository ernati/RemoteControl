#pragma once

#include <WinSock2.h>
#pragma comment(lib, "ws2_32")

// Message 구조체: BITMAPINFOHEADER와 픽셀 데이터 크기를 저장하고, 그 뒤에 픽셀 데이터가 연속됨
#pragma pack(push, 1)
struct Message {
    BITMAPINFOHEADER bmiHeader; // 비트맵 정보 헤더 (가로, 세로, 비트수 등)
    DWORD dataSize;             // 픽셀 데이터 크기 (바이트 단위)
    // 그 뒤에 픽셀 데이터가 이어집니다.
};
#pragma pack(pop)