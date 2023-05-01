#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>

#include "receive_image.c"
#include "functions.c"

// Creates directory for image storage
int createDirectory(char* filepath) {
    return mkdir(filepath, 0777);
}

// Creates folder for each server
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

// Get number of requests sent to server
int receiveRequestsNumber(int serverSocket) {
    
    // Struct to obtain client information
    struct sockaddr_in clientAddr;
    unsigned int sin_size = sizeof(clientAddr);

    // Waits for a client connection
    int clientSocket = accept(serverSocket, (struct sockaddr *) &clientAddr, &sin_size);

    // Create buffer 
    unsigned char* buffer = (char*) malloc(sizeof(unsigned char)*BUFFER_SIZE);
    memset(buffer, 0, sizeof(unsigned char)*BUFFER_SIZE);

    // Waits for starting message
    recv(clientSocket, buffer, BUFFER_SIZE, 0);

    return atoi(buffer);
}


/**
 * Function that manages received requests, filters image
 * clientSocket: client socket id
 * id: id of process of request
*/
void attendRequest(int clientSocket, int id, char* folderpath) {
    unsigned char* buffer = (char*) malloc(sizeof(unsigned char)*BUFFER_SIZE);
    
    //  BUffer is cleaned
    memset(buffer, 0, sizeof(unsigned char)*BUFFER_SIZE);
    
    // Waits for starting message
    recv(clientSocket, buffer, BUFFER_SIZE, 0);

    // Image is received
    Image* image = receiveImage(clientSocket);
    
    // Image is filtered
    Image filtered = sobel_filter(*image);

    // Image saving
    if (id <= 100) {
        char* s_id = int2str(id);
        char* filename = concat(s_id,".png");
        char* filepath = concat(folderpath, filename);
        writeImage(filepath, filtered);

        free(s_id);
        free(filename);
        free(filepath);
    }
    
    // Memory cleanse
    free(image->data);
    free(filtered.data);
    free(image);
    free(buffer);

    // Connection closed
    shutdown(clientSocket, SHUT_RDWR);
}