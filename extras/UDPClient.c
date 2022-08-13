#ifdef _WIN32
//If we are on a windows machine, use this.
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
//If we aren't, use this.
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

int userChoiceMenu = -1;

void sendPacket (int userChoiceMenu);
void printIntro (void);
int printMenu (void);
int initialization( struct sockaddr ** internet_address, socklen_t * internet_address_length );
void execution( int internet_socket, struct sockaddr * internet_address, socklen_t internet_address_length );
void cleanup( int internet_socket, struct sockaddr * internet_address );

int main( int argc, char * argv[] )
{
    printIntro();
	while (1)
	{
		userChoiceMenu = printMenu();
		sendPacket(userChoiceMenu);
	}

	return 0;
}

int printMenu (void)
{
    int userChoice;
    printf("What are we doing today boss?\n");
    printf("[1] Just send me some damn packets!\n");
    printf("- Choose the content of the packet, how many of them to send, the ip and port of the server you want to flood.\n");
	printf("-(i) You set the ip and port.");
    scanf("%d",&userChoice);
    return userChoice;
}

void sendPacket (userChoiceMenu)
{
	OSInit;
	struct sockaddr * internet_address = NULL;
	socklen_t internet_address_length = 0;

	if (userChoiceMenu == 1)
	{
		system("cls");
		printf("Enter the ip and port you want to flood.\n");
	}
	

	int internet_socket = initialization( &internet_address, &internet_address_length );
	execution( internet_socket, internet_address, internet_address_length );
	cleanup( internet_socket, internet_address );
}

int initialization( struct sockaddr ** internet_address, socklen_t * internet_address_length )
{
	struct addrinfo internet_address_setup;
	struct addrinfo * internet_address_result;
	memset( &internet_address_setup, 0, sizeof internet_address_setup );
	internet_address_setup.ai_family = AF_UNSPEC;
	internet_address_setup.ai_socktype = SOCK_DGRAM;
	int getaddrinfo_return = getaddrinfo( "::1", "24042", &internet_address_setup, &internet_address_result );
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
	int number_of_bytes_send = 0;
	number_of_bytes_send = sendto( internet_socket, "Hello UDP world!", 16, 0, internet_address, internet_address_length );
	if( number_of_bytes_send == -1 )
	{
		perror( "sendto" );
	}

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
		printf( "Received : %s\n", buffer );
	}
}

void cleanup( int internet_socket, struct sockaddr * internet_address )
{
	free( internet_address );
	close( internet_socket );
    OSCleanup();
}

void printIntro (void)
{
    system("color 0A");
    system("cls");
    printf("\n\n");
    printf("    /$$      /$$           /$$                                                     /$$                    \n");
    printf("   | $$  /$ | $$          | $$                                                    | $$                    \n");
    printf("   | $$ /$$$| $$  /$$$$$$ | $$  /$$$$$$$  /$$$$$$  /$$$$$$/$$$$   /$$$$$$        /$$$$$$    /$$$$$$       \n");
    printf("   | $$/$$ $$ $$ /$$__  $$| $$ /$$_____/ /$$__  $$| $$_  $$_  $$ /$$__  $$      |_  $$_/   /$$__  $$      \n");
    printf("   | $$$$_  $$$$| $$$$$$$$| $$| $$      | $$  \\ $$| $$ \\ $$ \\ $$| $$$$$$$$        | $$    | $$  \\ $$      \n");
    printf("   | $$$/ \\  $$$| $$_____/| $$| $$      | $$  | $$| $$ | $$ | $$| $$_____/        | $$ /$$| $$  | $$      \n");
    printf("   | $$/   \\  $$|  $$$$$$$| $$|  $$$$$$$|  $$$$$$/| $$ | $$ | $$|  $$$$$$$        |  $$$$/|  $$$$$$/      \n");
    printf("   |__/     \\__/ \\_______/|__/ \\_______/ \\______/ |__/ |__/ |__/ \\_______/         \\___/   \\______/       \n");
    printf("\n\n\n");                                                                                           
    printf("                       $$\\ $$$$$$$$\\ $$\\   $$\\ $$\\                                    \n");
    printf("                       $$ |\\____$$  |$$ |  $$ |$  |                                   \n");
    printf("   $$\\   $$\\  $$$$$$\\  $$ |    $$  / $$ |  $$ |\\_/$$$$$$$\\                            \n");
    printf("   \\$$\\ $$  |$$  __$$\\ $$ |   $$  /  $$$$$$$$ |  $$  _____|                           \n");
    printf("    \\$$$$  / $$$$$$$$ |$$ |  $$  /   \\_____$$ |  \\$$$$$$\\                             \n");
    printf("    $$  $$<  $$   ____|$$ | $$  /          $$ |   \\____$$\\                            \n");
    printf("   $$  /\\$$\\ \\$$$$$$$\\ $$ |$$  /           $$ |  $$$$$$$  |                           \n");
    printf("   \\__/  \\__| \\_______|\\__|\\__/            \\__|  \\_______/                            \n");
    printf("\n");
    printf("\n");
    printf("\n");
    printf("   $$\\   $$\\ $$$$$$$\\  $$$$$$$\\         $$$$$$\\  $$\\ $$\\                      $$\\     \n");
    printf("   $$ |  $$ |$$  __$$\\ $$  __$$\\       $$  __$$\\ $$ |\\__|                     $$ |    \n");
    printf("   $$ |  $$ |$$ |  $$ |$$ |  $$ |      $$ /  \\__|$$ |$$\\  $$$$$$\\  $$$$$$$\\ $$$$$$\\   \n");
    printf("   $$ |  $$ |$$ |  $$ |$$$$$$$  |      $$ |      $$ |$$ |$$  __$$\\ $$  __$$\\_$$  _|  \n");
    printf("   $$ |  $$ |$$ |  $$ |$$  ____/       $$ |      $$ |$$ |$$$$$$$$ |$$ |  $$ | $$ |    \n");
    printf("   $$ |  $$ |$$ |  $$ |$$ |            $$ |  $$\\ $$ |$$ |$$   ____|$$ |  $$ | $$ |$$\\ \n");
    printf("   \\$$$$$$  |$$$$$$$  |$$ |            \\$$$$$$  |$$ |$$ |\\$$$$$$$\\ $$ |  $$ | \\$$$$  |\n");
    printf("    \\______/ \\_______/ \\__|             \\______/ \\__|\\__| \\_______|\\__|  \\__|  \\____/ \n");
    printf("\n\n\n");
    system("pause");
    system("cls");
}