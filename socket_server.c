#include <stdio.h>
#include <string.h>	//strlen
#include <sys/socket.h>
#include <arpa/inet.h>	//inet_addr
#include <unistd.h>	//write
#include <pthread.h>
#include <stdlib.h>

const int NUM_THREADS = 100, SIZE_INPUT = 100;
char ret_status [10][100];
int i = 0;
char *arg1, *arg2;

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;

// Dividir el comando en argumentos
void split(char input[SIZE_INPUT], char command[SIZE_INPUT], char args[3][SIZE_INPUT])
{
	int i = 0;
	char *space = " \0", *token;
	token = strtok(input, space);
	strcpy(command, token);
	while(token != NULL)
	{
		token = strtok(NULL, space);
		if(token != NULL)
		{
			strcpy(args[i++], token);
		}
	}
}

void *createDocker(void *arg) // Crear contenedor
{
	pthread_mutex_lock( &mutex1 );
	int pid = fork();
	if(pid == 0)
	{
		execlp("docker", "docker", "run", "-it", "--name", arg1, "-di", arg2, (char *) NULL);
	}
	sprintf(ret_status[i], "Thread %d: %d", i, i + 10);
	pthread_mutex_unlock( &mutex1 );
	pthread_exit(ret_status [i]);
}

void *listDocker(void *arg) // Listar todos los contenedores
{
	pthread_mutex_lock( &mutex1 );
	int pid = fork();
	if(pid == 0)
	{
		execlp("docker", "docker", "ps", "-a", (char *) NULL);
	}
	sprintf(ret_status[i], "Thread %d: %d", i, i + 10);
	pthread_mutex_unlock( &mutex1 );
	pthread_exit(ret_status [i]);
}

void *stopDocker(void *arg) // Detener un contenedor
{
	pthread_mutex_lock( &mutex1 );
	int pid = fork();
	if(pid == 0)
	{
		execlp("docker", "docker", "stop", arg1, (char *) NULL);
	}
	sprintf(ret_status[i], "Thread %d: %d", i, i + 10);
	pthread_mutex_unlock( &mutex1 );
	pthread_exit(ret_status [i]);
}

void *removeDocker(void *arg) // Eliminar un contenedor ya detenido
{
	pthread_mutex_lock( &mutex1 );
	int pid = fork();
	if(pid == 0)
	{
		execlp("docker", "docker", "rm", arg1, (char *) NULL);
	}
	sprintf(ret_status[i], "Thread %d: %d", i, i + 10);
	pthread_exit(ret_status [i]);
	pthread_mutex_unlock( &mutex1 );
}

int checkDocker(char *nombre)
{
	int ans = 0, nameLength = 1000;
	char *linea;
	FILE *f = fopen("docker.dat", "r+");
	while(fgets(linea, nameLength, f) && ans == 0)
	{
    	linea = strtok(linea, "\n");
		if(strcmp(nombre, linea) == 0)
		{
			ans = 1;
		}
	}
	fclose(f);
	return ans;
}

