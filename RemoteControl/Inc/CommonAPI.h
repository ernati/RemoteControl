#pragma once
#include <WinSock2.h>
#include <cstdint>

// 소켓에서 정확히 len 바이트를 받을 때까지 반복 호출
bool recvn(SOCKET sock, void* buf, size_t len);

// 소켓으로 정확히 totalBytes 바이트를 전송
int sendn(SOCKET sock, const char* buffer, int totalBytes);
