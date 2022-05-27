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

// Whats code efficiency? We don't do that here.

#define PORT "9034"   // port we're listening on

char buf[256];    // This variable saves the client message.
int nbytes;		// This variable saves the amound of chars that are in the client message.
char latest16Messages[1000];    //This variable is used to store the latest 16 messages pulled from the HTTP server.

//These functions are to send whats in "buf" to the HTPP server.
int initializationMsg();
void executionMsg( int );
void cleanupMsg( int );
void sendMsgToHttp();

//These functions get the latest 16 messages on the HTPP server and print them to console. Used in the beginning of the code only, once the server launches its first start.
int initializationHttpReq();
void executionHttpReq( int );
void cleanupHttpReq( int );
void getHttpReq();

//These functions get the latest 16 messages on the HTPP server. And store them in "latest16messages" to forward them to a newly connected client.
int initializationHttpReqNoP();
void executionHttpReqNoP( int );
void cleanupHttpReqNoP( int );
void getHttpReqNoP();

//This function gets sockaddr, IPv4 or IPv6.
void *get_in_addr(struct sockaddr *sa);

void printIntro (void);

int main(void)
{
	OSInit();
	printIntro();
	//Gets latest 16 messages fromt he HTTP server.
	getHttpReq();
	printf("\n\n\nThose are the latest 16 messages saved on the HTTP server.\n");
	printf("Server is now waiting for incomming connections.\n");


    fd_set master;    // master file descriptor list
    fd_set read_fds;  // temp file descriptor list for select()
    int fdmax;        // maximum file descriptor number
    int listener;     // listening socket descriptor
    int newfd;        // newly accept()ed socket descriptor
    struct sockaddr_storage remoteaddr; // client address
    socklen_t addrlen;

    char remoteIP[INET6_ADDRSTRLEN];

    int yes=1;        // for setsockopt() SO_REUSEADDR, below
    int i, j, rv;

    struct addrinfo hints, *ai, *p;

    FD_ZERO(&master);    // clear the master and temp sets
    FD_ZERO(&read_fds);

    // get us a socket and bind it
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) 
	{
        fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
        exit(1);
    }
    
    for(p = ai; p != NULL; p = p->ai_next) 
	{
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0) 
		{ 
            continue;
        }
        
        // lose the pesky "address already in use" error message
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) 
		{
            close(listener);
            continue;
        }
        break;
    }

    // if we got here, it means we didn't get bound
    if (p == NULL) 
	{
        fprintf(stderr, "selectserver: failed to bind\n");
        exit(2);
    }

    freeaddrinfo(ai); // all done with this

    // listen
    if (listen(listener, 10) == -1) 
	{
        perror("listen");
        exit(3);
    }

    // add the listener to the master set
    FD_SET(listener, &master);

    // keep track of the biggest file descriptor
    fdmax = listener; // so far, it's this one

    // main loop
    for(;;) 
	{
        read_fds = master; // copy it
        if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1)
		{
            perror("select");
            exit(4);
        }

        // run through the existing connections looking for data to read
        for(i = 0; i <= fdmax; i++)
		{
            if (FD_ISSET(i, &read_fds))
			{ // we got one!!
                if (i == listener)
				{
                    // handle new connections
                    addrlen = sizeof remoteaddr;
                    newfd = accept(listener,(struct sockaddr *)&remoteaddr,&addrlen);

					//This gets the latest 16 messages from the HTTP server and forwards it to the new user.
					getHttpReqNoP();
					send(newfd, latest16Messages,strlen((latest16Messages)+1), 0);

                    if (newfd == -1) 
					{
                        perror("accept");
                    } 
					else
					{
                        FD_SET(newfd, &master); // add to master set
                        if (newfd > fdmax) 
						{   // keep track of the max
							fdmax = newfd;
                        }
                        printf("SERVER MESSAGE: new connection from %s on ""socket %d\n",inet_ntop(remoteaddr.ss_family,get_in_addr((struct sockaddr*)&remoteaddr),remoteIP, INET6_ADDRSTRLEN),newfd);

						//This code sends the IP and Socket from a newly connected client to all currently connected clients.
                        char newConMsg[256];
                        sprintf(newConMsg,"\nNew connection from ");
                        strcat(newConMsg, inet_ntop(remoteaddr.ss_family,get_in_addr((struct sockaddr*)&remoteaddr),remoteIP, INET6_ADDRSTRLEN));
                        strcat(newConMsg," on socket ");
                        char newfdString[4];
                        itoa(newfd,newfdString,10);
                        strcat(newConMsg, newfdString);
                        newConMsg[strlen(newConMsg)] = '\r';

                        for(j = 0; j <= fdmax; j++) 
						{  
                            // send to everyone!
                            if (FD_ISSET(j, &master)) 
							{   
                                // except the listener and ourselves
                                if (j != listener && j != i) 
								{	
									//Sends the string that was just crafted to all currently connected clients.
                                    if (send(j, newConMsg,strlen(newConMsg), 0) == -1) 
									{	
                                        perror("send");
                                    }
                                }
                                
                            }
                        }
                    }
                }
				else 
				{
                    // handle data from a client
                    if ((nbytes = recv(i, buf, sizeof buf, 0)) <= 0) 
					{
                        // got error or connection closed by client
                        if (nbytes == 0) 
						{
                            // connection closed
                            printf("SERVER MESSAGE: socket %d hung up\n", i);
                        } else 
						{
                            perror("recv");
                        }
                        close(i); // bye!
                        FD_CLR(i, &master); // remove from master set
                    } 
					else 
					{	
                        /*
						*	WAKE UP! We just got some data from a client. Let's do this shit.
						*/

						//Sends the message to the HTTP server. (Also manipulates the string, no fucking clue if this is needed but i dont want to spend more time on it).
						sendMsgToHttp();
						
                        for(j = 0; j <= fdmax; j++) 
						{  
                            // send to everyone!
                            if (FD_ISSET(j, &master)) 
							{   
                                // except the listener and ourselves
                                if (j != listener && j != i) 
								{	
									//This sends the client message to everyone. (Except for the listener and ourselves).
                                    if (send(j, buf, nbytes, 0) == -1) 
									{	
                                        perror("send");
                                    }
                                }
                                
                            }
                        }
                    }
                } // END handle data from client
            } // END got new incoming connection
        } // END looping through file descriptors
    } // END for(;;)--and you thought it would never end!

    OSCleanup();

    return 0;
}

