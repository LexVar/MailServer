#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <sys/wait.h>

#define SIZE 10
#define MAX_SIZE 25
#define MESSAGE_SIZE 100

void write_login(char *file, char **user, char **pass);
void apply_action(char *str, int fd_client, char *buf_user);
void replace_line(char *str);
int verifies_login(char *v_user,char *v_pass);
int num_lines(char *file);
char  **load_pass(char *file, int n_lines);
char  **load_users(char *file, int n_lines);
int load_logins(char **user, char **pw);
void process_client(int fd);
void erro(char *msg);

char **user, **pass;
int num_logins;
int it;

int main(int argc, char** argv) {
	int fd, client;
	struct sockaddr_in addr, client_addr;
	int client_addr_size;
	int i, n_proc = 0;
	int server_port;

	if(argc != 3)
	{
		printf("Usage: mail_server -p <port>\n");
		exit(-1);
	}

	server_port = atoi(argv[2]);

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

	printf("Mail Server running..\n");

	// load logins from file to arrays

 	num_logins = num_lines("../client.aut");
	user = load_users("../client.aut", num_logins);
	pass = load_pass("../client.aut", num_logins);

	while(1)
	{
		client_addr_size = sizeof(client_addr);
		client = accept(fd,(struct sockaddr *)&client_addr, (socklen_t *)&client_addr_size);
		if (client > 0)
		{
			// creates a new process to process client when a new client appears
			n_proc++;
			if (fork() == 0)
			{
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

	// frees all resources of arrays
	for(i = 0; i < num_logins; i++)
	{
		free(pass[i]);
		free(user[i]);
	}
	free(user);
	free(pass);

	return 0;
}

void process_client(int client_fd)
{
	int oper = 0;
	int nread = 0;
	char buf_pass[SIZE];
	char buf_user[SIZE], buffer[MAX_SIZE];
	int flag, i;

	// reads username from client
	nread = read(client_fd, buf_user, SIZE);
	buf_user[nread] = '\0';
	printf("\n-----Novo cliente-----\n");
	printf("Username entered: %s\t", buf_user);
	//fflush(stdout);

	// reads client password
	nread = read(client_fd, buf_pass, SIZE);
	buf_pass[nread] = '\0';
	printf("Password entered: %s\n", buf_pass);
	//fflush(stdout);

	// TO DO

	//verify if the client is authorized
	flag = verifies_login(buf_user, buf_pass);
	if(flag != 1)
	{
		printf("Client inserted the wrong login information, leaving..\n");
		exit(0);
	}
	write(client_fd, &flag, sizeof(flag));


	while(1)
	{
		nread = read(client_fd, buffer, MAX_SIZE);
		buffer[nread] = '\0';
		printf("Command entered from client %s: %s\n", buf_user, buffer);
		apply_action(buffer, client_fd, buf_user);
	}

	close(client_fd);
}



void apply_action(char *str, int client_fd, char *buf_user)
{
	char buf[SIZE], client[SIZE];
	int nread;
	char message[MESSAGE_SIZE], file[MESSAGE_SIZE];
	if(strcmp(str, "QUIT") == 0)
	{
		printf("Client %s leaving mail server..\n", buf_user);
		exit(0);
	}
	else if(strcmp(str, "LIST_USERS") == 0)
	{
		write(client_fd, &num_logins, sizeof(num_logins));
		for(int i = 0; i < num_logins; i++)
			write(client_fd, user[i], SIZE);
	}
	else if(strcmp(str, "CHANGE_PASSW") == 0)
	{
		nread = read(client_fd, buf, SIZE);
		buf[nread] = '\0';
		strcpy(pass[it], buf);
		printf("Client %s changed password\n", buf_user);
		write_login("../client.aut", user, pass);
	}
	else if(strcmp(str, "SEND_MESS") == 0)
	{
		read(client_fd, message, MESSAGE_SIZE);
		read(client_fd, client, SIZE);
		sprintf(file, "../new_msg/%s.txt", client);
		FILE *f = fopen(file, "a");
		if(f != NULL)
		{
			fprintf(f, "From %s: %s\n", buf_user, message);
		}
		fclose(f);
		printf("Client %s sent message to %s\n", buf_user, client);
	}

}

int verifies_login(char *v_user,char *v_pass)
{
	for(it = 0; it < num_logins; it++)
	{
		if(strcmp(user[it], v_user) == 0 && strcmp(pass[it], v_pass) == 0)
			return 1;
	}
	return 0;
}

// saves users in array
char  **load_users(char *file, int n_lines)
{
	FILE *f = fopen(file, "r");
	char buffer_pw[SIZE], buffer_user[SIZE];
	char **user;
	int i = 0;

	if(f != NULL)
	{
		user = malloc(n_lines * sizeof(char *));

		while(fscanf(f, "%s - %s\n", buffer_user, buffer_pw) != EOF)
		{
			user[i] = malloc(SIZE);
			strcpy(user[i], buffer_user);
			i++;
		}
		fclose(f);
	}
	return user;
}

// saves passwords in array
char  **load_pass(char *file, int n_lines)
{
	FILE *f = fopen(file, "r");
	char buffer_pw[SIZE], buffer_user[SIZE];
	char **pw;
	int i = 0;

	if(f != NULL)
	{
		pw = malloc(n_lines * sizeof(char *));

		while(fscanf(f, "%s - %s\n", buffer_user, buffer_pw) != EOF)
		{
			pw[i] = malloc(SIZE);
			strcpy(pw[i], buffer_pw);
			i++;
		}
		fclose(f);
	}
	return pw;
}

void write_login(char *file, char **user, char **pass)
{
	FILE *f = fopen(file, "w");
	if(f != NULL)
	{
		for(int i = 0; i < num_logins; i++)
			fprintf(f, "%s - %s\n", user[i], pass[i]);
		fclose(f);
	}
}

// return number of lines in a file
int num_lines(char *file)
{
	char buffer[2*SIZE];
	int n_lines = 0;
	FILE *f = fopen(file, "r");

	if(f != NULL)
	{
		while(fgets(buffer, 2*SIZE, f) != NULL){n_lines++;}
		fclose(f);
	}
	return n_lines;
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
