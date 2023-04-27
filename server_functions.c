#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/socket.h>

#include "receive_image.c"
#include "general_functions.c"

int receiveRequestsNumber(int serverSocket) {
    struct sockaddr_in clientAddr;
    unsigned int sin_size = sizeof(clientAddr);

    int clientSocket = accept(serverSocket, (struct sockaddr *) &clientAddr, &sin_size);

    unsigned char* buffer = (char*) malloc(sizeof(unsigned char)*BUFFER_SIZE);
    memset(buffer, 0, sizeof(unsigned char)*BUFFER_SIZE);

    recv(clientSocket, buffer, BUFFER_SIZE, 0);

    return atoi(buffer);
}

void attendRequest(int clientSocket, int id, char* folderpath) {
    unsigned char* buffer = (char*) malloc(sizeof(unsigned char)*BUFFER_SIZE);
    
    memset(buffer, 0, sizeof(unsigned char)*BUFFER_SIZE);
    
    recv(clientSocket, buffer, BUFFER_SIZE, 0);

    Image* image = receiveImage(clientSocket);
    
    Image filtered = sobel_filter(*image);

    if (id <= 100) {
        char* s_id = int2str(id);
        char* filename = concat(s_id,".png");
        char* filepath = concat(folderpath, filename);
        writeImage(filepath, filtered);

        free(s_id);
        free(filename);
        free(filepath);
    }
    
    free(image->data);
    free(filtered.data);
    free(image);
    free(buffer);

    shutdown(clientSocket, SHUT_RDWR);
}

 
int createDirectory(char* filepath) {
    return mkdir(filepath, 0777);
}

char* createFolder(char* server_type) {
    createDirectory(server_type);

    char *sCounter = NULL;
    char *name = NULL;
    int created = -1; 
    int counter = 0;
    char* fname = concat(server_type, "/server");

    while (created != 0){
        counter++;
        sCounter = int2str(counter);
        name = concat(fname, sCounter);
        created = createDirectory(name);
    }
    char* folderpath = concat(name, "/");
    free(name);
    free(fname);
    return folderpath;
}