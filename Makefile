libs = -lpng -lpthread
all: client

client: client.c
	gcc client.c $(libs) -o client