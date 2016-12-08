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

int verifies_login(char **array, int num, char *string);
int num_lines(char *file);
char  **load_pass(char *file, int n_lines);
char  **load_users(char *file, int n_lines);
int load_logins(char **user, char **pw);
void process_client(int fd);
void erro(char *msg);

char **user, **pass;
int num_logins;

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

	//for(i = 0; i < 4; i++)
	//	printf("Login %d: %s - %s\n", i, user[i], pass[i]);

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
	int nread = 0;
	char buf_pass[SIZE];
	char buf_user[SIZE];
	int flag, i;

	// reads username from client
	nread = read(client_fd, buf_user, SIZE);
	buf_user[nread] = '\0';
	printf("\n-----Novo cliente-----\n");
	printf("Username entered: %s", buf_user);
	//fflush(stdout);

	// reads client password
	nread = read(client_fd, buf_pass, SIZE);
	buf_pass[nread] = '\0';
	printf("Password entered: %s\n", buf_pass);
	//fflush(stdout);
	for(i = 0; i < 4; i++)
		printf("Login %d: %s|%s\n", i, user[i], pass[i]);

	// TO DO

	//verify if the client is authorized
	if(verifies_login(user, num_logins, buf_user) == 1 && verifies_login(pass, num_logins, buf_pass) == 1)
	{
		flag = 1;
		printf("cenas");
	}
	else
	{
		flag = 5000;
		printf("Client inserted the wrong login information, leaving..\n");
	}
	printf("Flag: %d\n", flag);
	write(client_fd, &flag, sizeof(flag));
	printf("Client inserted the wrong login information, leaving..\n");
	// work to do
	fflush(stdout);
	while(1)
	{
		printf("client\n");
	}

	close(client_fd);
}

int verifies_login(char **array, int num, char *string)
{
	int i = 0;
	while(i < num && strcmp(array[i], string) != 0){i++;}
	printf("array[i]: %s\n", array[i]);
	if(i < num)
		return 1;
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

void erro(char *msg)
{
	printf("Erro: %s\n", msg);
	exit(-1);
}
