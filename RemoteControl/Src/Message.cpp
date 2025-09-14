#include "Message.h"
#include <iostream>

// CRC32 체크섬 계산 함수
uint32_t calculateCRC32(const uint8_t* data, size_t length) {
    if (data == nullptr || length == 0) {
        return 0;  // 잘못된 입력에 대한 안전한 반환값
    }

    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < length; ++i) {
        crc ^= data[i];
        for (int bit = 0; bit < 8; ++bit) {
            if (crc & 1) crc = (crc >> 1) ^ 0xEDB88320;
            else         crc >>= 1;
        }
    }
    return ~crc;
}

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
) {
    // 입력 매개변수 검증
    if (bits == nullptr || bitsSize == 0) {
        std::cerr << "createMessageBuffer_tmp: 잘못된 입력 매개변수" << std::endl;
        outTotal = 0;
        return nullptr;
    }

    // flexible array 때문에 헤더 크기
    const size_t headerSize = sizeof(Message) - sizeof(uint8_t);
    outTotal = headerSize + bitsSize;

    // 메모리 할당 시도
    uint8_t* buf = nullptr;
    try {
        buf = new uint8_t[outTotal];
    }
    catch (const std::bad_alloc& e) {
        std::cerr << "createMessageBuffer_tmp: 메모리 할당 실패: " << e.what() << std::endl;
        outTotal = 0;
        return nullptr;
    }

    Message* msg = reinterpret_cast<Message*>(buf);

    // 1) 헤더 필드 채우기 (호스트바이트에서 네트워크 바이트 순서)
    msg->magic = htonl(MESSAGE_MAGIC_NUMBER);    // 상수 사용
    msg->width = htonl(bmp.bmWidth);
    msg->height = htonl(bmp.bmHeight);
    msg->planes = htons(bmp.bmPlanes);
    msg->bitCount = htons(bmp.bmBitsPixel);
    msg->widthBytes = htonl(bmp.bmWidthBytes);
    msg->payloadSize = htonll(bitsSize);

    // 2) 페이로드 복사
    memcpy(msg->payload, bits, bitsSize);

    // 3) 체크섬 계산: magic 제외한 나머지(header 및 magic) 대상
    const uint8_t* csStart = buf + sizeof(msg->magic);
    size_t        csLen = outTotal - sizeof(msg->magic);
    uint32_t      crc = calculateCRC32(csStart, csLen);
    msg->checksum = htonl(crc);

    return buf;
}