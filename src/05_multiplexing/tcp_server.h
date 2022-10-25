#pragma once

#include <string>
#include <map>
#include <memory>
#include <set>
#include <vector>

#include "nwhelper.h"

#define DEFAULT_BUFLEN 512

class ClientHandler;

class TcpServer {

	public:

		TcpServer( int port = 27015 );

		int Run();

		~TcpServer();

	private:

		friend ClientHandler;

		void init();
		void finish();
		void run();
		void checkSockets();

		void deleteHandlerForSocket( SOCKET socket );

		struct TaskForHandler {
			SOCKET sock;
			bool ready_for_reading;
			bool ready_for_writing;
		};

	private:

        #if defined(_WIN32)
		WSADATA wsaData;
        #endif

		SOCKET ListenSocket = INVALID_SOCKET;

		std::string port;

		struct addrinfo* result = NULL;
		struct addrinfo hints;

		std::map<SOCKET, std::shared_ptr<ClientHandler> > clientHandlers;

		std::set<SOCKET> read_sockets;
		std::set<SOCKET> write_sockets;
		//std::unordered_set<SOCKET> except_sockets;

		std::vector<TaskForHandler> tasks;
};


class ClientHandler {

	public:

		ClientHandler( SOCKET socket_, TcpServer* ptrServer_ );

		void Handle( bool canRead, bool canWrite );

		enum class State {

			READING,
			WRITING,
			FINISH
		};

	private:

		void finish();

		SOCKET socket;
		TcpServer* ptrServer;

		State state;

		char recvbuf[DEFAULT_BUFLEN];
		int recvbuflen = DEFAULT_BUFLEN;
		int bytesToSend = 0;
		int bytesSent = 0;
};