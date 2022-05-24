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
#define PORT "9034"   // port we're listening on

char buf[256];    // buffer for client data

//These functions are to send the buf to the HTPP server.
int initializationMsg();
void executionMsg( int );
void cleanupMsg( int );
void sendMsgToHttp();

//These functions get the latest 16 messages on the HTPP server.
int initializationHttpReq();
void executionHttpReq( int );
void cleanupHttpReq( int );
void getHttpReq();

//This function gets sockaddr, IPv4 or IPv6.
void *get_in_addr(struct sockaddr *sa);

int main(void)
{
	OSInit();
	strcpy(buf,"Test");
	sendMsgToHttp();
	//Gets latest 16 messages.
	getHttpReq();
	printf("Server is now waiting for incomming connections.\n");


    fd_set master;    // master file descriptor list
    fd_set read_fds;  // temp file descriptor list for select()
    int fdmax;        // maximum file descriptor number
    int listener;     // listening socket descriptor
    int newfd;        // newly accept()ed socket descriptor
    struct sockaddr_storage remoteaddr; // client address
    socklen_t addrlen;

    int nbytes;

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

					/*
					*	HIER MOET DE CODE NAAR DE NIEUWE CLIENT.
					*/
					//send(newfd, "BLABLA",6,0);

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

						//Sends that a new client got connected to everyone.
                        char newConMsg[256];
                        sprintf(newConMsg,"New connection from ");
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
                        // we got some data from a client
                        for(j = 0; j <= fdmax; j++) 
						{  
                            // send to everyone!
                            if (FD_ISSET(j, &master)) 
							{   
                                // except the listener and ourselves
                                if (j != listener && j != i) 
								{
                                    if (send(j, buf, nbytes, 0) == -1) 
									{	
                                        perror("send");
                                    }

									sendMsgToHttp();

									//buf[nbytes] = '\0';


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

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) 
	{
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

/*
*
*
*   START OF CODE TO SEND MESSAGE TO THE HTTP SERVER.
*
*
*/

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
	int lenghtOfContentPacketToSend;
	char contentPacketToSend[256]; 

	for (int i = 0, j; buf[i] != '\0'; ++i) 
	{
      // enter the loop if the character is not an alphabet
      // and not the null character
      while (!(buf[i] >= 'a' && buf[i] <= 'z') && !(buf[i] >= 'A' && buf[i] <= 'Z') && !(buf[i] == '\0')) 
	  {
         for (j = i; buf[j] != '\0'; ++j) 
		 {
            // if jth element of line is not an alphabet,
            // assign the value of (j+1)th element to the jth element
            buf[j] = buf[j + 1];
         }
         buf[j] = '\0';
      }
   }

	strcpy(contentPacketToSend,buf);

	char newConMsg[256];
	memset(newConMsg,0,strlen(newConMsg));
    sprintf(newConMsg,"GET /chat.php?i=12345678&msg=");
    strcat(newConMsg, contentPacketToSend);
    strcat(newConMsg," HTTP/1.0\r\nHost: student.pxl-ea-ict.be\r\n\r\n");

	int number_of_bytes_send = 0;
	number_of_bytes_send = send( internet_socket, newConMsg, 200, 0 );
	if( number_of_bytes_send == -1 )
	{
		perror( "send" );
	}
}

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

void sendMsgToHttp()
{
	int internet_socket = initializationMsg();

	executionMsg( internet_socket );

	cleanupMsg( internet_socket );

}

/*
*
*
*   END OF CODE TO SEND MESSAGE TO THE HTTP SERVER.
*
*
*/

/*
*
*
*   START OF CODE TO RECEIVE MESSAGE TO THE HTTP SERVER.
*
*
*/
int initializationHttpReq()
{
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

void executionHttpReq( int internet_socket )
{	
	printf("\n=========== LATEST 16 MESSAGES FROM THE HTTPSERVER ===========\n");
	int number_of_bytes_send = 0;
	number_of_bytes_send = send( internet_socket, "GET /history.php?i=12345678 HTTP/1.0\r\nHost: student.pxl-ea-ict.be\r\n\r\n", 77, 0 );
	if( number_of_bytes_send == -1 )
	{
		perror( "send" );
	}

	int number_of_bytes_received = 0;
	char buffer[10000];
	number_of_bytes_received = recv( internet_socket, buffer, 10000, 0 );
	if( number_of_bytes_received == -1 )
	{
		perror( "recv" );
	}
	else
	{	
		printf( "%s\n\n", buffer );
	}
	printf("\n==================================\n");
}

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

void getHttpReq()
{
	int internet_socket = initializationHttpReq();

	executionHttpReq( internet_socket );

	cleanupHttpReq( internet_socket );
}

/*
*
*
*   END OF CODE TO RECEIVE MESSAGE TO THE HTTP SERVER.
*
*
*/