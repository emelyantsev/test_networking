#include "socket_handlers.h"

#define DEFAULT_BUFLEN 512

nwkit::Task handleSocket( SOCKET sock )
{
	std::cout << "Handle socket started" << std::endl;

	co_return;
}


nwkit::Task echoHandle( SOCKET sock )
{

    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;

    int iResult;
    int iSendResult;

    // Receive until the peer shuts down the connection
    do {

        iResult = co_await nwkit::Reader( sock, (char*) recvbuf, recvbuflen );

        if( iResult > 0 ) {
            printf( "Bytes received: %d\n", iResult );

            // Echo the buffer back to the sender
            iSendResult = co_await nwkit::Writer( sock, recvbuf, iResult);

            if( iSendResult == SOCKET_ERROR ) {

                printf( "send failed with error: %d\n", GETSOCKETERRNO() );
                CLOSESOCKET( sock );

                co_return;
            }
            printf( "Bytes sent: %d\n", iSendResult );

        } else if( iResult == 0 )

            printf( "Client connection closing...\n" );
        else {
            printf( "recv failed with error: %d\n", GETSOCKETERRNO() );
            CLOSESOCKET( sock );
            co_return;
        }

    } while( iResult > 0 );


    // shutdown the connection since we're done
    #if defined(_WIN32)
    iResult = shutdown( sock, SD_SEND );
    #else
    iResult = shutdown( sock, SHUT_WR );
    #endif

    if( iResult == SOCKET_ERROR ) {
        printf( "shutdown failed with error: %d\n", GETSOCKETERRNO() );
        CLOSESOCKET( sock );
        co_return;
    }

    // cleanup
    CLOSESOCKET( sock );
}