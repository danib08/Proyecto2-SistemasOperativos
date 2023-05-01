libs = -lpng -lpthread
all: server_seq client
.PHONY: all

client: client.c
	gcc client.c $(libs) -o client

server_seq: server_seq.c
	gcc server_seq.c -lpng -o server_seq