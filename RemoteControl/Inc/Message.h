#pragma once

#include <WinSock2.h>
#pragma comment(lib, "ws2_32")
#include <cstdint>

// 64비트 네트워크 바이트 순서 변환 함수들
// Windows에서는 htonll/ntohll이 정의되어 있지 않을 수 있음
#ifndef htonll
#define htonll(x) ((1==htonl(1)) ? (x) : ((uint64_t)htonl((x) & 0xFFFFFFFF) << 32) | htonl((x) >> 32))
#endif

#ifndef ntohll
#define ntohll(x) ((1==ntohl(1)) ? (x) : ((uint64_t)ntohl((x) & 0xFFFFFFFF) << 32) | ntohl((x) >> 32))
#endif

// 매직 넘버 상수 정의
constexpr uint32_t MESSAGE_MAGIC_NUMBER = 0x424D4350;  // "BMCP"

#pragma pack(push, 1)
struct Message {
    uint32_t magic;         // 메시지 식별용 매직 넘버 (예: 0x424D4350 = "BMCP")
    uint32_t checksum;      // 헤더와 페이로드에 대한 CRC32 등 체크섬
    uint32_t width;         // BITMAP.bmWidth
    uint32_t height;        // BITMAP.bmHeight
    uint16_t planes;        // BITMAP.bmPlanes
    uint16_t bitCount;      // BITMAP.bmBitsPixel
    uint32_t widthBytes;    // BITMAP.bmWidthBytes
    uint64_t payloadSize;   // 뒤따르는 픽셀 데이터 크기 (바이트 단위)

    // flexible array member — 실제 비트맵 비트 데이터는 이 뒤에 붙여 보냄
    uint8_t  payload[1];
};
#pragma pack(pop)

// checksum 계산 함수 - crc
uint32_t calculateCRC32(const uint8_t* data, size_t length);

/**
 * @brief  Message 버퍼를 new[]로 할당하여 반환
 * @param  bmp       GetObject로 얻은 BITMAP 구조체
 * @param  bits      비트맵 픽셀 데이터 포인터
 * @param  bitsSize  픽셀 데이터 크기(bytes)
 * @param  outTotal  생성된 전체 버퍼 크기(bytes) 반환
 * @return new로 할당된 Message 버퍼 (사용 후 delete[])
 */
uint8_t* createMessageBuffer_tmp(
    const BITMAP& bmp,
    const uint8_t* bits,
    size_t bitsSize,
    size_t& outTotal
);



// ——— Legacy
#pragma pack(push,1)
struct ConnRequestHeader {
    char     mode;       // 's' = send, 'r' = recv
    uint32_t myId;       // host-byte-order ID
    uint32_t target;     // host-byte-order targetId (only for 'r')
};
#pragma pack(pop)



// ——— 통합 요청 헤더 ———
struct FullRequestHeader {
    char     authId[32];    // null-terminated 사용자 ID
    char     authPw[32];    // null-terminated 비밀번호
    char     mode;          // 's' 또는 'r'
    uint32_t myId;          // 네트워크 바이트 순서
    uint32_t targetId;      // 네트워크 바이트 순서 (recv 모드일 때만 의미)
};

struct ConnResponse {
    uint8_t  success;    // 0 = fail, 1 = ok
    char     info[60];   // null-terminated 메시지
};