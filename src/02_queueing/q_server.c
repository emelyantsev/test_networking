#include "nwhelper.h"

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

int main() 
{
    #if defined(_WIN32)
    
    WSADATA d;

    int wsa_result = WSAStartup( MAKEWORD( 2, 2 ), &d );
    
    if( wsa_result != 0 ) {
        fprintf( stderr, "WSAStartup failed with error: %d\n", wsa_result );
        return 1;
    }
    
    #endif

    int iResult;

    SOCKET ListenSocket;
    SOCKET ClientSocket;

    struct addrinfo hints;

    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;

    memset( &hints, 0, sizeof( hints ) );
    
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;


    struct addrinfo* bind_address = NULL;

    // Resolve the server address and port
    iResult = getaddrinfo( NULL, DEFAULT_PORT, &hints, &bind_address );

    if( iResult != 0 ) {

        fprintf( stderr, "getaddrinfo failed with error: %d\n", iResult );
        CLEANUP();
        return 1;
    }

    // Create a SOCKET for the server to listen for client connections.
    ListenSocket = socket( bind_address->ai_family, bind_address->ai_socktype, bind_address->ai_protocol );
    
    if( !ISVALIDSOCKET( ListenSocket ) ) {

        fprintf( stderr, "socket failed with error: %d\n", GETSOCKETERRNO() );
        freeaddrinfo( bind_address );
        CLEANUP();
        return 1;
    }

    // Setup the TCP listening socket
    iResult = bind( ListenSocket, bind_address->ai_addr, (int) bind_address->ai_addrlen );
    
    if( iResult == SOCKET_ERROR ) {

        fprintf( stderr, "bind failed with error: %d\n", GETSOCKETERRNO() );
        freeaddrinfo( bind_address );
        CLOSESOCKET( ListenSocket );
        CLEANUP();
        return 1;
    }

    freeaddrinfo( bind_address );

    iResult = listen( ListenSocket, SOMAXCONN );

    if( iResult == SOCKET_ERROR ) {

        fprintf( stderr, "listen failed with error: %d\n", GETSOCKETERRNO() );
        CLOSESOCKET( ListenSocket );
        CLEANUP();
        return 1;
    }


    while ( 1 ) {

        // Accept a client socket
        ClientSocket = accept( ListenSocket, NULL, NULL ); // while handling current client others are waiting in queue

        if( ClientSocket == INVALID_SOCKET ) {
            fprintf( stderr, "accept failed with error: %d\n", GETSOCKETERRNO() );
            CLOSESOCKET( ListenSocket );
            CLEANUP();
            return 1;
        }

        int iSendResult;

        // Receive until the peer shuts down the connection
        do {

            iResult = recv( ClientSocket, recvbuf, recvbuflen, 0 );
            
            if( iResult > 0 ) {
            
                printf( "Bytes received: %d\n", iResult );

                // Echo the buffer back to the sender
                iSendResult = send( ClientSocket, recvbuf, iResult, 0 );
            
                if( iSendResult == SOCKET_ERROR ) {
            
                    fprintf( stderr, "send failed with error: %d\n", GETSOCKETERRNO() );

                    CLOSESOCKET( ClientSocket );
                    CLEANUP();
                    return 1;
                }

                printf( "Bytes sent: %d\n", iSendResult );
            } 
            else if( iResult == 0 ) {

                printf( "Connection closing...\n" );
            }
            else {
            
                fprintf( stderr, "recv failed with error: %d\n", GETSOCKETERRNO() );
                CLOSESOCKET( ClientSocket );
                CLEANUP();
                return 1;
            }

        } while( iResult > 0 );

        // shutdown the connection since we're done
        #if defined(_WIN32)
        iResult = shutdown( ClientSocket, SD_SEND );
        #else
        iResult = shutdown( ClientSocket, SHUT_WR );
        #endif

        if( iResult == SOCKET_ERROR ) {

            fprintf( stderr, "shutdown failed with error: %d\n", GETSOCKETERRNO() );

            CLOSESOCKET( ClientSocket );
            CLEANUP();
            return 1;
        }

        CLOSESOCKET( ClientSocket );
    }

    CLOSESOCKET( ListenSocket );
    CLEANUP();

    return 0;
}