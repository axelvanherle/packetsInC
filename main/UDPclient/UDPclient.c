#ifdef _WIN32
	#define _WIN32_WINNT _WIN32_WINNT_WIN7 //select minimal legacy support, needed for inet_pton, inet_ntop
	#include <winsock2.h> //for all socket programming
	#include <ws2tcpip.h> //for getaddrinfo, inet_pton, inet_ntop
	#include <stdio.h> //for fprintf
	#include <unistd.h> //for close
	#include <stdlib.h> //for exit
	#include <string.h> //for memset
#else
	#include <sys/socket.h> //for sockaddr, socket, socket
	#include <sys/types.h> //for size_t
	#include <netdb.h> //for getaddrinfo
	#include <netinet/in.h> //for sockaddr_in
	#include <arpa/inet.h> //for htons, htonl, inet_pton, inet_ntop
	#include <errno.h> //for errno
	#include <stdio.h> //for fprintf, perror
	#include <unistd.h> //for close
	#include <stdlib.h> //for exit
	#include <string.h> //for memset
#endif

/*
*
*   Code written by Axel Vanherle.
*   Sends a UDP packet to localhost on port 50000.
*
*/

void initialize (void);

void execute (void);

void cleanup (void);

int main()
{

    WSADATA wsaData;
	WSAStartup( MAKEWORD(2,0), &wsaData );

    struct addrinfo internet_address_setup;
	struct addrinfo *internet_address = NULL;
	memset( &internet_address_setup, 0, sizeof internet_address_setup );
	internet_address_setup.ai_family = AF_UNSPEC; //IPV4 or IPV6. We dont care.
	internet_address_setup.ai_socktype = SOCK_DGRAM; //UDP packet.
	getaddrinfo( "127.0.0.1", "50000", &internet_address_setup, &internet_address );

    int internet_socket;
	internet_socket = socket( internet_address->ai_family, internet_address->ai_socktype, internet_address->ai_protocol );

    sendto( internet_socket, "Hello UDP server!", 17, 0, internet_address->ai_addr, internet_address->ai_addrlen );

    int number_of_bytes_received = 0;
	char buffer[1000];
	socklen_t internet_address_length = internet_address->ai_addrlen;
	number_of_bytes_received = recvfrom( internet_socket, buffer, ( sizeof buffer ) - 1, 0, internet_address->ai_addr, &internet_address_length );
	buffer[number_of_bytes_received] = '\0';
	printf( "Received: %s\n", buffer );

    freeaddrinfo( internet_address );
	close( internet_socket );
    WSACleanup();
    printf("Compiler works!\n");
    return 0;

}

void initialize (void)
{
    
}

void execute (void)
{
    
}

void cleanup (void)
{
    
}