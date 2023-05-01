libs = -lpng -lpthread
all: client sequential heavy_process

client: client.c
	gcc client.c $(libs) -o client

sequential: sequential.c
	gcc sequential.c -lpng -o sequential

heavy_process: heavy_process.c
	gcc heavy_process.c $(libs) -o heavy_process