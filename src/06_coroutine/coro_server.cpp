#include "nwkit.h"
#include "socket_handlers.h"

int main()
{

	nwkit::TcpServer server;
	server.SetServingFunction( &echoHandle );

	return server.Run();
}