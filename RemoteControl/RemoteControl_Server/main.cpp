#include <CMultiThreadServer.h>

int main(void)
{
	int nRet = 0;
	CMultiThreadServer server;

	nRet = server.StartServer(9000);
	if (nRet < 0) {
		printf("server.StartServer() failed, nRet:%d\n", nRet);
	}

	return nRet;
}