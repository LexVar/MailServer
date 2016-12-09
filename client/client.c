#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>

#define SIZE 10
#define MAX_SIZE 25
#define MESSAGE_SIZE 100

void apply_action(char *str, int fd, char *user);
void erro(char *msg);
void replace_line(char *str);
void notify_new_msg(char *user);

int main(int argc, char *argv[])
{

	char endServer[100];
	int fd, nread, flag;
	struct sockaddr_in addr;
	struct hostent *hostPtr;
	char buffer[MAX_SIZE], user[SIZE];

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

	fflush(stdin);

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
		notify_new_msg(user);
		/*
		printf("LIST_MESS – para listar todas as mensagens por ler.\n");
		printf("LIST_USERS – para listar todos os clientes autorizados\n");
		printf("SEND_MESS – para enviar uma mensagem para um cliente (autorizado).\n");
		printf("LIST_READ – para listar todas as mensagens já lidas.\n");
		printf("REMOVE_MES – para apagar mensagens.\n");
		printf("CHANGE_PASSW – alterar a password\n");
		printf("OPER – para o cliente obter os privilégios do operador.\n");
		printf("QUIT – para o cliente abandonar o sistema.\n");*/
		printf("Enter command: ");
		fgets(buffer, MAX_SIZE, stdin);

		fflush(stdin);

		replace_line(buffer);

		write(fd, buffer, MAX_SIZE);
		apply_action(buffer, fd, user);
	}

	close(fd);

	exit(0);
}

void apply_action(char *str, int fd, char *user)
{
	char buf[SIZE];
	int num_logins, nread;
	int client;
	char message[MESSAGE_SIZE];

	if(strcmp(str, "QUIT") == 0)
	{
		printf("Client abandoning server..\n");
		exit(0);
	}
	else if(strcmp(str, "LIST_USERS") == 0)
	{
		read(fd, &num_logins, sizeof(num_logins));
		for(int i = 0; i < num_logins; i++)
		{
			nread = read(fd, buf, SIZE);
			buf[nread] = '\0';
			printf("User %d -> %s\n", i, buf);
		}
	}
	else if(strcmp(str, "CHANGE_PASSW") == 0)
	{
		printf("New password: ");
		fgets(buf, SIZE, stdin);
		replace_line(buf);
		write(fd, buf, SIZE);
	}
	else if(strcmp(str, "SEND_MESS") == 0)
	{
		printf("Insert message: ");
		fgets(message, MESSAGE_SIZE, stdin);
		replace_line(message);
		printf("Choose client: ");
		fgets(buf, SIZE, stdin);
		replace_line(buf);
		write(fd, message, MESSAGE_SIZE);
		write(fd, buf, SIZE);
	}
	else if(strcmp(str, "LIST_MESS") == 0)
	{
		char file[MESSAGE_SIZE];
		//open file to write messages after reading them
		sprintf(file, "../read_msg/%s.txt", user);
		FILE *fwr = fopen(file, "a");

		// open file to read new messages
		sprintf(file, "../new_msg/%s.txt", user);
		FILE *frd = fopen(file, "r");

		if(frd != NULL)
		{
			while(fgets(message, MESSAGE_SIZE, frd) != NULL)
			{
				printf("%s", message);
				if(fwr != NULL)
				{
					fprintf(fwr, "%s", message);
				}
			}
			fclose(frd);
			fclose(fwr);
			remove(file);
		}
		else
			printf("There are no new mesages..\n");
	}
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

void notify_new_msg(char *user)
{
	char file[MESSAGE_SIZE];
	sprintf(file, "../new_msg/%s.txt", user);
	FILE *f = fopen(file, "r");

	// if file with new messages exist, notify client
	if(f != NULL)
	{
		printf("You have a new message..\n");
		fclose(f);
	}
}

void erro(char *msg)
{
	printf("Erro: %s\n", msg);
	exit(-1);
}
