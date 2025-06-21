#include <WinSock2.h>
#include <cstdint>

// sock로부터 정확히 len 바이트를 받을 때까지 반복 호출
bool recvn(SOCKET sock, void* buf, size_t len);

int sendn(SOCKET sock, const char* buffer, int totalBytes);
