#include "CommonAPI.h"


// sock로부터 정확히 len 바이트를 받을 때까지 반복 호출
bool recvn(SOCKET sock, void* buf, size_t len) {
	uint8_t* ptr = reinterpret_cast<uint8_t*>(buf);
	size_t   received = 0;
	while (received < len) {
		int r = recv(sock, reinterpret_cast<char*>(ptr + received), int(len - received), 0);
		if (r <= 0) return false;  // 에러 혹은 연결 종료
		received += r;
	}
	return true;
}

int sendn(SOCKET sock, const char* buffer, int totalBytes) {
	int totalSent = 0;
	while (totalSent < totalBytes) {
		int n = send(sock, buffer + totalSent, totalBytes - totalSent, 0);
		if (n == SOCKET_ERROR) {
			// 에러 처리: 필요하면 WSAGetLastError()로 에러 코드 확인
			return SOCKET_ERROR;
		}
		totalSent += n;
	}
	return totalSent;
}