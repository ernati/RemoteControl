#include <WinSock2.h>
#include <cstdint>

// sock�κ��� ��Ȯ�� len ����Ʈ�� ���� ������ �ݺ� ȣ��
bool recvn(SOCKET sock, void* buf, size_t len);

int sendn(SOCKET sock, const char* buffer, int totalBytes);
