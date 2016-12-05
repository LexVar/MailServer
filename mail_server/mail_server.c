#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <sys/wait.h>

#define BUF_SIZE 10

void process_client(int fd);
void erro(char *msg);

int main(int argc, char** argv) {
	int fd, client;
	struct sockaddr_in addr, client_addr;
	int client_addr_size;
	int i, n_proc = 0;
	int server_port;

	if(argc != 2)
	{
		printf("Usage: mail_server -p <port>\n");
		exit(-1);
	}

	server_port = atoi(argv[1]);

	bzero((void *) &addr, sizeof(addr));

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(server_port);

	if ( (fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		erro("na funcao socket");

	if ( bind(fd,(struct sockaddr*)&addr,sizeof(addr)) < 0)
		erro("na funcao bind");

	if( listen(fd, 5) < 0)
		erro("na funcao listen");

	printf("\nMail Server running..\n");

	while(1)
	{
		client_addr_size = sizeof(client_addr);
		client = accept(fd,(struct sockaddr *)&client_addr, (socklen_t *)&client_addr_size);
		if (client > 0)
		{
			// creates a new process to process client when a new client appears
			if (fork() == 0)
			{
				n_proc++;
				close(fd);
				process_client(client);
				exit(0);
			}

			close(client);
		}
	}

	// waits for all the son processes to end
	for(i = 0; i < n_proc; i++)
		wait(NULL);

	return 0;
}

void process_client(int client_fd)
{
	int nread = 0;
	char pw[BUF_SIZE];
	char user[BUF_SIZE];

	// reads username from client
	nread = read(client_fd, user, BUF_SIZE);
	user[nread] = '\0';
	printf("Username entered: %s\n", user);
	fflush(stdout);

	// reads client password
	nread = read(client_fd, pw, BUF_SIZE);
	pw[nread] = '\0';
	printf("Password entered: *********\n");
	fflush(stdout);

	// TO DO

	//verify if the client is authorized

	// work to do
	while(1)
	{

	}

	close(client_fd);
}

void erro(char *msg)
{
	printf("Erro: %s\n", msg);
	exit(-1);
}
