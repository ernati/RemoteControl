#include <CMultiThreadClient.h>

int main() {
	int nRet = 0;
	CMultiThreadClient client;

	nRet = client.StartClient(9000);
	if (nRet < 0) {
		printf("client.StartClient() failed, nRet:%d\n", nRet);
	}

	return nRet;
}

