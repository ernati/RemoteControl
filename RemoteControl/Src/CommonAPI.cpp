#include "CommonAPI.h"


// sock�κ��� ��Ȯ�� len ����Ʈ�� ���� ������ �ݺ� ȣ��
bool recvn(SOCKET sock, void* buf, size_t len) {
	uint8_t* ptr = reinterpret_cast<uint8_t*>(buf);
	size_t   received = 0;
	while (received < len) {
		int r = recv(sock, reinterpret_cast<char*>(ptr + received), int(len - received), 0);
		if (r <= 0) return false;  // ���� Ȥ�� ���� ����
		received += r;
	}
	return true;
}

int sendn(SOCKET sock, const char* buffer, int totalBytes) {
	int totalSent = 0;
	while (totalSent < totalBytes) {
		int n = send(sock, buffer + totalSent, totalBytes - totalSent, 0);
		if (n == SOCKET_ERROR) {
			// ���� ó��: �ʿ��ϸ� WSAGetLastError()�� ���� �ڵ� Ȯ��
			return SOCKET_ERROR;
		}
		totalSent += n;
	}
	return totalSent;
}