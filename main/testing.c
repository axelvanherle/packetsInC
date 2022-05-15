#include <winsock2.h> //for all socket programming
#include <ws2tcpip.h> //for getaddrinfo, inet_pton, inet_ntop
#include <stdio.h> //for fprintf, perror
#include <unistd.h> //for close
#include <stdlib.h> //for exit
#include <string.h> //for memset
#include <time.h>

int main()
{
	int lenghtOfContentPacketToSend;
	char contentPacketToSend[180];

	printf("What should the content of the packet be?\n");
    gets(contentPacketToSend);

    printf("%s\n",contentPacketToSend);

	//Adds NUL terminator to the end of the stings and gets the bytes for sendto function.
	lenghtOfContentPacketToSend = strlen(contentPacketToSend);
	contentPacketToSend[lenghtOfContentPacketToSend] = '\0';
	
	printf("\n%s %d\n",contentPacketToSend, lenghtOfContentPacketToSend);
}
