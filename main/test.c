int initializationHttpReq();

void executionHttpReq( int );

void cleanupHttpReq( int );

void getHttpReq();

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
		printf( "\rReceived : %s\n\n", buffer );
	}
	number_of_bytes_received = recv( internet_socket, buffer, 10000, 0 );
	if( number_of_bytes_received == -1 )
	{
		perror( "recv" );
	}
	else
	{	
		printf( "\rReceived : %s\n\n", buffer );
	}
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
	int internet_socket = initialization();

	executionHttpReq( internet_socket );

	cleanupHttpReq( internet_socket );
}
