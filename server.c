//the calling of sendto and recvfrom consulted this tutorial http://velep.com/archives/934.html

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
    //called in the format server <UDP listen port>
    if (argc != 2) {
        fprintf(stderr, "usage: server <UDP listen port>\n");
        exit(1);
    }

    //first, open a UDP socket 
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(fd < 0){
        //error in creation
        printf("fails in openning socket");
        exit(1);
    }

    //then, associates the socket with the address to listen to
    //need to first create the receiver sockaddr struct
    struct sockaddr_in server; //on stack
    bzero(&server, sizeof(server)); //initialize struct
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY); //bind socket to all local interfaces(so can receive from any IP)
    int portNum = atoi(argv[1]); //get port num in int
    server.sin_port = htons(portNum); //bind to this port num
    
    //now, can bind
    if(bind(fd, (struct sockaddr*)&server, sizeof(server)) == -1){
        //failure
        printf("Fails to bind to this port number, try again with a different one.\n");
        exit(1);
    }

    //now binding successful, receive from deliver
    char buffer[100]; //the receive buffer
    //need to create the sender sockaddr struct
    struct sockaddr_in sender;
    int sender_len = sizeof(struct sockaddr_in); //the pointer to size of sender's address
    bzero(buffer, sizeof(buffer)); //clear the revieve buffer
    if(recvfrom(fd, buffer, sizeof(buffer), 0, (struct sockaddr*)&sender, &sender_len) <= 0){
        //fail to receive properly
        printf("Fails to recieve from the client properly.\n");
        exit(1);
    }
    
    //finally, respond accordingly to client
    if (strcmp(buffer, "ftp") == 0) {
        //the sended message is ftp, reply yes
        //now send yes
        if ((sendto(fd, "yes", strlen("yes"), 0, (struct sockaddr *) &sender, sender_len)) == -1) {
            printf("Fails to send to the client properly.\n");
            exit(1);
        }
    } else {
        if ((sendto(fd, "no", strlen("no"), 0, (struct sockaddr *) &sender, sender_len)) == -1) {
            printf("Fails to send to the client properly.\n");
            exit(1);
        }
    }

}
