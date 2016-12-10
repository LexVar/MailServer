#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/wait.h>

#define SIZE 10
#define MAX_SIZE 25
#define MESSAGE_SIZE 100

void apply_action(char *str, int fd, char *user);
void erro(char *msg);
void replace_line(char *str);
int num_lines(char *file);
void notify_new_msg(char *user);

int oper = 0;

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

	signal(SIGINT, SIG_IGN);

	printf("Enter your username: ");
	fgets(user, SIZE, stdin);
	replace_line(user);
	write(fd, user, SIZE);

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
		printf("\nEnter command: ");

		fgets(buffer, MAX_SIZE, stdin);
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
	int client, msg;
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
		int n;
		printf("Insert message: ");
		fgets(message, MESSAGE_SIZE, stdin);
		replace_line(message);
		// send message and user to server
		write(fd, message, MESSAGE_SIZE);

		printf("How maney users do you want to send it too: ");
		scanf("%d", &n);
		// getchar to remove the left '\n'
		getchar();
		// sends number of users to server
		write(fd, &n, sizeof(n));
		for(int i = 0; i < n; i++)
		{
			printf("Input user: ");
			fgets(buf, SIZE, stdin);
			replace_line(buf);
			// sends user to server
			write(fd, buf, SIZE);
		}
	}
	else if(strcmp(str, "LIST_MESS") == 0)
	{
		int n_lines;

		// read number of messages
		read(fd, &n_lines, sizeof(n_lines));

		// if not empty read messages and print
		if(n_lines != 0)
		{
			for(int i = 0; i < n_lines; i++)
			{
				nread = read(fd, message, MESSAGE_SIZE);
				message[nread] = '\0';
				printf("%d -> %s", i, message);
			}
		}
		else
			printf("Mailbox empty\n");

	}
	else if(strcmp(str, "LIST_READ") == 0)
	{

		int n_lines;
		// read number of messages
		read(fd, &n_lines, sizeof(n_lines));

		// if not empty read messages and print
		if(n_lines != 0)
		{
			for(int i = 0; i < n_lines; i++)
			{
				nread = read(fd, message, MESSAGE_SIZE);
				message[nread] = '\0';
				printf("%d -> %s", i, message);
			}
		}
		else
			printf("Read archive empty..\n");
	}
	else if(strcmp(str, "REMOVE_MES") == 0)
	{
		int msg;
		printf("Message to remove(index): ");
		scanf("%d", &msg);
		getchar();
		write(fd, &msg, sizeof(msg));
	}
	else if(strcmp(str, "OPER") == 0)
	{
		if(oper == 1)
		{
			printf("You already hava OPER priveligies\n");
			return;
		}
		else
		{
			printf("Enter admin password: ");
			fgets(buf, SIZE, stdin);
			replace_line(buf);
			// sends password
			write(fd, buf, SIZE);
			// read oper response
			read(fd, &msg, sizeof(msg));
			if(msg == 1)
			{
				printf("OPER previligies conceded\n");
				oper = 1;
			}
			else
				printf("Wrong admin password, no OPER priveligies given\n");
		}
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

int num_lines(char *file)
{
	char buffer[MESSAGE_SIZE];
	int n_lines = 0;
	FILE *f = fopen(file, "r");

	if(f != NULL)
	{
		while(fgets(buffer, MESSAGE_SIZE, f) != NULL){n_lines++;}
		fclose(f);
	}
	return n_lines;
}

void erro(char *msg)
{
	printf("Erro: %s\n", msg);
	exit(-1);
}
