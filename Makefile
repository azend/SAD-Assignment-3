
all: server client

clean:
	rm server client

server: server.c
	gcc server.c -o server

client: client.c
	gcc server.c -o client
