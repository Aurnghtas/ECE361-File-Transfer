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
        fprintf(stderr, "usage: deliver <server address> <server port number>\n");
        exit(1);
    }

    printf("Please input a message as follows: ftp <file name>\n");
    char command[100], file_name[100];
    scanf("%s", command);
    scanf("%s", file_name);

    if(access(file_name, F_OK) == 0){
        printf("The File Exists\n");
    } else {
        printf("The File does NOT Exist\n");
        exit(1);
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
        return 1;
    }

    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    int num_bytes = sendto(sockfd, "ftp", strlen("ftp"), 0, res->ai_addr, res->ai_addrlen);

    // int sockfd, rv;
    // struct addrinfo hints, *res, *p;

    // memset(&hints, 0, sizeof hints);    // using memset to initialize struct
    // hints.ai_family = AF_INET;  // IPv4 internet protocols
    // hints.ai_socktype = SOCK_DGRAM;     // socket is UDP
    
    // /* get IP address, return 0 if it succeeds
    //  * return human readable string for error reporting */
    // if((rv = getaddrinfo(argv[1], argv[2], &hints, &res)) != 0) {
    //     fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    //     return 1;
    // }

    // /* find the first available socket */
    // for(p=res; p!=NULL; p=p->ai_next) {
    //     if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
    //         perror("client: socket");
    //         continue;
    //     }
    //     break;
    // }

    // if(p==NULL) {
    //     fprintf(stderr, "client: failed to find available socket");
    // }

}