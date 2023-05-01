#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>

#include "inc/server_functions.c"

char* folderpath;  // Path of server folder
int serverSocket;  // Server socket descriptor
char* report_filename = "sequential.txt";

void handle_sigint(int sig);

int main(int argc, char **argv) {
    if(argc != 3) {
        printf("\033[1;31m");
        printf("Error: 2 arguments needed: <ip address>, <port>\n");
        printf("\033[0m");
        exit(1);
    }
    // IP address for socket
    char* IP = argv[1];
    // Socket port
    int port = atoi(argv[2]);

    signal(SIGINT, handle_sigint);  // Handles SIGINT signal (interrupt)

    // Creates directories
    folderpath = createFolder("sequential");
    printf("%s\n", folderpath);

    // Creates server socket descriptor
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    
    // Configs sock address and port
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port); // Port
    serverAddr.sin_addr.s_addr = inet_addr(IP); // IP address
    
    // Assigns port to socket
    bind(serverSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

    // Listens for requests
    listen(serverSocket, __INT_MAX__);

    FILE* pfile = fopen(report_filename, "w");
    fclose(pfile);
    while (1) {
        // Receives number of requests from client
        int requests = receiveRequestsNumber(serverSocket);

        // Start timer
        struct timeval t1, t2;
        double elapsedTime;
        gettimeofday(&t1, NULL);

        int processCount = 0;
        // Manages client requests
        while (1) {
            // Struct to obtain client information
            struct sockaddr_in clientAddr;
            unsigned int sin_size = sizeof(clientAddr);

            // Waits for a client request
            int clientSocket = accept(serverSocket, (struct sockaddr *) &clientAddr, &sin_size);

            // Processes request
            processCount++;
            attendRequest(clientSocket, processCount, folderpath);
            printf("%d received requests!\n", processCount);

            if (processCount == requests){
                // Get total time that passes
                gettimeofday(&t2, NULL);
                elapsedTime = (t2.tv_sec - t1.tv_sec); // seconds
                elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000000.0;   // us to s
                printf("Time: %f s\n", elapsedTime);

                // Update statistics report
                pfile = fopen(report_filename, "a");
                fprintf(pfile, "%d %f ", processCount, elapsedTime);
                fclose(pfile);

                break;
            }
        }
    }

    // Close connection
    shutdown(serverSocket, SHUT_RDWR);
    fclose(pfile);
    free(folderpath);
    return 0;
}

/**
 * Handles the CTRL+C action by user to stop father process
*/
void handle_sigint(int sig) { 
    free(folderpath);
    shutdown(serverSocket, SHUT_RDWR);
    exit(0);
}