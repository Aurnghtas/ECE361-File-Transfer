/* 
 * some parts of the code are cited from https://beej.us/guide/bgnet/html/
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("usage: deliver <server address> <server port number>\n");
        exit(1);
    }

    printf("Please input a message as follows: ftp <file name>\n");
    char command[100], file_name[100];
    scanf("%s", command);
    scanf("%s", file_name);

    if(strcmp("ftp", command) == 0) {
        if(access(file_name, F_OK) == 0){
            printf("The File Exists\n");
        } else {
            printf("The File does NOT Exist\n");
            exit(1);
        }
    } else {
        printf("The only supported command is: ftp\n");
    }

    /* 
     * send message "ftp" to the server 
     */
    int rv, sockfd;
    struct addrinfo hints, *res;
    
    memset(&hints, 0, sizeof hints);    // using memset to initialize struct
    hints.ai_family = AF_INET;  // IPv4 internet protocols
    hints.ai_socktype = SOCK_DGRAM;     // socket is UDP

    /* get IP address, return 0 if it succeeds
     * return human readable string for error reporting */
    if((rv = getaddrinfo(argv[1], argv[2], &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }

    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    int num_bytes_send = sendto(sockfd, "ftp", strlen("ftp"), 0, res->ai_addr, res->ai_addrlen);
    if(num_bytes_send == -1) {
        printf("Failed to send to the server");
        exit(1);
    }

    /* 
     * Receive message from the server
     */
    char recv_buff[100];
    struct sockaddr_storage src_addr;
    socklen_t src_addrlen = sizeof(struct sockaddr_storage);

    int num_bytes_recv = recvfrom(sockfd, recv_buff, sizeof(recv_buff), 0, (struct sockaddr*)&src_addr, &src_addrlen);
    if(num_bytes_recv == -1) {
        printf("Failed to receive from the server");
        exit(1);
    }

    recv_buff[num_bytes_recv] = '\0'; // add the string terminator to the buffer

    if(strcmp(recv_buff, "yes") == 0 ) {
        printf("A file transfer can start");
    }
    return 0;

}