all: server client main
client: socket_client.c
	gcc socket_client.c -o client.out

server: socket_server.c
	gcc socket_server.c -o server.out