#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>

#define SERVER_PORT 9000
#define BUF_SIZE 1024

void erro(char *msg);

int main(int argc, char *argv[])
{

	char endServer[100];
	int fd, nread;
	struct sockaddr_in addr;
	struct hostent *hostPtr;
	char buffer[1024];

	if (argc != 4)
	{
		printf("cliente <host> <port> <string>\n");
		exit(-1);
	}

	strcpy(endServer, argv[1]);

	if ((hostPtr = gethostbyname(endServer)) == 0)
		erro("Nao consegui obter endereço");

	bzero((void *) &addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = ((struct in_addr *)(hostPtr->h_addr))->s_addr;
	addr.sin_port = htons((short) atoi(argv[2]));

	if((fd = socket(AF_INET,SOCK_STREAM,0)) == -1)
		erro("socket");
	if( connect(fd,(struct sockaddr *)&addr,sizeof (addr)) < 0)
		erro("Connect");

	// TO DO
	/*write(fd, argv[3], 1 + strlen(argv[3]));
	nread = read(fd, buffer, BUF_SIZE-1);
	buffer[nread] ='\0';
	printf("Received from server: %s\n", buffer);*/
	close(fd);

	exit(0);
}

void erro(char *msg)
{
	printf("Erro: %s\n", msg);
	exit(-1);
}
