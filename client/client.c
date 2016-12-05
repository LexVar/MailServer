#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>

#define SERVER_PORT 9000
#define BUF_SIZE 10

void erro(char *msg);

int main(int argc, char *argv[])
{

	char endServer[100];
	int fd, nread;
	struct sockaddr_in addr;
	struct hostent *hostPtr;
	char buffer[10];

	if (argc != 3)
	{
		printf("cliente <host> <port>\n");
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

	printf("Enter your username: ");
	fgets(buffer, BUF_SIZE, stdin);
	write(fd, buffer, BUF_SIZE);

	fflush(stdin);

	printf("Enter your password: ");
	fgets(buffer, BUF_SIZE, stdin);
	write(fd, buffer, BUF_SIZE);

	fflush(stdin);

	// work to do
	while(1)
	{

	}

	close(fd);

	exit(0);
}

void erro(char *msg)
{
	printf("Erro: %s\n", msg);
	exit(-1);
}
