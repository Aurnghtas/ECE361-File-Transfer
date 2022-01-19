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

    struct addrinfo hints, *servinfo;
    int rv;

    memset(&hints, 0, sizeof hints);    // using memset to initialize struct
    hints.ai_family = AF_INET;  // IPv4 internet protocols
    hints.ai_socktype = SOCK_DGRAM;     // socket is UDP

    /* gets IP address, return 0 if it succeeds
     * return human readable string for error reporting */
    if((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0 ){
        fprintf(stdeer, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
}