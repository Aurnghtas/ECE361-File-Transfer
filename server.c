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

int main(int argc, char *argv[]){
    if (argc != 2) {
        fprintf(stderr, "usage: deliver <server address> <server port number>\n");
        exit(1);
    }
}
