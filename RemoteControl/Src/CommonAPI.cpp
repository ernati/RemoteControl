#include "CommonAPI.h"
#include <iostream>

// 소켓에서 정확히 len 바이트를 받을 때까지 반복 호출
bool recvn(SOCKET sock, void* buf, size_t len) {
	if (sock == INVALID_SOCKET || buf == nullptr || len == 0) {
		return false;  // 잘못된 매개변수
	}

	uint8_t* ptr = reinterpret_cast<uint8_t*>(buf);
	size_t   received = 0;
	while (received < len) {
		int r = recv(sock, reinterpret_cast<char*>(ptr + received), int(len - received), 0);
		if (r == SOCKET_ERROR) {
			int error = WSAGetLastError();
			std::cerr << "recv 오류: " << error << std::endl;
			return false;
		}
		if (r == 0) {
			// 연결이 정상적으로 종료됨
			return false;
		}
		received += r;
	}
	return true;
}

int sendn(SOCKET sock, const char* buffer, int totalBytes) {
	if (sock == INVALID_SOCKET || buffer == nullptr || totalBytes <= 0) {
		return SOCKET_ERROR;  // 잘못된 매개변수
	}

	int totalSent = 0;
	while (totalSent < totalBytes) {
		int n = send(sock, buffer + totalSent, totalBytes - totalSent, 0);
		if (n == SOCKET_ERROR) {
			int error = WSAGetLastError();
			std::cerr << "send 오류: " << error << std::endl;
			return SOCKET_ERROR;
		}
		totalSent += n;
	}
	return totalSent;
}