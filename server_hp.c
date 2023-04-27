#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/time.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <fcntl.h>

#include "server_functions.c"

#define SEM_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)
char* parent_sem = "/parent_sem_hp";
char* report_filename = "heavy_process.txt";

char* folderpath;  
int serverSocket;  

void childFunction(int clientSocket, int id);
void handle_sigint(int sig);

int main() {
    signal(SIGINT, handle_sigint);  

    folderpath = createFolder("heavy_process");
    printf("%s\n", folderpath);

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080); 
    serverAddr.sin_addr.s_addr = INADDR_ANY; 
    
    bind(serverSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

    listen(serverSocket, __INT_MAX__);

    sem_t* sem_parent = sem_open(parent_sem, O_CREAT, SEM_PERMS, 0);

    FILE* pfile = fopen(report_filename, "w");
    fclose(pfile);
    while (1) {
        int totalRequests = receiveRequestsNumber(serverSocket);

        // start timer
        struct timeval t1, t2;
        double elapsedTime;
        gettimeofday(&t1, NULL);

        int processCount = 0;
        while (1) {
            struct sockaddr_in clientAddr;
            unsigned int sin_size = sizeof(clientAddr);

            int clientSocket = accept(serverSocket, (struct sockaddr *) &clientAddr, &sin_size);

            processCount++;
            if(fork() == 0) { 
                childFunction(clientSocket, processCount); 
                exit(EXIT_SUCCESS);

            } else { 
                printf("%d images received!\n", processCount);
                if (processCount == totalRequests){
                    for (int i = 0; i < processCount; i++) sem_wait(sem_parent);
                    break;
                }
            }
        }
        
        gettimeofday(&t2, NULL);
        elapsedTime = (t2.tv_sec - t1.tv_sec);
        elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000000.0;   // us to s
        printf("Time: %f s\n", elapsedTime);

        pfile = fopen(report_filename, "a");
        fprintf(pfile, "%d %f ", processCount, elapsedTime);
        fclose(pfile);
    }


    shutdown(serverSocket, SHUT_RDWR);
    
    free(folderpath);
    return 0;
}


void childFunction(int clientSocket, int id) {
    sem_t* sem_parent = sem_open(parent_sem, O_RDWR);

    attendRequest(clientSocket, id, folderpath);

    sem_post(sem_parent);
    sem_close(sem_parent);

    exit(EXIT_SUCCESS);
}


void handle_sigint(int sig) { 
    sem_unlink(parent_sem);
    free(folderpath);
    shutdown(serverSocket, SHUT_RDWR);
    exit(0);
}