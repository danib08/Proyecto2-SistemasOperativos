#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/socket.h>

#include "constants.c"
#include "process_image.c"

/**
 * FUnction to receive data from socket client
 * return: pointer of sent image
*/ 
Image* receiveImage(int socket) {

    // Message from client with name and dimention
    unsigned char clientMessage[BUFFER_SIZE];
    recv(socket, &clientMessage, BUFFER_SIZE, 0);

    // Get image information
    char *rows = strtok(clientMessage, "*");
    char *cols = strtok(NULL, "*");
    char *size = strtok(NULL, "*");

    // Allocate memory to save image
    int** data = malloc(atoi(size));
    int offset = 0;

    // Approx iteration calc
    int iter = (atoi(size)/BUFFER_SIZE);
    int iter_max = iter + 5; // To assure a complete the complete transfer of data

    //  
    // Buffer creation to read message from socket client
    unsigned char* buffer = (char*) malloc(sizeof(unsigned char)*BUFFER_SIZE);
    
    // File reception
    for (int i = 0; i < iter_max; i++) {

        // Buffer cleaned
        memset(buffer, 0, sizeof(unsigned char)*BUFFER_SIZE);

        // Read new message
        int receivedBytes = recv(socket, buffer, BUFFER_SIZE, 0); 

        if (receivedBytes == 0) { // Connection lost
            free(buffer);
            exit(1);
        }

        // Verifies if messaged is send successfully 
        if (strcmp(END_MSG, buffer) == 0) break;
        
        // Verifies if message is received successfully (bytes recibidos = BUFFER_SIZE)
        if(receivedBytes != BUFFER_SIZE && i < iter) {
            int completed = 0;
            while (!completed) {

                // Send error message
                int s = send(socket, INCOMPLETE_MSG, BUFFER_SIZE, 0);

                if (s == 0) { 
                    
                    //Connection lost handle
                    free(buffer);
                    printf("Error in data received\n");
                    exit(1);
                }
                // Buffer is cleaned
                memset(buffer, 0, sizeof(unsigned char)*BUFFER_SIZE);

                // New message received
                receivedBytes = recv(socket, buffer, BUFFER_SIZE, 0);

                // Verifies new message reception
                if (receivedBytes == BUFFER_SIZE) {
                    completed = 1;
                    memcpy((void*)data+offset, (void*)buffer, receivedBytes);
                    send(socket, COMPLETE_MSG, BUFFER_SIZE, 0);
                }
            }
        } else {

            // Received data is stored
            memcpy((void*)data+offset, (void*)buffer, receivedBytes);
            send(socket, COMPLETE_MSG, BUFFER_SIZE, 0);
        }
        offset += BUFFER_SIZE;
    }
    // Memory cleaned
    free(buffer);
    
    Image *image = malloc(sizeof(image));
    image->rows = atoi(rows);
    image->cols = atoi(cols);
    image->data = data;

    //Pointer to correct direction
    int *ptr = (int *)(data + image->rows);
    for(int i = 0; i < image->rows; i++) 
        data[i] = (ptr + image->cols * i); 

    return image;
}