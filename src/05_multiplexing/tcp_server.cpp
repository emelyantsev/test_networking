#include "tcp_server.h"

#include <iostream>
#include <cassert>

TcpServer::TcpServer( int port_ ) : port( std::to_string( port_ ) ) {}

TcpServer::~TcpServer() {}

int TcpServer::Run() {

    init();
    run();
    finish();

	return 0;
}


void TcpServer::init() {

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

void TcpServer::finish() {

    CLEANUP();
    CLOSESOCKET( ListenSocket );
}

void TcpServer::run()
{
    while( true ) {

        checkSockets();

        for( const auto& task : tasks ) {

            assert( clientHandlers.count( task.sock ) > 0 );

            clientHandlers[task.sock]->Handle( task.ready_for_reading, task.ready_for_writing );
        }
    }
}

void TcpServer::checkSockets()
{
    read_sockets.clear();
    write_sockets.clear();
    //except_sockets.clear();

    fd_set read_sockets_fd;
    fd_set write_sockets_fd;

    FD_ZERO( &read_sockets_fd );
    FD_ZERO( &write_sockets_fd );

    FD_SET( ListenSocket, &read_sockets_fd );

    SOCKET max_socket = ListenSocket;

    for( const auto& p : clientHandlers ) {

        read_sockets.insert( p.first );
        write_sockets.insert( p.first );

        FD_SET( p.first, &read_sockets_fd );
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
            
            fprintf( stderr, "accept failed with error: %d\n", GETSOCKETERRNO() );   
        }
        else {

            clientHandlers[newClientSocket] = std::make_shared<ClientHandler>( newClientSocket, this );
            printf( "accept new client\n" );
        }
    }

    tasks.clear();

    for( const auto& read_sock : read_sockets ) {

        bool ready_for_reading = FD_ISSET( read_sock, &read_sockets_fd );
        bool ready_for_writing = write_sockets.count( read_sock ) > 0 && FD_ISSET( read_sock, &write_sockets_fd );

        if (ready_for_reading || ready_for_writing ) {

            tasks.push_back( { read_sock, ready_for_reading, ready_for_writing } );
        }
    }

    for( const auto& write_sock : write_sockets ) {

        if( read_sockets.count( write_sock ) > 0 ) {
            continue;
        }

        bool ready_for_reading = false;
        bool ready_for_writing = FD_ISSET( write_sock, &write_sockets_fd );

        if( ready_for_writing ) {
            tasks.push_back( { write_sock, ready_for_reading, ready_for_writing } );
        }

   }

}


void TcpServer::deleteHandlerForSocket( SOCKET socket )
{
    clientHandlers.erase( socket );

}

ClientHandler::ClientHandler( SOCKET socket_, TcpServer* ptrServer_ ) : socket(socket_), ptrServer(ptrServer_), state(State::READING) {}

void ClientHandler::Handle( bool canRead, bool canWrite )
{

    if( state == State::READING && canRead ) {


        int iResult = recv( socket, recvbuf, recvbuflen, 0 );

        if( iResult > 0 ) {
            
            printf( "Bytes received: %d\n", iResult );

            bytesToSend = iResult;
            bytesSent = 0;
            state = State::WRITING;
        }
        else if( iResult == 0 ) {

            printf( "Client connection closing...\n" );
            finish();
        }
        else {
            
            fprintf( stderr, "recv failed with error: %d\n", GETSOCKETERRNO() );   
            finish();
        }
    }
    else if( state == State::WRITING && canWrite ) {

        int iSendResult = send( socket, recvbuf + bytesSent, bytesToSend - bytesSent, 0 );

        if( iSendResult == SOCKET_ERROR ) {
            fprintf( stderr, "send failed with error: %d\n", GETSOCKETERRNO() );  
            finish();
        }
        else {

            bytesSent += iSendResult;

            if (bytesSent < bytesToSend ) {
                state = State::WRITING;
            }
            else {
                printf( "Bytes sent: %d\n", iSendResult );
                state = State::READING;
            }
        }
    }
}


void ClientHandler::finish()
{
    state = State::FINISH;
    CLOSESOCKET( socket );
    ptrServer->deleteHandlerForSocket( socket );
}