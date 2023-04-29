libs = -lpng -lpthread
all : client server_seq server_hp server_php

client : client.c
	gcc client.c $(libs) -o client