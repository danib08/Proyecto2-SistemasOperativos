#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "functions.c"
#include "read_image.c"
#include "constants.c"

typedef struct Request {
    int ncycles; 
    int port; 
    char* serverIP;
    Image image;
} Request;


int isPNG(char* filename);
int sendImage(Image image, int socket);
char* int2str(int number);
char* concat(const char *s1, const char *s2);
void* sendRequest(void* request);
void* sendRequestsNumber(char* serverIP, int port, int requests);

int main(int argc, char **argv) {
    if(argc != 6) {
        printf("Error: 2 arguments needed: <ip address>, <port>, <image path>, <n-threads> y <n-cycles>\n");
        return 0;
    }
    // IP direction from server
    char* serverIP = argv[1];

    // Server port
    int port = atoi(argv[2]);

    // Image path
    char* filepath = argv[3];
    if(!isPNG(filepath)) {
        printf("Imagen debe tener formato png\n");
        return -1;
    }

    // Number of threads
    int nthreads = atoi(argv[4]);

    // NUmber of cycles
    int ncycles = atoi(argv[5]);

    // Send requests
    sendRequestsNumber(serverIP, port, nthreads*ncycles);

    // Read image
    Image image = readImage(filepath); 

    // Struct to pass arguments to threads 
    Request* request = malloc(sizeof(request));
    request->ncycles = ncycles;
    request->port = port;
    request->serverIP = serverIP;
    request->image = image;

    // Thread creation
    pthread_t threads_id[nthreads];
    for (int i = 0; i < nthreads; i++) {
        pthread_t thread;
        pthread_create(&threads_id[i], NULL, sendRequest, request);
    }

    printf("All threads created successfully\n");

    // Wait to all threads to be created
    for (int i = 0; i < nthreads; i++)
       pthread_join(threads_id[i], NULL);
        
    printf("Finished crearing threads\n");

    return 0;
}

/**
 * Sends n requests to server
 * requestStruct: struct with request arguments
*/
void* sendRequest(void* requestStruct){
    
    // Request parameters
    Request* request = (Request*) requestStruct;
    int ncycles = request->ncycles;
    int port = request->port;
    char* serverIP = request->serverIP;
    Image image = request->image;

    for (int i = 0; i < ncycles; i++) {
        
        // Socket creation
        int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
        
        // Configuracion de direccion y puerto del cliente
        struct sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port); // Port
        serverAddr.sin_addr.s_addr = inet_addr(serverIP); // Server IP address

        // Se intenta conectar con el puerto de la direccion ip establecida
        int connectionStatus = connect(clientSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));
        
        // Verify connection status
        if (connectionStatus == -1) {
            printf("Error while trying to connect to server\n");
            exit(1);
        }

        unsigned char* buffer = (char*) malloc(sizeof(unsigned char)*BUFFER_SIZE);
        while (1) {
            // Clean buffer
            memset(buffer, 0, sizeof(unsigned char)*BUFFER_SIZE);

            // Send start message
            send(clientSocket, START_MSG, BUFFER_SIZE, 0);

            // Sending image
            sendImage(image, clientSocket);
            
            // Send end message
            if (!send(clientSocket, END_MSG, sizeof(END_MSG), 0)) {
                printf("Error while finalizing client");
                break;
            }
            break; 
        }
        // Closing client connection
        shutdown(clientSocket, SHUT_RDWR);
    }
}

/**
 * Function to send the amount of requests sent to server
 * requestStruct: struct with request arguments
*/
void* sendRequestsNumber(char* serverIP, int port, int requests){
    
    // Socket creation
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    
    // IP address and port config
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port); // Port
    serverAddr.sin_addr.s_addr = inet_addr(serverIP); // Server IP Address 

    // Tries to connect to established ip address
    if (connect(clientSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0) {
        printf("Error while trying to connect to server\n");
        exit(1);
    }

    // Send start message
    char* s_requests = int2str(requests);
    if (!send(clientSocket, s_requests, BUFFER_SIZE, 0)) {
        printf("Error while sending requests");
        exit(1);
    }
    
    // Close connection
    shutdown(clientSocket, SHUT_RDWR);
    free(s_requests);
}

/**
 * Function to send image to server
 * return: 0 if completed, -1 otherwise
*/ 
int sendImage(Image image, int socket) {

    // Calculate image size
    int size = sizeof(int *) * image.rows + sizeof(int) * image.cols * image.rows; 
    char* ssize = int2str(size);

    // Stats message creation
    char* rows = int2str(image.rows);
    char* cols = int2str(image.cols); 
    char* aux1 = concat(rows, "*");
    char* aux2 = concat(cols, "*");
    char* aux3 = concat(aux1,aux2);
    char* imageData = concat(aux3,ssize);

    //Clean mem
    free(ssize);
    free(rows);
    free(cols);
    free(aux1);
    free(aux2);
    free(aux3);
    
    // Sends data and stats to server
    if (!send(socket, imageData, BUFFER_SIZE, 0)) {
        printf("Error while sending stats");
        exit(1);
    }
    free(imageData);

    int offset = 0;
    int bsize = BUFFER_SIZE;

    unsigned char* buffer = (char*) malloc(sizeof(unsigned char)*BUFFER_SIZE);
    char* status = (char*) malloc(sizeof(char)*BUFFER_SIZE);
    
    // Checks and compares offset and image size
    while (offset < size) {

        // Buffer clean
        memset(buffer, 0, sizeof(unsigned char)*BUFFER_SIZE);
        memset(status, 0, sizeof(unsigned char)*BUFFER_SIZE);

        // image read
        if((size - offset) < BUFFER_SIZE ) bsize = size - offset;
        memcpy((void*)buffer, (void*)image.data + offset, bsize);

        int completed = 0;

        // while to send file until completed
        while (!completed) {

            //Send image
            if (!send(socket, buffer, bsize, 0)){
                printf("Error while sending chunk\n");
                free(buffer);
                free(status);
                return -1;
            }

            // Response from server
            recv(socket, status, BUFFER_SIZE, 0);
            if (strcmp(status, COMPLETE_MSG) == 0) completed = 1;
        }
        offset += BUFFER_SIZE;
    }
    // Buffer clean
    free(buffer);
    free(status);

    return 0;
}

/**
 * Verifies png extension
 * return: 1 if is .png, 0 otherwise
*/ 
int isPNG(char* filename) {

    //Copies original string
    char * copy = malloc(strlen(filename) + 1);
    strcpy(copy, filename);

    // Dividing filename until getting '.'
    char ch[] = ".";
    char * token = strtok(copy, ch);
    while(token != NULL){
        if(strcmp(token, "png") == 0) {
            free(copy);
            return 1;
        }
        token = strtok(NULL, ch);
    }
    free(copy);
    return 0;
}