#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>

#define SERVER_PORT 9000
#define BUF_SIZE 1024

void process_client(int fd);
void erro(char *msg);

int main() {
	int fd, client;
	struct sockaddr_in addr, client_addr;
	int client_addr_size;
	bzero((void *) &addr, sizeof(addr));

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(SERVER_PORT);

	if ( (fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		erro("na funcao socket");
	if ( bind(fd,(struct sockaddr*)&addr,sizeof(addr)) < 0)
		erro("na funcao bind");
	if( listen(fd, 5) < 0)
		erro("na funcao listen");

	while(1)
	{
		client_addr_size = sizeof(client_addr);
		client = accept(fd,(struct sockaddr *)&client_addr, (socklen_t *)&client_addr_size);
		if (client > 0)
		{
			if (fork() == 0)
			{
				close(fd);
				process_client(client);
				exit(0);
			}
		close(client);
		}
	}
	return 0;
}

void process_client(int client_fd)
{
	int nread = 0;
	char buffer[BUF_SIZE];

	do{
		// TO DO
		/*nread = read(client_fd, buffer, BUF_SIZE-1);
		buffer[nread] = '\0';
		printf("Received from client: %s\n", buffer);
		fflush(stdout);*/
	} while (nread > 0);

	close(client_fd);
}

void erro(char *msg)
{
	printf("Erro: %s\n", msg);
	exit(-1);
}
