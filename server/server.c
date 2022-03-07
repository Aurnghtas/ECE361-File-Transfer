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
#include "../packet.h"
#include <stdbool.h>
#include <time.h>

FILE* FD = NULL;
//This is the helper function that parse the recieved message into packet,
//it will create a new file if the message is the first one and close file after the last message
int packet_from_message(char* message, int prev_index){
    //first use strtok function to break up the message header
    //referred to this tutorial https://www.cplusplus.com/reference/cstring/strtok/
    const char breaker[2] = ":";
    char* total_frag = strtok(message, breaker);
    char* frag_no = strtok(NULL, breaker); //use null to continue scanning
    char* size = strtok(NULL, breaker);
    char* filename = strtok(NULL, breaker);

    //now turn strings to int values
    unsigned int Total_frag = atoi(total_frag);
    unsigned int Frag_no = atoi(frag_no);
    unsigned int dSize = atoi(size);

    /*if(Frag_no != (prev_index) + 1){
        return -1;
    }*/

    if(Frag_no == 1){
        //this is the first message revieved, need to create a Packet struct and file stream
        Packet recieved;
        recieved.filename = filename;
        recieved.frag_no = Frag_no;
        recieved.size = dSize;
        recieved.total_frag = Total_frag;

        //create file stream, open a file
        FD = fopen(filename, "wb"); //recieved file needs to have same name as sent one
        if(FD == NULL){
            printf("Cannot create file\n"); 
            return -1; // -1 for error
        }
    }

    //now, write data into the created file use fwrite to write to stream
    int header_size = strlen(total_frag) + strlen(frag_no) + strlen(size) + strlen(filename);
    //fwrite, notice need to skip the header
    fwrite(message + header_size + sizeof(char) * 4, sizeof(char), dSize, FD);

    //now check if the last message is done transmitting
    if(Frag_no == Total_frag){
        //fclose(fd);
        return 1; // 1 for end of this file transmission
    }

    return 0; // 0 for success and continue this file transmission
}

int main(int argc, char *argv[]){
//lab1******************************************************************************
    //called in the format server <UDP listen port>
    if(argc != 2){
        printf("usage: server <UDP listen port>\n");
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
    server.sin_family = AF_INET;    // IPv4 internet protocols
    server.sin_addr.s_addr = htonl(INADDR_ANY); //bind socket to all local interfaces(so can receive from any IP)
    int portNum = atoi(argv[1]); //get port num in int
    server.sin_port = htons(portNum); //bind to this port num
    
    //now, can bind
    if(bind(fd, (struct sockaddr*)&server, sizeof(server)) == -1){
        //failure
        printf("Fails to bind to this port number, try again with a different one.\n");
        exit(1);
    }
    
    //\\while(1){
    //now binding successful, receive from deliver
    char buffer[100]; //the receive buffer
    //need to create the sender sockaddr struct
    struct sockaddr_in sender;
    socklen_t sender_len = sizeof(sender); //the pointer to size of sender's address
    bzero(buffer, sizeof(buffer)); //clear the receiver buffer
    if(recvfrom(fd, buffer, sizeof(buffer), 0, (struct sockaddr*)&sender, &sender_len) <= 0){
        //fail to receive properly
        printf("Fails to recieve from the client properly.\n");
        exit(1);
    }
    
    //finally, respond accordingly to client
    if(strcmp(buffer, "ftp") == 0){
        //the sended message is ftp, reply yes
        //now send yes
        if((sendto(fd, "yes", strlen("yes"), 0, (struct sockaddr *) &sender, sender_len)) == -1){
            printf("Fails to send to the client properly.\n");
            exit(1);
        }
    }else{
        if((sendto(fd, "no", strlen("no"), 0, (struct sockaddr *) &sender, sender_len)) == -1){
            printf("Fails to send to the client properly.\n");
            exit(1);
        }
    }
    
//lab2******************************************************************************
    //recieve the messages one by one
    char data_buffer[1100]; //the receive buffer for data transmission
    bzero(data_buffer, sizeof(data_buffer)); 

    int prev_index = 0;
    srand(time(NULL)); //seed

    while(true){ 
        //while true so that the server keeps running and wait for new messages even when a full file is transmitted
        if(recvfrom(fd, data_buffer, sizeof(data_buffer), 0, (struct sockaddr*)&sender, &sender_len) <= 0){
            //fail to receive properly
            printf("Fails to recieve from the client properly.\n");
            exit(1);
        }

        //******************implement random dropping************************************************************
        double randomNum = (double)rand() / (double)RAND_MAX;
        //1 percent dropping
        if(randomNum > 0.01){
        //transfer message recieved into packet
        int status = packet_from_message(data_buffer, prev_index);
            prev_index++;
            if(status == -1){
                prev_index--;
            }
            //implement acknowledgement******************************************************************************
            // (send to client: -1 for error and retry, 0 for continue sending, 
            // 1 for end of this file transmission and can start sending other files)
            //*******************************************************************************************************
            char Status[2];
            sprintf(Status, "%d\n", status);
            if((sendto(fd, Status, strlen(Status), 0, (struct sockaddr *) &sender, sender_len)) == -1){
                printf("Fails to send to the client properly 2.\n");
                exit(1);
            }

            if(status == 1){
                printf("File transfer successful!\n");
                fclose(FD);
                break;
            }
        }else{
            //if dropped, print out
            printf("Packet dropped.\n");
        }
    }
    //}
    return 0;
}