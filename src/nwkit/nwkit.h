#pragma once

#include <string>
#include <iostream>
#include <unordered_set>
#include <memory>
#include <cassert>
#include <utility>
#include <vector>

#include "coro_tools.h"

namespace nwkit {


class TcpServer {

	typedef Task ( *TaskFunction )(SOCKET sock );

public:

	TcpServer( int port_ = 27015 );

	~TcpServer();

	int Run();

	void SetServingFunction( TaskFunction func );

	static Task DoNothingTaskFunction( SOCKET sock );

private:

	void init();
	void finish();
	void run();
	void checkSockets();
	void clearHandlers();

private:

	TaskFunction taskFunction = DoNothingTaskFunction;

	#if defined(_WIN32)
	WSADATA wsaData;
	#endif

	SOCKET ListenSocket = INVALID_SOCKET;

	std::unordered_set<SOCKET> read_sockets;
	std::unordered_set<SOCKET> write_sockets;

	std::map<SOCKET, Task > socketHandlers;

	struct addrinfo* result = NULL;
	struct addrinfo hints;

	std::string port;

	std::vector<SOCKET> tasks;

};

}