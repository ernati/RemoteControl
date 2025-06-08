#include "Message.h"


// checksum ��� �Լ� - crc
uint32_t calculateCRC32(const uint8_t* data, size_t length) {
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
 * @brief  Message ���۸� new[]�� �Ҵ��Ͽ� ��ȯ
 * @param  bmp       GetObject�� ���� BITMAP ����ü
 * @param  bits      ��Ʈ�� �ȼ� ������ ������
 * @param  bitsSize  �ȼ� ������ ũ��(bytes)
 * @param  outTotal  ������ ��ü ���� ũ��(bytes) ��ȯ
 * @return new�� �Ҵ�� Message ���� (��� �� delete[])
 */
uint8_t* createMessageBuffer_tmp(
    const BITMAP& bmp,
    const uint8_t* bits,
    size_t bitsSize,
    size_t& outTotal
) {
    // flexible array ������ ��� ũ��
    const size_t headerSize = sizeof(Message) - sizeof(uint8_t);
    outTotal = headerSize + bitsSize;

    // new�� ���� �Ҵ�
    uint8_t* buf = new uint8_t[outTotal];
    Message* msg = reinterpret_cast<Message*>(buf);

    // 1) ��� �ʵ� ä��� (ȣ��Ʈ���Ʈ��ũ ����Ʈ ����)
    msg->magic = htonl(0x424D4350);          // 'BMCP'
    msg->width = htonl(bmp.bmWidth);
    msg->height = htonl(bmp.bmHeight);
    msg->planes = htons(bmp.bmPlanes);
    msg->bitCount = htons(bmp.bmBitsPixel);
    msg->widthBytes = htonl(bmp.bmWidthBytes);
    msg->payloadSize = htonll(bitsSize);

    // 2) ���̷ε� ����
    memcpy(msg->payload, bits, bitsSize);

    // 3) üũ�� ���: magic ���ĺ��� ������(header ���� magic) ����
    const uint8_t* csStart = buf + sizeof(msg->magic);
    size_t        csLen = outTotal - sizeof(msg->magic);
    uint32_t      crc = calculateCRC32(csStart, csLen);
    msg->checksum = htonl(crc);

    return buf;
}