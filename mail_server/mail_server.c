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

void write_login(char *file);
void apply_action(char *str, int client_fd, char *buf_user, int *oper);
void replace_line(char *str);
int verifies_login(char *v_user,char *v_pass);
void refresh_logins();
int num_lines(char *file);
char **load_pass(char *file, int n_lines);
char **load_users(char *file, int n_lines);
int load_logins(char **user, char **pw);
int check_client(char *clt);
void process_client(int fd);
void sigint(int signum);
void erro(char *msg);
char *encrypt(char *str);

char **user = NULL, **pass = NULL;
int num_logins, n_proc = 0;
int it;

int main(int argc, char** argv) {
	int fd, client;
	struct sockaddr_in addr, client_addr;
	int client_addr_size;
	int i;
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
	printf("Username entered: %s, ", buf_user);

	// reads client password
	nread = read(client_fd, buf_pass, SIZE);
	buf_pass[nread] = '\0';
	printf("Password entered: *********\n");

	//verify if the client is authorized
	flag = verifies_login(buf_user, buf_pass);

	if(flag != 1)
	{
		printf("Client inserted the wrong login information, leaving..\n");
		exit(0);
	}
	else
		printf("Client connected to Mail Server\n");
	write(client_fd, &flag, sizeof(flag));


	while(1)
	{
		nread = read(client_fd, buffer, MAX_SIZE);
		buffer[nread] = '\0';
		printf("Command entered from client %s: %s\n", buf_user, buffer);
		apply_action(buffer, client_fd, buf_user, &oper);
	}

	close(client_fd);
}



void apply_action(char *str, int client_fd, char *buf_user, int *oper)
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
		write_login("../client.aut");
	}
	else if(strcmp(str, "SEND_MESS") == 0)
	{
		int n;
		// reads message and user
		nread = read(client_fd, message, MESSAGE_SIZE);
		message[nread] = '\0';
		read(client_fd, &n, sizeof(n));

		for(int i = 0; i < n; i++)
		{
			nread = read(client_fd, client, SIZE);
			client[nread] = '\0';
			// adds message to new_msg file for user
			sprintf(file, "../new_msg/%s.txt", client);

			// if client is authorized
			if(check_client(client) == 1)
			{
				// sends message
				FILE *f = fopen(file, "a");
				if(f != NULL)
				{
					fprintf(f, "From %s: %s\n", buf_user, message);
					fclose(f);
				}
				printf("Client %s sent message to %s\n", buf_user, client);
			}
			else
				printf("Client %s not authorized. Message not sent\n", client);
		}
	}
	else if(strcmp(str, "LIST_MESS") == 0)
	{
		int n_lines;
		//open file to write messages after reading them
		sprintf(file, "../read_msg/%s.txt", buf_user);
		FILE *fwr = fopen(file, "a");

		sprintf(file, "../new_msg/%s.txt", buf_user);

		n_lines = num_lines(file);
		write(client_fd, &n_lines, sizeof(n_lines));

		// open file to read new messages
		FILE *frd = fopen(file, "r");

		if(frd != NULL)
		{
			// while there is a message, send it to client
			while(fgets(message, MESSAGE_SIZE, frd) != NULL)
			{
				write(client_fd, message, MESSAGE_SIZE);
				// once the message is read, add it to the read messages file
				if(fwr != NULL)
					fprintf(fwr, "%s", message);
			}
			fclose(frd);
			fclose(fwr);
			// after reading all messages, deletes the new messages file
			remove(file);
		}
	}
	else if(strcmp(str, "LIST_READ") == 0)
	{
		int n_lines;
		// send number of messages to client
		sprintf(file, "../read_msg/%s.txt", buf_user);
		n_lines = num_lines(file);
		write(client_fd, &n_lines, sizeof(n_lines));

		// send messages to client
		FILE *fwr = fopen(file, "r");
		if(fwr != NULL)
		{
			for(int i = 0; i < n_lines; i++)
			{
				fgets(message, MESSAGE_SIZE, fwr);
				write(client_fd, message, MESSAGE_SIZE);
			}
			fclose(fwr);
		}
	}
	else if(strcmp(str, "REMOVE_MES") == 0)
	{
		int msg, n_lines, i = 0;
		sprintf(file, "../read_msg/%s.txt", buf_user);
		n_lines = num_lines(file);
		char msgs[n_lines][MESSAGE_SIZE];

		read(client_fd, &msg, sizeof(msg));

		FILE *f = fopen(file, "r");
		if(f != NULL)
		{
			while(fgets(message, MESSAGE_SIZE, f) != NULL)
			{
				if(i != msg)
					strcpy(msgs[i], message);
				i++;
			}
			fclose(f);
		}
		f = fopen(file, "w");
		if(f != NULL)
		{
			i = 0;
			while(i < n_lines)
			{
				if(i != msg)
					fprintf(f, "%s", msgs[i]);
				i++;
			}
			fclose(f);
		}
	}
	else if(strcmp(str, "OPER") == 0)
	{
		if((*oper) == 1)
		{
			printf("User already has OPER privilegies\n");
			return;
		}
		{
			int i;
			// receive password
			nread = read(client_fd, buf, SIZE);
			buf[nread] = '\0';

			// get the admin password
			for(i = 0; i < num_logins && strcmp(user[i], "admin") != 0; i++){}
			if(strcmp(buf, pass[i]) == 0)
			{
				(*oper) = 1;
				printf("Client %s now has OPER privilegies\n", buf_user);
			}
			else
				printf("Client %s inserted wrong admin password\n", buf_user);
			write(client_fd, oper, sizeof(*oper));
		}
	}
	else if(strcmp(str, "REMOVE_ALL") == 0)
	{
		sprintf(file, "../read_msg/%s.txt", buf_user);
		remove(file);
	}
	else if(strcmp(str, "REMOVE_USER") == 0)
	{
		// verifies if client has oper priveligies
		if((*oper) == 0)
		{
			printf("ERROR, User %s doesn't have OPER priveligies\n", buf_user);
			return;
		}
		refresh_logins();
		nread = read(client_fd, buf, SIZE);
		buf[nread] = '\0';
		int i;

		for(i = 0; i < num_logins && strcmp(user[i], buf) != 0; i++){}
		if(i < num_logins)
		{
			for(; i < num_logins-1; i++)
			{
				strcpy(user[i], user[i+1]);
				strcpy(pass[i], pass[i+1]);
			}
			num_logins--;
			for(int i = 0; i < num_logins; i++)
			{
				printf("user: %s, pass: %s\n", user[i], pass[i]);
			}
			printf("Client %s removed user %s\n", buf_user, buf);
			write_login("../client.aut");
		}
	}
	else if(strcmp(str, "ADD_USER") == 0)
	{
		// verifies if client has oper priveligies
		if((*oper) == 0)
		{
			printf("ERROR, User %s doesn't have OPER priveligies\n", buf_user);
			return;
		}
		int i;
		refresh_logins();
		// reads new login information
		nread = read(client_fd, client, SIZE);
		client[nread] = '\0';
		nread = read(client_fd, buf, SIZE);
		buf[nread] = '\0';
		// allocs more space for logins
		user = realloc(user, ++num_logins);
		pass = realloc(pass, num_logins);
		user[num_logins-1] = malloc(SIZE);
		pass[num_logins-1] = malloc(SIZE);
		// saves logins
		strcpy(user[num_logins-1], client);
		strcpy(pass[num_logins-1], buf);

		for(int i = 0; i < num_logins; i++)
		{
			printf("user: %s, pass: %s\n", user[i], pass[i]);
		}
		// updates logins on database
		write_login("../client.aut");
	}
	else if(strcmp(str, "LIST_USER_MESS") == 0)
	{
		if((*oper) == 0)
		{
			printf("ERROR, User %s doesn't have OPER priveligies\n", buf_user);
			return;
		}
		int n_lines;
		// send number of messages to client
		sprintf(file, "../read_msg/%s.txt", buf_user);
		n_lines = num_lines(file);
		write(client_fd, &n_lines, sizeof(n_lines));

		// send messages to client
		FILE *fwr = fopen(file, "r");
		if(fwr != NULL)
		{
			for(int i = 0; i < n_lines; i++)
			{
				fgets(message, MESSAGE_SIZE, fwr);
				write(client_fd, message, MESSAGE_SIZE);
			}
			fclose(fwr);
		}
	}
}

