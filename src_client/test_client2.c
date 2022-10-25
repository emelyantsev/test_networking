#include "nwhelper.h"

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

int main( int argc, char** argv )
{
    // Validate the parameters
    if( argc != 2 ) {
        printf( "usage: %s server-name\n", argv[0] );
        return 1;
    }

    #if defined(_WIN32)
    
    WSADATA d;

    int wsa_result = WSAStartup( MAKEWORD( 2, 2 ), &d );
    
    if( wsa_result != 0 ) {
        fprintf( stderr, "WSAStartup failed with error: %d\n", wsa_result );
        return 1;
    }
    
    #endif

    SOCKET ConnectSocket = INVALID_SOCKET;

    struct addrinfo* result = NULL,
        * ptr = NULL,
        hints;

    const char* sendbuf = "this is a test";

    char recvbuf[DEFAULT_BUFLEN];
    int iResult;
    int recvbuflen = DEFAULT_BUFLEN;

    memset( &hints, 0, sizeof( hints ) );

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    iResult = getaddrinfo( argv[1], DEFAULT_PORT, &hints, &result );
    
    if( iResult != 0 ) {

        fprintf( stderr, "getaddrinfo failed with error: %d\n", iResult );
        CLEANUP();
        return 1;
    }

    // Attempt to connect to an address until one succeeds
    for( ptr = result; ptr != NULL; ptr = ptr->ai_next ) {

        // Create a SOCKET for connecting to server
        ConnectSocket = socket( ptr->ai_family, ptr->ai_socktype,
            ptr->ai_protocol );

        if( ConnectSocket == INVALID_SOCKET ) {
            fprintf( stderr, "socket failed with error: %d\n", GETSOCKETERRNO() );
            CLEANUP();
            return 1;
        }

        // Connect to server.
        iResult = connect( ConnectSocket, ptr->ai_addr, (int) ptr->ai_addrlen );
        
        if( iResult == SOCKET_ERROR ) {
        
            CLOSESOCKET( ConnectSocket );
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo( result );

    if( ConnectSocket == INVALID_SOCKET ) {
        fprintf( stderr, "Unable to connect to server!\n" );
        CLEANUP();
        return 1;
    }
    

    int len = strlen( sendbuf );

    // Send an initial buffer
    iResult = send( ConnectSocket, sendbuf, (int) len / 2, 0 );

    if( iResult == SOCKET_ERROR ) {
        fprintf( stderr, "send failed with error: %d\n", GETSOCKETERRNO() );
        CLOSESOCKET( ConnectSocket );
        CLEANUP();
        return 1;
    }

    printf( "Bytes sent: %d\n", iResult );
    printf( "Press ENTER to continue\n");
    getchar();

    iResult = send( ConnectSocket, sendbuf + len/2, (int) len - len / 2, 0 );

    if( iResult == SOCKET_ERROR ) {
        fprintf( stderr, "send failed with error: %d\n", GETSOCKETERRNO() );
        CLOSESOCKET( ConnectSocket );
        CLEANUP();
        return 1;
    }

    printf( "Bytes Sent: %d\n", iResult );


    // shutdown the connection since no more data will be sent
    #if defined(_WIN32)
    iResult = shutdown( ConnectSocket, SD_SEND );
    #else
    iResult = shutdown( ConnectSocket, SHUT_WR );
    #endif

    if( iResult == SOCKET_ERROR ) {
        fprintf( stderr, "shutdown failed with error: %d\n", GETSOCKETERRNO() );
        CLOSESOCKET( ConnectSocket );
        CLEANUP();
        return 1;
    }

    // Receive until the peer closes the connection
    do {

        iResult = recv( ConnectSocket, recvbuf, recvbuflen, 0 );
        
        if( iResult > 0 ) {
        
            printf( "Bytes received: %d\n", iResult );
        }
        else if ( iResult == 0 ) {
        
            printf( "Connection closed\n" );
        }
        else {
        
            printf( "recv failed with error: %d\n", GETSOCKETERRNO() );
        }
    } 
    while ( iResult > 0 );

    // cleanup
    CLOSESOCKET( ConnectSocket );
    CLEANUP();

    return 0;
}