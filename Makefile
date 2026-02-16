CC = gcc
CFLAGS = -Wall -g
LDFLAGS_CLIENT = -lncurses

all: server client

server: server.c server.h
	$(CC) $(CFLAGS) -o server server.c

client: client.c
	$(CC) $(CFLAGS) -o client client.c $(LDFLAGS_CLIENT)

clean:
	rm -f server client
