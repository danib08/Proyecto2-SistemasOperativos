#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>

#include "constants.c"
#include "process_image.c"

/**
 * Receives and image from a client socket
 * socket: client socket descriptor
 * return: pointer of the image sent by client
*/ 
Image* receiveImage(int socket) {
    // Client message with name and size of file
    unsigned char clientMessage[BUFFER_SIZE];
    recv(socket, &clientMessage, BUFFER_SIZE, 0);

    // Extracts image info
    char *rows = strtok(clientMessage, "*");
    char *cols = strtok(NULL, "*");
    char *size = strtok(NULL, "*");

    // Sets memory space for received image
    int** data = malloc(atoi(size));
    int offset = 0;

    // Calculates approximate necessary iterations
    int iter = (atoi(size)/BUFFER_SIZE);
    int iter_max = iter + 5; // To make sure whole image is received

    // Creates buffer to read message sent by client socket
    unsigned char* buffer = (char*) malloc(sizeof(unsigned char)*BUFFER_SIZE);
    
    // Archive reception
    for (int i = 0; i < iter_max; i++) {
        // Cleans buffer
        memset(buffer, 0, sizeof(unsigned char)*BUFFER_SIZE);

        // Reads incoming message
        int receivedBytes = recv(socket, buffer, BUFFER_SIZE, 0); 

        if (receivedBytes == 0) { // Lost connection
            printf("\033[1;31m");
            fprintf(stderr, "Lost connection\n");
            printf("\033[0m");
            free(buffer);
            exit(1);
        }

        // Verifies if archive sending ended
        if (strcmp(END_MSG, buffer) == 0) break;
        
        // Verifies if whole image is received (receivedBytes = BUFFER_SIZE)
        if(receivedBytes != BUFFER_SIZE && i < iter) {
            int completed = 0;
            while (!completed) {
                // Send message indicating incomplete reception 
                int s = send(socket, INCOMPLETE_MSG, BUFFER_SIZE, 0);

                if (s == 0) { // Lost connection
                    free(buffer);
                    printf("\033[1;31m");
                    fprintf(stderr, "Error receiving file\n");
                    printf("\033[0m");
                    exit(1);
                }
                // Cleans buffer
                memset(buffer, 0, sizeof(unsigned char)*BUFFER_SIZE);

                // Receiving new message
                receivedBytes = recv(socket, buffer, BUFFER_SIZE, 0);

                // Verifies if new message was received correctly
                if (receivedBytes == BUFFER_SIZE) {
                    completed = 1;
                    memcpy((void*)data+offset, (void*)buffer, receivedBytes);
                    send(socket, COMPLETE_MSG, BUFFER_SIZE, 0);
                }
            }
        } else {
            // Save received info to memory
            memcpy((void*)data+offset, (void*)buffer, receivedBytes);
            send(socket, COMPLETE_MSG, BUFFER_SIZE, 0);
        }
        offset += BUFFER_SIZE;
    }
    // Frees memoru
    free(buffer);
    
    Image* image = malloc(sizeof(image));
    image->rows = atoi(rows);
    image->cols = atoi(cols);
    image->data = data;

    // Correct of pointers of rows to correct address
    int *ptr = (int *)(data + image->rows);
    for(int i = 0; i < image->rows; i++) 
        data[i] = (ptr + image->cols * i); 

    return image;
}