CC = clang
CFLAGS = -Wall -lpthread -g

all: client server

client: client.c
	$(CC) -o $@ $< $(CFLAGS)
server: server.c
	$(CC) -o $@ $< $(CFLAGS)

clean:
	$(RM) client
	$(RM) server