//This function gets sockaddr, IPv4 or IPv6.
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) 
	{
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void printIntro(void)
{
	printf("========================================================================================\n\n");
		printf(" /$$$$$$                                                            \n");
		printf("/$$__  $$                                                           \n");
		printf(" $$  \\__/  /$$$$$$   /$$$$$$  /$$    /$$ /$$$$$$   /$$$$$$          \n");
		printf("  $$$$$$  /$$__  $$ /$$__  $$|  $$  /$$//$$__  $$ /$$__  $$         \n");
		printf("\\____  $$| $$$$$$$$| $$  \\__/ \\  $$/$$/| $$$$$$$$| $$  \\__/         \n");
		printf("/$$  \\ $$| $$_____/| $$        \\  $$$/ | $$_____/| $$               \n");
		printf("  $$$$$$/|  $$$$$$$| $$         \\  $/  |  $$$$$$$| $$               \n");
		printf("\\______/  \\_______/|__/          \\_/    \\_______/|__/               \n");
		printf("\n");
		printf("\n");
		printf("\n");
		printf(" /$$$$$$   /$$                           /$$                     /$$\n");
		printf("/$$__  $$ | $$                          | $$                    | $$\n");
		printf(" $$  \\__//$$$$$$    /$$$$$$   /$$$$$$  /$$$$$$    /$$$$$$   /$$$$$$$\n");
		printf("  $$$$$$|_  $$_/   |____  $$ /$$__  $$|_  $$_/   /$$__  $$ /$$__  $$\n");
		printf("\\____  $$ | $$      /$$$$$$$| $$  \\__/  | $$    | $$$$$$$$| $$  | $$\n");
		printf("/$$  \\ $$ | $$ /$$ /$$__  $$| $$        | $$ /$$| $$_____/| $$  | $$\n");
		printf("  $$$$$$/ |  $$$$/|  $$$$$$$| $$        |  $$$$/|  $$$$$$$|  $$$$$$$\n");
		printf("\\______/   \\___/   \\_______/|__/         \\___/   \\_______/ \\_______/\n\n");
	printf("========================================================================================\n\n");
}

/*
*	START OF SENDING MESSAGES TO THE HTTP SERVER.
*
*	This is just a regular TCP client.
*	Gets whatever is in buf, do some string manipulation then send it to the HTTP server.
*	More detail in the code itself.
*/

//Gets a socket and returns it.
int initializationMsg()
{
	struct addrinfo internet_address_setup;
	struct addrinfo * internet_address_result;
	memset( &internet_address_setup, 0, sizeof internet_address_setup );
	internet_address_setup.ai_family = AF_UNSPEC;
	internet_address_setup.ai_socktype = SOCK_STREAM;
	int getaddrinfo_return = getaddrinfo( "student.pxl-ea-ict.be", "80", &internet_address_setup, &internet_address_result );
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
			int connect_return = connect( internet_socket, internet_address_result_iterator->ai_addr, internet_address_result_iterator->ai_addrlen );
			if( connect_return == -1 )
			{
				perror( "connect" );
				close( internet_socket );
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

void executionMsg( int internet_socket )
{	
	//We're gonna be using this to send buf to the HTTP server.
	char contentPacketToSend[256];

	//Adds a NUL terminator to the end of the string, if this isnt done there is overflow.
	buf[nbytes] = '\0';

	//Saves buf into "contentPacketToSend", we can also just manipulate "buf" but since this fucntion is called before we send the message any changes done here
	//will also be send to the new client. We can ignore this and just manipulate "buf" if we are very lazy and don't care, but i did care.
	strcpy(contentPacketToSend,buf);

	//This for loop filters everything out of the string
	//(a-Z ; 1-9 can stay. Fuck the rest.)
	for (int i = 0, j; buf[i] != '\0'; ++i) 
	{
      // Enter the loop if the character is not an alphabet.
      // And if it isnt a NUL terminator.
      while (!(contentPacketToSend[i] >= 'a' && contentPacketToSend[i] <= 'z') && !(contentPacketToSend[i] >= 'A' && contentPacketToSend[i] <= 'Z') && !(contentPacketToSend[i] == '\0')) 
	  {
         for (j = i; contentPacketToSend[j] != '\0'; ++j) 
		 {
            // If jth element of line is not an alphabet, assign the value of (j+1)th element to the jth element.
            contentPacketToSend[j] = contentPacketToSend[j + 1];
         }
         contentPacketToSend[j] = '\0';
      }
   	}

	//This code crafts the HTTP get request. We could also do this with "contentPacketToSend" if we want to be memmory efficient, but i again dont care.
	char newConMsg[256];
    sprintf(newConMsg,"GET /chat.php?i=12345678&msg=");
    strcat(newConMsg, contentPacketToSend);
    strcat(newConMsg," HTTP/1.0\r\nHost: student.pxl-ea-ict.be\r\n\r\n");

	//You've got mail!
	int number_of_bytes_send = 0;
	number_of_bytes_send = send( internet_socket, newConMsg, 200, 0 );
	if( number_of_bytes_send == -1 )
	{
		perror( "send" );
	}

	/*
	* 	:O Where is my recv?
	*/
}

//Does cleanup.
void cleanupMsg( int internet_socket )
{	
	//Step 3.2
	int shutdown_return = shutdown( internet_socket, SD_SEND );
	if( shutdown_return == -1 )
	{
		perror( "shutdown" );
	}

	//Step 3.1
	close( internet_socket );
}

//This is the actual client, aka call this when you want to send buf to the HTTP server.
void sendMsgToHttp()
{
	int internet_socket = initializationMsg();

	executionMsg( internet_socket );

	cleanupMsg( internet_socket );

}

/*
*	START OF RECIVING MESSAGES TO THE HTTP SERVER.
*	*WITH PRINTING IT TO CONSOLE*
*
*	This is just a regular TCP client.
*	Sends a HTTP request and gets the latest 16 messages sent to console, then promtly prints it to the console.
*/

//Gets a socket and returns it.
int initializationHttpReq()
{
	//Step 1.1
	struct addrinfo internet_address_setup;
	struct addrinfo * internet_address_result;
	memset( &internet_address_setup, 0, sizeof internet_address_setup );
	internet_address_setup.ai_family = AF_INET;
	internet_address_setup.ai_socktype = SOCK_STREAM;
	int getaddrinfo_return = getaddrinfo( "student.pxl-ea-ict.be", "80", &internet_address_setup, &internet_address_result );
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
			int connect_return = connect( internet_socket, internet_address_result_iterator->ai_addr, internet_address_result_iterator->ai_addrlen );
			if( connect_return == -1 )
			{
				perror( "connect" );
				close( internet_socket );
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

void executionHttpReq( int internet_socket )
{	
	//Sends a HTTP request for the latest 16 messages aka history.
	int number_of_bytes_send = 0;
	number_of_bytes_send = send( internet_socket, "GET /history.php?i=12345678 HTTP/1.0\r\nHost: student.pxl-ea-ict.be\r\n\r\n", 77, 0 );
	if( number_of_bytes_send == -1 )
	{
		perror( "send" );
	}

	/*
	*	This recveives the latest 16 messages and prints it to console.
	*	"But why 2 recv?" - Sometimes the server sends the information in two packets.
	*	recv isnt blocking, so i dont care and i can do this.
	*/
	int number_of_bytes_received = 0;
	char buffer[1000];
	number_of_bytes_received = recv( internet_socket, buffer, ( sizeof buffer ) - 1, 0 );
	if( number_of_bytes_received == -1 )
	{
		perror( "recv" );
	}
	else
	{
		buffer[number_of_bytes_received] = '\0';
		printf( "%s\n\n", buffer );
	}
	number_of_bytes_received = recv( internet_socket, buffer, ( sizeof buffer ) - 1, 0 );
	if( number_of_bytes_received == -1 )
	{
		perror( "recv" );
	}
	else
	{
		buffer[number_of_bytes_received] = '\0';
		printf( "%s\n\n", buffer );
	}

	//All done? That was fast.
}

//Does cleanu.
void cleanupHttpReq( int internet_socket )
{
	//Step 3.2
	int shutdown_return = shutdown( internet_socket, SD_SEND );
	if( shutdown_return == -1 )
	{
		perror( "shutdown" );
	}

	//Step 3.1
	close( internet_socket );
}

//Call this fucntion when you want to get the latest 16 messages and print it to console.
void getHttpReq()
{
	int internet_socket = initializationHttpReq();

	executionHttpReq( internet_socket );

	cleanupHttpReq( internet_socket );
}

/*
*	START OF RECIVING MESSAGES TO THE HTTP SERVER.
*	*WITHOUT PRINTING IT TO CONSOLE*
*
*	This is just a regular TCP client.
*	Sends a HTTP request and gets the latest 16 messages, then promtly saves it into "latest16Messages".
*
*	"But why do i need another client to retreive the latest 16 messages?" - Well, because i'm lazy.
*	But really, it is because i'm lazy. This could be done in one client, but i didnt.
*	This gets the latest 16 messages and saves it, without printing.
*	The other client that gets the messages and prints it to console, but when sending them to a new client i dont want to print it everytime, hence this client.
*/

//BTW, NoP stands for No Print.

//Gets us a socket, you should know this by now.
int initializationHttpReqNoP()
{
	//Step 1.1
	struct addrinfo internet_address_setup;
	struct addrinfo * internet_address_result;
	memset( &internet_address_setup, 0, sizeof internet_address_setup );
	internet_address_setup.ai_family = AF_INET;
	internet_address_setup.ai_socktype = SOCK_STREAM;
	int getaddrinfo_return = getaddrinfo( "student.pxl-ea-ict.be", "80", &internet_address_setup, &internet_address_result );
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
			int connect_return = connect( internet_socket, internet_address_result_iterator->ai_addr, internet_address_result_iterator->ai_addrlen );
			if( connect_return == -1 )
			{
				perror( "connect" );
				close( internet_socket );
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

//Gets latest 16 messages and saves it into "latest16Messages".
void executionHttpReqNoP( int internet_socket )
{	
	//Step 2.1
	int number_of_bytes_send = 0;
	number_of_bytes_send = send( internet_socket, "GET /history.php?i=12345678 HTTP/1.0\r\nHost: student.pxl-ea-ict.be\r\n\r\n", 77, 0 );
	if( number_of_bytes_send == -1 )
	{
		perror( "send" );
	}

	int number_of_bytes_received = 0;
	char buffer[1000];

	number_of_bytes_received = recv( internet_socket, buffer, ( sizeof buffer ) - 1, 0 );
	if( number_of_bytes_received == -1 )
	{
		perror( "recv" );
	}
	else
	{	
		//We got some usefull data!
		//Gets buffer and saves it into (should i really name the var again?).
		//Also adds some syntax, because why not.
		buffer[number_of_bytes_received] = '\0';
		sprintf(latest16Messages,"\n=========== LATEST 16 MESSAGES FROM THE HTTPSERVER ===========\n%s\n==================================\n\n\n",buffer);
	}
	//Another recv, this one does nothing but i don't want to delete is.
	number_of_bytes_received = recv( internet_socket, buffer, ( sizeof buffer ) - 1, 0 );
	if( number_of_bytes_received == -1 )
	{
		perror( "recv" );
	}
	else
	{	
		buffer[number_of_bytes_received] = '\0';
	}
}

//Bla bla you know what this does.
void cleanupHttpReqNoP( int internet_socket )
{
	//Step 3.2
	int shutdown_return = shutdown( internet_socket, SD_SEND );
	if( shutdown_return == -1 )
	{
		perror( "shutdown" );
	}

	//Step 3.1
	close( internet_socket );
}

//Use this function to save the latest 16 messages into "latest16Messages".
void getHttpReqNoP()
{
	int internet_socket = initializationHttpReqNoP();

	executionHttpReqNoP( internet_socket );

	cleanupHttpReqNoP( internet_socket );
}

/*
*	That was is, why are you reading me? There clearly isnt code under me.
*	
*	
*	Really? I just told you there was no code left.
*	
*	
*	Coded by axel vanhere using stolen code written by bart and beej.
*/