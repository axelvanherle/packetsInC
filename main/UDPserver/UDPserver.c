#ifdef _WIN32
	#define _WIN32_WINNT _WIN32_WINNT_WIN7
	#include <winsock2.h> //for all socket programming
	#include <ws2tcpip.h> //for getaddrinfo, inet_pton, inet_ntop
	#include <stdio.h> //for fprintf, perror
	#include <unistd.h> //for close
	#include <stdlib.h> //for exit
	#include <string.h> //for memset
	#include <time.h> //for clock
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
	#include <time.h> //for clock
	int OSInit( void ) {}
	int OSCleanup( void ) {}
#endif

int numberOfPacketsReceived = 0;

int initialization();
void execution( int internet_socket );
void cleanup( int internet_socket );

int main( int argc, char * argv[] )
{
	//////////////////
	//Initialization//
	//////////////////

	OSInit();

	int internet_socket = initialization();

	/////////////
	//Execution//
	/////////////

	execution( internet_socket );


	////////////
	//Clean up//
	////////////

	cleanup( internet_socket );

	OSCleanup();

	return 0;
}

int initialization()
{
	//Step 1.1
	struct addrinfo internet_address_setup;
	struct addrinfo * internet_address_result;
	memset( &internet_address_setup, 0, sizeof internet_address_setup );
	internet_address_setup.ai_family = AF_INET;
	internet_address_setup.ai_socktype = SOCK_DGRAM;
	internet_address_setup.ai_flags = AI_PASSIVE;
	int getaddrinfo_return = getaddrinfo( NULL, "24042", &internet_address_setup, &internet_address_result );
	if( getaddrinfo_return != 0 )
	{
		fprintf( stderr, "getaddrinfo: %s\n", gai_strerror( getaddrinfo_return ) );
		exit( 1 );
	}

	int internet_socket = -1;
	struct addrinfo * internet_address_result_iterator = internet_address_result;
	while( internet_address_result_iterator != NULL )
	{
		//Step 1.2
		internet_socket = socket( internet_address_result_iterator->ai_family, internet_address_result_iterator->ai_socktype, internet_address_result_iterator->ai_protocol );
		if( internet_socket == -1 )
		{
			perror( "socket" );
		}
		else
		{
			//Step 1.3
			int bind_return = bind( internet_socket, internet_address_result_iterator->ai_addr, internet_address_result_iterator->ai_addrlen );
			if( bind_return == -1 )
			{
				close( internet_socket );
				perror( "bind" );
			}
			else
			{
				break;
			}
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

void execution( int internet_socket )
{
	FILE *OUTPUTFILE;
    OUTPUTFILE = fopen("receivedPackets.csv", "w+");
	int userChoice = 0;
	int number_of_bytes_received = 0;
	char buffer[1000];
	struct sockaddr_storage client_internet_address;
	socklen_t client_internet_address_length = sizeof client_internet_address;

	system("ipconfig");
	printf("\n\n--------------------------------------");
	printf("\nServer started. Find your IPV4 IP above.\n");
	printf("\nWhat do you want to do?\n");
	printf("[ 1 ] - Receive unlimited packets.\n");
	printf("[ 2 ] - Set the amount of packets to receive.\n");
	printf("Enter your choice: ");
	scanf("%d",&userChoice);


	if (userChoice == 1)
	{	
		while(1)
		{
			int timeout = 10000;
   			if (setsockopt(internet_socket, SOL_SOCKET, SO_RCVTIMEO,&timeout,sizeof(timeout)) < 0) 
			{
        	perror("Error");
    		}
			number_of_bytes_received = recvfrom( internet_socket, buffer, ( sizeof buffer ) - 1, 0, (struct sockaddr *) &client_internet_address, &client_internet_address_length );
			if( number_of_bytes_received == -1 )
			{
				perror( "recvfrom" );
			}
			else
			{
				numberOfPacketsReceived++;
				buffer[number_of_bytes_received] = '\0';
				printf( "Packet [ %d ]: %s\n",numberOfPacketsReceived, buffer);
				fprintf(OUTPUTFILE,"Packet [ %d ]: %s\n",numberOfPacketsReceived, buffer);
			}

			int number_of_bytes_send = 0;
			number_of_bytes_send = sendto( internet_socket, "PACKET RECEIVED", 16, 0, (struct sockaddr *) &client_internet_address, client_internet_address_length );
			if( number_of_bytes_send == -1 )
			{
				perror( "sendto" );
			}
		}
	}

	else if (userChoice == 2)
	{
		int amountOfPacketsToReceive = 0;

		printf("\nHow many packets do you want to receive?: ");
		scanf("%d",&amountOfPacketsToReceive);

		clock_t begin = clock();
		while (amountOfPacketsToReceive != 0)
		{
			number_of_bytes_received = recvfrom( internet_socket, buffer, ( sizeof buffer ) - 1, 0, (struct sockaddr *) &client_internet_address, &client_internet_address_length );
			if( number_of_bytes_received == -1 )
			{
				perror( "recvfrom" );
			}
			else
			{
				numberOfPacketsReceived++;
				buffer[number_of_bytes_received] = '\0';
				printf( "Packet [ %d ]: %s\n",amountOfPacketsToReceive, buffer);
				fprintf(OUTPUTFILE,"Packet [ %d ]: %s\n",amountOfPacketsToReceive, buffer);
			}

			int number_of_bytes_send = 0;
			number_of_bytes_send = sendto( internet_socket, "PACKET RECEIVED", 16, 0, (struct sockaddr *) &client_internet_address, client_internet_address_length );
			if( number_of_bytes_send == -1 )
			{
				perror( "sendto" );
			}

			amountOfPacketsToReceive--;
		}

		clock_t end = clock();
    	double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;

    	printf("\n\nTime between first and last packet: %f seconds.",time_spent);
	}

	else
	{
		printf("\n\n\n-----------------------------------------\n");
		printf("ERROR: please choose either 1 or 2.\n");
		printf("Restart the program.\n");
		return 0;
	}
	

}

void cleanup( int internet_socket )
{
	//Step 3.1
	close( internet_socket );
}