void refresh_logins()
{
	// is arrays arent empty, free memory
	if(user != NULL && pass != NULL)
	{
		for(int i = 0; i < num_logins; i++)
		{
			free(user[i]);
			free(pass[i]);
		}
		free(user);
		free(pass);
	}
	// load logins from file to arrays
 	num_logins = num_lines("../client.aut");
	user = load_users("../client.aut", num_logins);
	pass = load_pass("../client.aut", num_logins);
}

int verifies_login(char *v_user,char *v_pass)
{
	refresh_logins();

	for(int i = 0; i < num_logins; i++)
	{
		printf("user: %s, pass: %s\n", user[i], pass[i]);
	}

	// checks if the login exists
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
			strcpy(pw[i],encrypt(buffer_pw));
			i++;
		}
		fclose(f);
	}
	return pw;
}

void write_login(char *file)
{
	FILE *f = fopen(file, "w");
	if(f != NULL)
	{
		for(int i = 0; i < num_logins; i++)
			fprintf(f, "%s - %s\n", user[i], encrypt(pass[i]));
		fclose(f);
	}
}

// returns 1 if client is authorized, 0 if not
int check_client(char *clt)
{
	int i;
	for(i = 0; i < num_logins && strcmp(clt, user[i]) != 0; i++){}
	if(i < num_logins)
		return 1;
	return 0;
}

// function to decrypt and encrypt (same with xor encryption)
char *encrypt(char *str)
{
  	char key[10]="123456789";
  	for(int i = 0; i < strlen(str); i++)
  	{
    	str[i] = str[i] ^key[i];
  	}
	return str;
}

// return number of lines in a file
int num_lines(char *file)
{
	char buffer[MESSAGE_SIZE];
	int n_lines = 0;
	FILE *f = fopen(file, "r");

	if(f != NULL)
	{
		while(fgets(buffer, MESSAGE_SIZE, f) != NULL)
		{n_lines++;}
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

void sigint(int signum)
{
	// waits for all the son processes to end
	for(int i = 0; i < n_proc; i++)
		wait(NULL);

	// frees all resources of arrays
	for(int i = 0; i < num_logins; i++)
	{
		free(pass[i]);
		free(user[i]);
	}
	free(user);
	free(pass);

	exit(0);
}

void erro(char *msg)
{
	printf("Erro: %s\n", msg);
	exit(-1);
}