void writeToFile(char *nombre)
{
	printf("%s\n", nombre);
	FILE *f = fopen("docker.dat", "a");
	fflush(stdin);
	fputs(nombre, f);
	fputs("\n", f);
	fclose(f);
}
/*
void removeFromFile(char *nombre)
{
	int nameLength = 1000, i = 0;
	char *linea;
	char* info[1000];
	FILE *f = fopen("docker.dat", "r+");
	if(f == NULL)
	{
		printf("NUL");
	}
	while(fgets(linea, nameLength, f))
	{
		printf("BB\n");
    	linea = strtok(linea, "\n");
		if(strcmp(nombre, linea) != 0)
		{
			strcpy(info[i++], linea);
		}
	}
	printf("CC\n");
	fclose(f);
	f = fopen("docker.dat", "w");
	i = 0;
	while(fgets(linea, nameLength, f))
	{
		fputs(info[i++], f);
	}
	fclose(f);
}
*/
int main(int argc , char *argv[]) {
	char args[3][SIZE_INPUT];
	char command[SIZE_INPUT];
	int socket_desc, client_sock, c, read_size, init_socket = 0;
	int thread_no[NUM_THREADS];
	struct sockaddr_in server, client;  // https://github.com/torvalds/linux/blob/master/tools/include/uapi/linux/in.h
	char client_message[2000], buffer[128];
	pthread_t tid[NUM_THREADS];
	void *status;
	
	// Create socket
    // AF_INET (IPv4 protocol) , AF_INET6 (IPv6 protocol) 
    // SOCK_STREAM: TCP(reliable, connection oriented)
    // SOCK_DGRAM: UDP(unreliable, connectionless)
    // Protocol value for Internet Protocol(IP), which is 0
	socket_desc = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_desc == -1) {
		printf("Could not create socket");
	}
	puts("Socket created");
	
	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(8888);
	
	//Bind the socket to the address and port number specified
	if( bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0) {
		//print the error message
		perror("bind failed. Error");
		return 1;
	}
	puts("bind done");
	
	// Listen
    // It puts the server socket in a passive mode, where it waits for the client 
    // to approach the server to make a connection. The backlog, defines the maximum 
    // length to which the queue of pending connections for sockfd may grow. If a connection 
    // request arrives when the queue is full, the client may receive an error with an 
    // indication of ECONNREFUSED.
	// https://man7.org/linux/man-pages/man2/listen.2.html
	listen(socket_desc , 3);
	
	//Accept and incoming connection
	puts("Waiting for incoming connections...");
	c = sizeof(struct sockaddr_in);
	
	while(1) {
		//accept connection from an incoming client
		client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
		if(client_sock < 0) {
			perror("accept failed");
			return 1;
		}
		puts("Connection accepted");

		int pid = fork();

		if (pid == 0) {
			memset(client_message, 0, 2000);
			//Receive a message from client
			while(strcmp(client_message, "exit\n") != 0)
			{
				int r;
				thread_no[i] = i;
				init_socket = 1;
				printf("Enter a command. Enter help for help.\n");
				if(recv(client_sock, client_message, 2000, 0) > 0) {
					printf("received message: %s\n", client_message);
					split(client_message, command, args);
					arg1 = strtok(args[0], "\n");
					arg2 = strtok(args[1], "\n");
					if(strcmp(command, "create") == 0)
					{
						if(checkDocker(arg1) == 0)
						{
							if ((r = pthread_create(&tid[i], NULL, createDocker, (void *) &args)) != 0)
							{
								strerror_r(r, buffer, sizeof(buffer));
								fprintf(stderr, "Error = %d (%s)\n", r, buffer); exit (1);
							}
							writeToFile(arg1);
						}
						else
						{
							printf("Error. El contenedor ya existe.\n");
							init_socket = 0;
						}
					}
					else if(strcmp(command, "list\n") == 0)
					{
						if ((r = pthread_create(&tid[i], NULL, listDocker, (void *) &args)) != 0) {
							strerror_r(r, buffer, sizeof(buffer));
							fprintf(stderr, "Error = %d (%s)\n", r, buffer); exit (1);
						}
					}
					else if(strcmp(command, "remove") == 0)
					{
						if ((r = pthread_create(&tid[i], NULL, removeDocker, (void *) &args)) != 0) {
							strerror_r(r, buffer, sizeof(buffer));
							fprintf(stderr, "Error = %d (%s)\n", r, buffer); exit (1);
						}
					}
					else if(strcmp(command, "stop") == 0)
					{
						if ((r = pthread_create(&tid[i], NULL, stopDocker, (void *) &args)) != 0) {
							strerror_r(r, buffer, sizeof(buffer));
							fprintf(stderr, "Error = %d (%s)\n", r, buffer); exit (1);
						}
					}
					else if(strcmp(command, "help\n") == 0)
					{
						printf("--HELP--\n");
						printf("create (name) (image): creates a container.\n");
						printf("list: lists all containers.\n");
						printf("stop (name): stops a container.\n");
						printf("remove (name): removes a stopped container.\n");
						printf("exit: disconnects the client.\n");
						printf("\n");
						init_socket = 0;
					}
					else
					{
						fprintf(stderr, "Error. Comando desconocido o sintaxis erronea\n");
						init_socket = 0;
					}
					if(init_socket == 1)
					{
						if ((r = pthread_cancel(tid[i])) != 0) {
							strerror_r(r, buffer, sizeof (buffer));
							fprintf(stderr, "Error = %d (%s)\n", r, buffer); exit (1);
						}
						if ((r = pthread_join(tid [i], &status)) != 0) {
							strerror_r(r, buffer, sizeof (buffer));
							fprintf(stderr, "Error = %d (%s)\n", r, buffer); exit (1);
						}

						if (status == PTHREAD_CANCELED)
							printf("i = %d, status = CANCELED\n", i);
						else
							printf("i = %d, status = %s\n", i, (char *) status);
					}

					//Send the message back to client
					send(client_sock, client_message, strlen(client_message), 0);
				++i;
				}
			}
			printf("Client disconnected\n");
		}
    }
	return 0;
}