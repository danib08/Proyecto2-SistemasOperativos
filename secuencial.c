#include <unistd.h>  
#include <netdb.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h> 
#include <string.h> 
#include "inc/server_functions.c"


#define BACKLOG         100                 /* Max. client pending connections  */
char* folderpath;

int main(int argc, char* argv[]){
    int sockfd, connfd ;  // listening socket and connection socket file descriptors 
    unsigned int len;     //length of client address 
    struct sockaddr_in servaddr, client; 
    folderpath = createFolder("Secuencial");
    
     
    /* socket creation */
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd == -1) 
    { 
        fprintf(stderr, "[SERVER-error]: socket creation failed. %d: %s \n", errno, strerror( errno ));
        return -1;
    } 
    else
    {
        printf("[SERVER]: Socket successfully created..\n"); 
    }
    
    /* clear structure */
    memset(&servaddr, 0, sizeof(servaddr));
  
    /* assign IP, SERV_PORT, IPV4 */
    servaddr.sin_family      = AF_INET; 
    servaddr.sin_addr.s_addr = INADDR_ANY; 
    servaddr.sin_port        = htons(8080); 
    
    
    /* Bind socket */
    if ((bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) != 0) 
    { 
        fprintf(stderr, "[SERVER-error]: socket bind failed. %d: %s \n", errno, strerror( errno ));
        return -1;
    } 
    else
    {
        printf("[SERVER]: Socket successfully binded \n");
    }
  
    /* Listen */
    if ((listen(sockfd, BACKLOG)) != 0) 
    { 
        fprintf(stderr, "[SERVER-error]: socket listen failed. %d: %s \n", errno, strerror( errno ));
        return -1;
    } 
    else
    {
        printf("[SERVER]: Listening on SERV_PORT %d \n\n", ntohs(servaddr.sin_port) ); 
    }
    
    len = sizeof(client); 
  
      /* Accept the data from incoming sockets in a iterative way */
      while(1){
        int processCount = 0;
        connfd = accept(sockfd, (struct sockaddr *)&client, &len); 
        if (connfd < 0){ 
            fprintf(stderr, "[SERVER-error]: connection not accepted. %d: %s \n", errno, strerror( errno ));
            return -1;
        } 
        else{              
            while(1){  
                processCount++;
                attendRequest(connfd, processCount, folderpath);
                exit(EXIT_SUCCESS);
            }  
        }                      
    }
    shutdown(sockfd, SHUT_RDWR);
    free(folderpath);
    return 0;    
} 
