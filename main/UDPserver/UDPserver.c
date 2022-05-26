#ifdef _WIN32
	#define _WIN32_WINNT _WIN32_WINNT_WIN7
	#include <winsock2.h> //for all socket programming
	#include <ws2tcpip.h> //for getaddrinfo, inet_pton, inet_ntop
	#include <stdio.h> //for fprintf, perror
	#include <unistd.h> //for close
	#include <stdlib.h> //for exit
	#include <string.h> //for memset
	#include <time.h> //for clock
	#include <float.h>
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
	#define perror(string) fprintf(stderr, string ": WSA errno = %d\n", WSAGetLastError())
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

//Used to keep the amount of packets received
int numberOfPacketsReceived = 0;

//Why are these global? I'll explain later. Mostly lazyness though.
double stats1Avg[3];
double stats2Avg[3];
double stats3Avg[3];

int initialization();
void execution( int internet_socket );
void cleanup( int internet_socket );

int main( int argc, char * argv[] )
{

	OSInit();

	int internet_socket = initialization();

	execution( internet_socket );

	cleanup( internet_socket );

	OSCleanup();

	return 0;
}

//Gets us a socket and binds it.
int initialization()
{
	struct addrinfo internet_address_setup;
	struct addrinfo * internet_address_result;
	memset( &internet_address_setup, 0, sizeof internet_address_setup );
	internet_address_setup.ai_family = AF_INET;
	internet_address_setup.ai_socktype = SOCK_DGRAM;
	internet_address_setup.ai_flags = AI_PASSIVE;
	int getaddrinfo_return = getaddrinfo( NULL, "20000", &internet_address_setup, &internet_address_result );
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

//This is where the magic happens.
void execution( int internet_socket )
{	
	//Make two files, one for the receivedPackets and one for the statisticalData. Shoudnt be hard to figure out which ones which.
	FILE *OUTPUTFILE;
    OUTPUTFILE = fopen("receivedPackets.csv", "w+");
	FILE *OUTPUTFILESTATS;
    OUTPUTFILESTATS = fopen("statisticalData.csv", "w+");

	//Used to know if the user wants unlimited or a set amount of packets.
	int userChoice = 0;
	
	//Saves numbers of bytes reveived and the client message.
	int number_of_bytes_received = 0;
	char buffer[1000];
	struct sockaddr_storage client_internet_address;
	socklen_t client_internet_address_length = sizeof client_internet_address;

	//This is the same as typing ipconfig into CMD.
	system("ipconfig");

	//Print intro and ask the user what they want to do. Saves their choice in userchoice.
	printf("\n\n--------------------------------------");
	printf("\nServer started. Find your IPV4 IP above.\n");
	printf("\nWhat do you want to do?\n");
	printf("[ 1 ] - Receive unlimited packets. (With no timeout on packets)\n");
	printf("[ 2 ] - Set the amount of packets to receive.\n");
	printf("Enter your choice: ");
	scanf("%d",&userChoice);

	//If the user chose unlimited packets do this.
	if (userChoice == 1)
	{	
	
	//Print what the user chose in the file, and in terminal.
	fprintf(OUTPUTFILESTATS,"User chose unlimited packets.\n");
	printf("\nYou chose unlimited. Program will not stop unless you force it too with ctrl+c.\n");

		
		/*	
		*	While(1) because we want to be in a unlimited loop of receiving, and sending a confirmation.
		*	I didn't add any features like timeout, parsing, ... to this. This is a barebones receive and send confirmation loop. Why? I'm lazy.
		*	What did you expect, a for(;;)?
		*/
		while(1)
		{	
			//Receive the packet and prints it. Also saves it to the text file.
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

			//Send a confirmation.
			int number_of_bytes_send = 0;
			number_of_bytes_send = sendto( internet_socket, "PACKET RECEIVED", 16, 0, (struct sockaddr *) &client_internet_address, client_internet_address_length );
			if( number_of_bytes_send == -1 )
			{
				perror( "sendto" );
			}
		}
	}

	//If the user chose a set amount of packets we can parse it, and give them a timeout.
	else if (userChoice == 2)
	{	
		/*
		*	These are to read the data of the string sent from the phone app, save them into workable doubles.
		*	Example of a string sent from the phone app and also tells you where i save them.
		*
		*	232.75048, 3,  5.364, -3.278, 2.443, 4, -1.222, 0.001, -6.664, 5, -465.654, -654.654, 150.644
		*	trash,trash,stats1[0],stats1[1],stats1[2],trash,stats2[0],stats2[1],stats2[2],trash,stats3[0],stats3[1],stats3[2]
		*/

		double trash[5];	//Dont need this shit.
		double stats1[3];
		double stats2[3];
		double stats3[3];

		//Here i save the max. Initilazed to -DBL_MAX because otherwise i dont know what it gets inititialed to. (-DBL_MAX is the biggest negative number i can save in a double).
		//I initialize it to this so i can do checks to it.
		double stats1Max[3] = {-DBL_MAX,-DBL_MAX,-DBL_MAX,};
		double stats2Max[3] = {-DBL_MAX,-DBL_MAX,-DBL_MAX,};
		double stats3Max[3] = {-DBL_MAX,-DBL_MAX,-DBL_MAX,};

		//Here is where i save the min. Initilazed to DBL_MAX because otherwise i dont know what it gets inititialed to. (DBL_MAX is the biggest positive number i can save in a double).
		double stats1Min[3] = {DBL_MAX,DBL_MAX,DBL_MAX,};
		double stats2Min[3] = {DBL_MAX,DBL_MAX,DBL_MAX,};
		double stats3Min[3] = {DBL_MAX,DBL_MAX,DBL_MAX,};

		/*
		*	But why is my average global?
		*	I do my calucations in a for loop, and they get lost if i dont make them global. (I do my final calculation of them outside the for loop).
		*	Yes this is a terrible way of doing it and lots of shit can go wrong, but i dont care.
		*	Fuck pointers.
		*/
		
		int packetLossCounterTimeout = 0;
		//	I fuck with packetLossCounterTimeout so i made another one that saves it original value.
		int packetLossCounterTimeoutPrint = 0;
		double packetLossCounterTimeoutPercentage = 0.0;
		int amountOfPacketsToReceive = 0;
		//	Same story for this one.
		int amountOfPacketsToReceivePrint = 0;
		int timeout = 10000;
		int userChoiceTimeout = 0;
		fprintf(OUTPUTFILESTATS,"User chose a set amount of packets to receive.\n");

		printf("\n\nDo you want to set a custom timeout? (Standard is 10 seconds.)\n");
		printf("[ 1 ] - Yes, i want to set a custom timeout.\n");
		printf("[ 2 ] - No, 10 seconds is fine.\n");
		printf("Enter your choice: ");
		scanf("%d",&userChoiceTimeout);


		if (userChoiceTimeout == 1)
		{
			printf("\nEnter custom time in seconds: ");
			scanf("%d",&timeout);
			fprintf(OUTPUTFILESTATS,"User chose a custom timeout of %d seconds.\n",timeout);
			//Gets saved in ms, but we asked seconds so *1000 for ms.
			timeout = timeout *1000;
		}
		else if (userChoiceTimeout == 2)
		{	
			fprintf(OUTPUTFILESTATS,"User chose the standard timeout of 10 seconds.\n",timeout);
			//Do nothing
		}
		else
		{
			fprintf(OUTPUTFILESTATS,"User chose a wrong option in timeout selection. Stopping now.\n");
			printf("\n\n\n-----------------------------------------\n");
			printf("ERROR: please choose either 1 or 2.\n");
			printf("Restart the program.\n");
			exit(-1);
		}

		printf("\nHow many packets do you want to receive?: ");
		scanf("%d",&amountOfPacketsToReceive);
		fprintf(OUTPUTFILESTATS,"User chose to receive %d packets.\n",amountOfPacketsToReceive);
		amountOfPacketsToReceivePrint = amountOfPacketsToReceive;
		
		//Begin my cock! Oops, clock.
		clock_t begin = clock();

    	if (setsockopt(internet_socket, SOL_SOCKET, SO_RCVTIMEO,&timeout,sizeof(timeout)) < 0) 
		{
        perror("Error");
    	}

		while (amountOfPacketsToReceive != 0)
		{
			number_of_bytes_received = recvfrom( internet_socket, buffer, ( sizeof buffer ) - 1, 0, (struct sockaddr *) &client_internet_address, &client_internet_address_length );
			if( number_of_bytes_received == -1 )
			{
				perror( "recvfrom" );
				++packetLossCounterTimeout;
			}
			else
			{
				numberOfPacketsReceived++;
				buffer[number_of_bytes_received] = '\0';
				printf( "Packet [ %d ]: %s\n",amountOfPacketsToReceive, buffer);
				fprintf(OUTPUTFILE,"Packet [ %d ]: %s\n",amountOfPacketsToReceive, buffer);

				sscanf(buffer, "%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf",&trash,&trash,&stats1[0],&stats1[1],&stats1[2],&trash,&stats2[0],&stats2[1],&stats2[2],&trash,&stats3[0],&stats3[1],&stats3[2]);

				//Gets the max for stats1-3.
				for (size_t i = 0; i < 3; i++)
				{
					if (stats1[i] > stats1Max[i])
					{
						stats1Max[i] = stats1[i];
					}
					else
					{
						//Do nothing.
					}

					if (stats2[i] > stats2Max[i])
					{
						stats2Max[i] = stats2[i];
					}
					else
					{
						//Do nothing.
					}

					if (stats3[i] > stats3Max[i])
					{
						stats3Max[i] = stats3[i];
					}
					else
					{
						//Do nothing.
					}
				}

				//Gets the min for stats1-3.
				for (size_t i = 0; i < 3; i++)
				{
					if (stats1[i] < stats1Min[i])
					{
						stats1Min[i] = stats1[i];
					}
					else
					{
						//Do nothing.
					}

					if (stats2[i] < stats2Min[i])
					{
						stats2Min[i] = stats2[i];
					}
					else
					{
						//Do nothing.
					}
					
					if (stats3[i] < stats3Min[i])
					{
						stats3Min[i] = stats3[i];
					}
					else
					{
						//Do nothing.
					}
				}

				//Save every value in the average, you will see why later.
				for (size_t i = 0; i < 3; i++)
				{
					stats1Avg[i] = stats1Avg[i] + stats1[i];
					stats2Avg[i] = stats2Avg[i] + stats2[i];
					stats3Avg[i] = stats3Avg[i] + stats3[i];
				}
			}

			int number_of_bytes_send = 0;
			number_of_bytes_send = sendto( internet_socket, "PACKET RECEIVED", 16, 0, (struct sockaddr *) &client_internet_address, client_internet_address_length );
			if( number_of_bytes_send == -1 )
			{
				perror( "sendto" );
			}

			amountOfPacketsToReceive--;
		}

		//This is why.
		for (size_t i = 0; i < 3; i++)
		{
			stats1Avg[i] = stats1Avg[i] / numberOfPacketsReceived;
			stats2Avg[i] = stats2Avg[i] / numberOfPacketsReceived;
			stats3Avg[i] = stats3Avg[i] / numberOfPacketsReceived;
		}

		clock_t end = clock();
    	double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;

    	printf("\n\nTime between first and last packet: %f seconds.\n",time_spent);
		fprintf(OUTPUTFILESTATS,"\n\nTime between first and last packet: %f seconds.\n",time_spent);

		//Do my calulations for the timeout.
		packetLossCounterTimeoutPrint = packetLossCounterTimeout;
		packetLossCounterTimeoutPrint = amountOfPacketsToReceivePrint - packetLossCounterTimeoutPrint;
		packetLossCounterTimeoutPercentage = 100.0 * (amountOfPacketsToReceivePrint - packetLossCounterTimeoutPrint) / amountOfPacketsToReceivePrint;

		printf("You expected %d packets, due to timeouts you only received %d packets. That is a %.2f%% loss.\n",amountOfPacketsToReceivePrint,packetLossCounterTimeoutPrint,packetLossCounterTimeoutPercentage);
		fprintf(OUTPUTFILESTATS,"You expected %d packets, due to timeouts you only received %d packets. That is a %.2f%% loss.",amountOfPacketsToReceivePrint,packetLossCounterTimeoutPrint,packetLossCounterTimeoutPercentage,time_spent);

		printf("\nParsed values:\n\n");
		fprintf(OUTPUTFILESTATS,"\nParsed values:\n\n");

		printf("Bellow you find the parsed values for stats1:\n");
		printf("The max values:\n");
		for (int i = 0; i < 3; i++)		
		{
			printf("%lf, ",stats1Max[i]);
		}
		printf("\nThe min values:\n");
		for (int i = 0; i < 3; i++)		
		{
			printf("%lf, ",stats1Min[i]);
		}
		printf("\nThe average values:\n");
		for (int i = 0; i < 3; i++)		
		{
			printf("%lf, ",stats1Avg[i]);
		}

		printf("\n\nBellow you find the parsed values for stats2:\n");
		printf("The max values:\n");
		for (int i = 0; i < 3; i++)		
		{
			printf("%lf, ",stats2Max[i]);
		}
		printf("\nThe min values:\n");
		for (int i = 0; i < 3; i++)		
		{
			printf("%lf, ",stats2Min[i]);
		}
		printf("\nThe average values:\n");
		for (int i = 0; i < 3; i++)		
		{
			printf("%lf, ",stats2Avg[i]);
		}
		
		printf("\n\nBellow you find the parsed values for stats3:\n");
		printf("The max values:\n");
		for (int i = 0; i < 3; i++)		
		{
			printf("%lf, ",stats3Max[i]);
		}
		printf("\nThe min values:\n");
		for (int i = 0; i < 3; i++)		
		{
			printf("%lf, ",stats3Min[i]);
		}
		printf("\nThe average values:\n");
		for (int i = 0; i < 3; i++)		
		{
			printf("%lf, ",stats3Avg[i]);
		}

		//Now print them to the file.

		fprintf(OUTPUTFILESTATS,"Bellow you find the parsed values for stats1:\n");
		fprintf(OUTPUTFILESTATS,"The max values:\n");
		for (int i = 0; i < 3; i++)		
		{
			fprintf(OUTPUTFILESTATS,"%lf, ",stats1Max[i]);
		}
		fprintf(OUTPUTFILESTATS,"\nThe min values:\n");
		for (int i = 0; i < 3; i++)		
		{
			fprintf(OUTPUTFILESTATS,"%lf, ",stats1Min[i]);
		}
		fprintf(OUTPUTFILESTATS,"\nThe average values:\n");
		for (int i = 0; i < 3; i++)		
		{
			fprintf(OUTPUTFILESTATS,"%lf, ",stats1Avg[i]);
		}

		fprintf(OUTPUTFILESTATS,"\n\nBellow you find the parsed values for stats2:\n");
		fprintf(OUTPUTFILESTATS,"The max values:\n");
		for (int i = 0; i < 3; i++)		
		{
			fprintf(OUTPUTFILESTATS,"%lf, ",stats2Max[i]);
		}
		fprintf(OUTPUTFILESTATS,"\nThe min values:\n");
		for (int i = 0; i < 3; i++)		
		{
			fprintf(OUTPUTFILESTATS,"%lf, ",stats2Min[i]);
		}
		fprintf(OUTPUTFILESTATS,"\nThe average values:\n");
		for (int i = 0; i < 3; i++)		
		{
			fprintf(OUTPUTFILESTATS,"%lf, ",stats2Avg[i]);
		}
		
		fprintf(OUTPUTFILESTATS,"\n\nBellow you find the parsed values for stats3:\n");
		fprintf(OUTPUTFILESTATS,"The max values:\n");
		for (int i = 0; i < 3; i++)		
		{
			fprintf(OUTPUTFILESTATS,"%lf, ",stats3Max[i]);
		}
		fprintf(OUTPUTFILESTATS,"\nThe min values:\n");
		for (int i = 0; i < 3; i++)		
		{
			fprintf(OUTPUTFILESTATS,"%lf, ",stats3Min[i]);
		}
		fprintf(OUTPUTFILESTATS,"\nThe average values:\n");
		for (int i = 0; i < 3; i++)		
		{
			fprintf(OUTPUTFILESTATS,"%lf, ",stats3Avg[i]);
		}
	}

	else
	{
		fprintf(OUTPUTFILESTATS,"User chose a wrong option in option selection. Stopping now.\n");
		printf("\n\n\n-----------------------------------------\n");
		printf("ERROR: please choose either 1 or 2.\n");
		printf("Restart the program.\n");
		exit(-1);
	}
}

void cleanup( int internet_socket )
{
	close( internet_socket );
}