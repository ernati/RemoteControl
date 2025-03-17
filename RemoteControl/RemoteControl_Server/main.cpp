#include <CSingleThreadServer.h>

int main(void)
{
	CSingleThreadServer server;
	server.StartServer(9000);

	return 0;
}