#include "nwkit.h"

namespace nwkit {

TcpServer::TcpServer( int port_ ) : port( std::to_string( port_ ) ) {}


TcpServer::~TcpServer() {}


void TcpServer::SetServingFunction( TaskFunction func )
{
    taskFunction = func;
}



void TcpServer::init()
{
    #if defined(_WIN32)
    
    int wsa_result = WSAStartup( MAKEWORD( 2, 2 ), &wsaData );
    
    if( wsa_result != 0 ) {
        std::cerr << "WSAStartup failed with error: " << wsa_result << std::endl;
        exit(1);
    }
    
    #endif

    memset( &hints, 0, sizeof( hints ) );

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;


    int iResult = getaddrinfo( NULL, port.c_str(), &hints, &result );

    if( iResult != 0 ) {

        std::cerr << "getaddrinfo failed with error: " << iResult << std::endl;

        CLEANUP();
        exit( 1 );
    }

    ListenSocket = socket( result->ai_family, result->ai_socktype, result->ai_protocol );

    if( ListenSocket == INVALID_SOCKET ) {

        std::cerr << "socket failed with error: " << GETSOCKETERRNO() << std::endl;

        freeaddrinfo( result );
        CLEANUP();
        exit( 1 );
    }

    iResult = bind( ListenSocket, result->ai_addr, (int)result->ai_addrlen );

    if( iResult == SOCKET_ERROR ) {

        std::cerr << "bind failed with error: " << GETSOCKETERRNO() << std::endl;

        freeaddrinfo( result );

        CLOSESOCKET( ListenSocket );
        CLEANUP();
        exit( 1 );
    }

    freeaddrinfo( result );

    iResult = listen( ListenSocket, SOMAXCONN );

    if( iResult == SOCKET_ERROR ) {
        std::cerr << "listen failed with error: " << GETSOCKETERRNO() << std::endl;
        CLOSESOCKET( ListenSocket );
        CLEANUP();
        exit( 1 );
    }

}

void TcpServer::finish()
{
    CLOSESOCKET( ListenSocket );
    CLEANUP();
}

void TcpServer::run()
{
    while( true ) {

        checkSockets();

        for( const auto& sock : tasks ) {

            assert( socketHandlers.count( sock ) > 0 );
            
            socketHandlers.at(sock).resume();
        }

        clearHandlers();
    }
}

void TcpServer::checkSockets()
{
    read_sockets.clear();
    write_sockets.clear();

    fd_set read_sockets_fd;
    fd_set write_sockets_fd;

    FD_ZERO( &read_sockets_fd );
    FD_ZERO( &write_sockets_fd );

    FD_SET( ListenSocket, &read_sockets_fd );

    SOCKET max_socket = ListenSocket;

    for( const auto& p : global_read_sockets ) {

        read_sockets.insert( p.first );
        
        FD_SET( p.first, &read_sockets_fd );

        if (p.first > max_socket) {
            max_socket = p.first;
        }
    }

    for( const auto& p : global_write_sockets ) {

        write_sockets.insert( p.first );

        FD_SET( p.first, &write_sockets_fd );

        if (p.first > max_socket) {
            max_socket = p.first;
        }
    }


    if( select( max_socket + 1, &read_sockets_fd, &write_sockets_fd, 0, 0 ) < 0 ) {

        fprintf( stderr, "select() failed. (%d)\n", GETSOCKETERRNO() );

        exit( 1 );
    }

    if( FD_ISSET( ListenSocket, &read_sockets_fd ) ) {

        SOCKET newClientSocket = accept( ListenSocket, NULL, NULL );

        if( newClientSocket == INVALID_SOCKET ) {

            printf( "accept failed with error: %d\n", GETSOCKETERRNO() );

        } else {

            socketHandlers.emplace(std::make_pair( newClientSocket, std::move( taskFunction( newClientSocket ) ) ) );
            printf( "accept new client\n" );
        }
    }

    tasks.clear();

    for( const auto& read_sock : read_sockets ) {

        bool ready_for_reading = FD_ISSET( read_sock, &read_sockets_fd );

        if( ready_for_reading  ) {

            tasks.push_back( read_sock  );
            global_read_sockets.erase( read_sock );
        }
    }

    for( const auto& write_sock : write_sockets ) {

        bool ready_for_writing = FD_ISSET( write_sock, &write_sockets_fd );

        if( ready_for_writing ) {

            tasks.push_back( write_sock );
            global_write_sockets.erase( write_sock );
        }
    }
}


int TcpServer::Run()
{
    init();

    run();

    finish();

    return 0;
}

void TcpServer::clearHandlers()
{
    std::vector<SOCKET> doneHandlers;

    for( auto& p : socketHandlers ) {

        if( p.second.done() ) {

            doneHandlers.push_back( p.first );
        }
    }

    for( SOCKET sock : doneHandlers ) {

        socketHandlers.erase( sock );
    }
}

Task TcpServer::DoNothingTaskFunction( SOCKET sock )
{
    CLOSESOCKET( sock );
    co_return;
}

}
