
/*
*
*   Code written by Axel Vanherle.
*   Sends a UDP packet to localhost on port 50000 via IPV4.
*
*/

#ifdef _WIN32
    
	#define _WIN32_WINNT _WIN32_WINNT_WIN7
	#include <winsock2.h> //for all socket programming
	#include <ws2tcpip.h> //for getaddrinfo, inet_pton, inet_ntop
	#include <stdio.h> //for fprintf, perror
	#include <unistd.h> //for close
	#include <stdlib.h> //for exit
	#include <string.h> //for memset
	void OSInit( void )
	{
		WSADATA wsaData;
		int WSAError = WSAStartup( MAKEWORD( 2, 0 ), &wsaData ); 
		if( WSAError != 0 )
		{
			fprintf( stderr, "WSAStartup errno = %d\n", WSAError );
			exit( -1 );
		}
	}
	void OSCleanup( void )
	{
		WSACleanup();
	}
    #define perror(string) fprintf( stderr, string ": WSA errno = %d\n", WSAGetLastError() )
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
	void OSInit( void ) {}
	void OSCleanup( void ) {}
#endif

int initialization( struct sockaddr ** internet_address, socklen_t * internet_address_length );
void execution( int internet_socket, struct sockaddr * internet_address, socklen_t internet_address_length );
void cleanup( int internet_socket, struct sockaddr * internet_address );

int main( int argc, char * argv[] )
{
	OSInit();

	struct sockaddr * internet_address = NULL;
	socklen_t internet_address_length = 0;
	int internet_socket = initialization( &internet_address, &internet_address_length );

	execution( internet_socket, internet_address, internet_address_length );

	cleanup( internet_socket, internet_address );

	OSCleanup();

	return 0;
}

int initialization( struct sockaddr ** internet_address, socklen_t * internet_address_length )
{
	struct addrinfo internet_address_setup;
	struct addrinfo * internet_address_result;
	memset( &internet_address_setup, 0, sizeof internet_address_setup );
	internet_address_setup.ai_family = AF_UNSPEC;
	internet_address_setup.ai_socktype = SOCK_DGRAM;
	int getaddrinfo_return = getaddrinfo( "192.168.0.101", "50000", &internet_address_setup, &internet_address_result );
	if( getaddrinfo_return != 0 )
	{
		fprintf( stderr, "getaddrinfo: %s\n", gai_strerror( getaddrinfo_return ) );
		exit( 1 );
	}

	int internet_socket = -1;
	struct addrinfo * internet_address_result_iterator = internet_address_result;
	while( internet_address_result_iterator != NULL )
	{
		internet_socket = socket( internet_address_result_iterator->ai_family, internet_address_result_iterator->ai_socktype, internet_address_result_iterator->ai_protocol );
		if( internet_socket == -1 )
		{
			perror( "socket" );
		}
		else
		{
			*internet_address_length = internet_address_result_iterator->ai_addrlen;
			*internet_address = (struct sockaddr *) malloc( internet_address_result_iterator->ai_addrlen );
			memcpy( *internet_address, internet_address_result_iterator->ai_addr, internet_address_result_iterator->ai_addrlen );
			break;
		}
		internet_address_result_iterator = internet_address_result_iterator->ai_next;
	}

	freeaddrinfo( internet_address_result );

	if( internet_socket == -1 )
	{
		fprintf( stderr, "socket: no valid socket address found\n" );
		exit( 2 );
	}

	return internet_socket;
}

void execution( int internet_socket, struct sockaddr * internet_address, socklen_t internet_address_length )
{
	int amountOfPacketsToSend = 0;
	int lenghtOfContentPacketToSend;
	char contentPacketToSend[180] = {};
	printf("How many packets do you want to send?\n");
	scanf("%d",&amountOfPacketsToSend);

	printf("What should the content of the packet be?\n");
	scanf("%s",&contentPacketToSend);

	//Adds NUL terminator to the end of the stings and gets the bytes for sendto function.
	lenghtOfContentPacketToSend = strlen(contentPacketToSend);
	contentPacketToSend[lenghtOfContentPacketToSend] = '\0';
	
	printf("\nSending %s %d times.\n",contentPacketToSend, amountOfPacketsToSend);


	int number_of_bytes_send = 0;
	number_of_bytes_send = sendto( internet_socket, contentPacketToSend, lenghtOfContentPacketToSend, 0, internet_address, internet_address_length );
	if( number_of_bytes_send == -1 )
	{
		perror( "sendto" );
	}
	printf("\n\nPacket [ 1 ] sent.\n");

	for (size_t i = 1; i < amountOfPacketsToSend; i++)
	{
		sendto( internet_socket, contentPacketToSend, lenghtOfContentPacketToSend, 0, internet_address, internet_address_length );
		printf("Packet [ %d ] sent.\n",i+1);
	}
	printf("All packets sent.\n\n");
	printf("Waitng for response from server...\n");

	int number_of_bytes_received = 0;
	char buffer[1000];
	number_of_bytes_received = recvfrom( internet_socket, buffer, ( sizeof buffer ) - 1, 0, internet_address, &internet_address_length );
	if( number_of_bytes_received == -1 )
	{
		perror( "recvfrom" );
	}
	else
	{
		buffer[number_of_bytes_received] = '\0';
		printf("Received : %s\n", buffer );
	}

	printf("Shutting down.");
}

void cleanup( int internet_socket, struct sockaddr * internet_address )
{
	free( internet_address );
	close( internet_socket );
}