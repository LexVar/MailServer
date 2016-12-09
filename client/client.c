#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>

#define SIZE 10

void erro(char *msg);
void replace_line(char *str);

int main(int argc, char *argv[])
{

	char endServer[100];
	int fd, nread, flag;
	struct sockaddr_in addr;
	struct hostent *hostPtr;
	char buffer[SIZE], user[SIZE];

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
	fgets(user, SIZE, stdin);
	replace_line(user);
	write(fd, user, SIZE);

	fflush(stdin);


	printf("Enter your password: ");
	fgets(buffer, SIZE, stdin);
	replace_line(buffer);
	write(fd, buffer, SIZE);

	read(fd, &flag, sizeof(flag));
	if(flag == 1)
		printf("Login accepted, welcome %s\n", user);
	else
	{
		printf("Login incorrect! Leaving server...\n");
		exit(0);
	}

	// work to do
	while(1)
	{

	}

	close(fd);

	exit(0);
}

void replace_line(char *str)
{
	int i;
	for(i = 0; str[i] != '\0'; i++)
	{
		if(str[i] == '\n')
			str[i] = '\0';

	}
}

void erro(char *msg)
{
	printf("Erro: %s\n", msg);
	exit(-1);
}
