#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <omp.h>
#include <stdlib.h> 
#include "definitions.h"


/**
 * Method that sends a picture to the host
 * @param filename the path to the file to send
 * @param ip the ip of the host
 * @return 0 if everything went right 1 otherwise
 */

int execute_server_client(char *ip_address, int port, char *filename) {
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Error while creating socket \n");
        return 1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip_address, &serv_addr.sin_addr) <= 0) {
        printf("\n Error: Cannot connect to ip\n");
        return 1;
    }
    if (connect(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return 1;
    }

    //Sends filename
    send(sock, filename, strlen(filename) + 1, 0);
    printf("Filename sent\n");
    
    //Reads confirmation from server
    valread = read(sock, buffer, 1024);
    printf("%s\n", buffer);
    FILE *fp = fopen(filename, "rb");
    
    //Checks size more than 1024
    fseek(fp, 0L, SEEK_END);
    int sz = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    
    //Converts int to string in order to send it
    char str_num[50];
    sprintf(str_num, "%d", sz);
    send(sock, str_num, strlen(str_num) + 1, 0);
    long int sentBytes = 0;
    size_t readBytes = 0;
    printf("The size is %d \n", sz);
    
    //Sending file cycle
    valread = read(sock, buffer, 1024);
    memset(buffer, 0, sizeof(buffer));

    while (sz != 0) {
        readBytes = fread(buffer, sizeof(char), 1024, fp);
        while (readBytes != 0) {
            sentBytes = send(sock, buffer, 1024, 0);
            readBytes -= sentBytes;
            sz -= sentBytes;
        }
        if (sz < 1024) {
            readBytes = fread(buffer, sizeof(char), sz, fp);
            sentBytes = send(sock, buffer, 1024, 0);
            break;
        }
    }
    // Wait for finish confirmation
    valread = read( sock , buffer, 1024);
    return 0;
}


int main(int argc, char const *argv[]) {

    if (argc != 7) {
        printf("Incorrect number of arguments. Usage is: client <ip address> <port> <filename> <n-threads> <n-cycles> <metrics filename>\n");
        return 1;
    }

    char *ip_address = argv[1];
    int port = atoi(argv[2]);
    char *filename = argv[3];
    int num_threads = atoi(argv[4]);
    int num_images = atoi(argv[5]);
    char *output = argv[6];

    if (port == 0 || num_threads == 0 || num_images == 0) {
        printf("Invalid arguments. Usage is: client <ip address> <port> <filename> <n-threads> <n-cycles> <metrics filename>\n");
        return 1;
    }

    if(check_file_existence(filename) == -1 || get_image_type(filename) == -1){
        printf("Invalid image path or invalid format. Format must be .jpeg, .jpg or .png\n");
        return 1;
    }

    const int requests = num_images * num_threads;
    double request_time[requests];
    double cumulative_time[requests];
    int request_counter = 0;

    omp_set_num_threads(num_threads);
#pragma omp parallel
    {
        double start_time = omp_get_wtime();
        for(int i = 0; i < num_images; i++) {
            printf("Thread: %d Cycle: %d\n", omp_get_thread_num(), i);
            double init_time = omp_get_wtime();
            execute_server_client(ip_address, port, filename);
#pragma omp critical
            {
                request_time[request_counter] = (omp_get_wtime() - init_time);
                cumulative_time[request_counter] = (omp_get_wtime() - start_time);
                request_counter++;
            }
        }
    }

    
}