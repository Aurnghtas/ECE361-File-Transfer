/*************************************************************************** 
 * some parts of the code are cited from https://beej.us/guide/bgnet/html/ *
 ***************************************************************************/

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
#include <sys/time.h>
#include "../packet.h"

void constructPacketsArray(Packet* array, int total_packets, char* data, char* fileName, int remaining_file);
void message_from_packet(Packet packet, char* message);
int numOfDigits(int input);

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
        exit(1);
    }

    /************************************
     * send message "ftp" to the server * 
     ************************************/
    int rv, sockfd;
    struct addrinfo hints, *res;
    struct timeval start, end;
    
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

    gettimeofday(&start, NULL);
    int num_bytes_send = sendto(sockfd, "ftp", strlen("ftp"), 0, res->ai_addr, res->ai_addrlen);
    if(num_bytes_send == -1) {
        printf("Failed to send to the server\n");
        exit(1);
    }

    /*********************************** 
     * Receive message from the server *
     ***********************************/
    char recv_buff[100];
    struct sockaddr_storage src_addr;
    socklen_t src_addrlen = sizeof(struct sockaddr_storage);

    int num_bytes_recv = recvfrom(sockfd, recv_buff, sizeof(recv_buff), 0, (struct sockaddr*)&src_addr, &src_addrlen);
    if(num_bytes_recv == -1) {
        printf("Failed to receive from the server\n");
        exit(1);
    }
    gettimeofday(&end, NULL);

    double elapsedTime = (end.tv_usec - start.tv_usec)/1000.0;  // us to ms for RTT
    printf("The RTT is %lf ms\n", elapsedTime);

    recv_buff[num_bytes_recv] = '\0'; // add the string terminator to the buffer

    if(strcmp(recv_buff, "yes") == 0 ) {
        printf("A file transfer can start\n");
    } else {
        printf("server says NO. File transfer can NOT start\n");
        exit(1);
    }

    printf("Now we are going to fragment a file if needed\n");
    /*****************************
     * Fragment a file if needed *
     *****************************/
    FILE* fp = fopen(file_name, "rb");  // rb - open binary file for reading
    if(fp == NULL) {
        printf("Can NOT open the file");
        exit(1);
    }

    fseek(fp, 0, SEEK_END);     // seek to the end of the file
    int file_size = ftell(fp);    // get the size of the file
    fseek(fp, 0, SEEK_SET);     // seek back to the beginning of the file

    int num_packets = file_size/max_package_size;
    int remaining_bits = file_size - num_packets*max_package_size;    // check for remainder 
    if(remaining_bits > 0) {
        num_packets += 1;
    }

    printf("Now we are going to construct struct packet array\n");
    /*********************************
     * Construct struct packet array *
     *********************************/
    char* data_buffer = (char*)malloc(sizeof(char) * file_size);
    fread(data_buffer, file_size, 1, fp);   // store the file contents into the buffer
    fclose(fp);

    Packet* packet_array = (Packet*)malloc(sizeof(Packet) * num_packets);
    printf("Now we are going to fill in elements of the packet_array\n");
    constructPacketsArray(packet_array, num_packets, data_buffer, file_name, remaining_bits);

    printf("Now we are going to construct packets(messages) from struct packet array\n");
    /********************************************************
     * Construct packets(messages) from struct packet array *
     ********************************************************/    
    for(int index=0; index<num_packets; index++) {
        /* get the corresponding message from struct packet array */
        int message_len = numOfDigits(packet_array[index].total_frag) + numOfDigits(packet_array[index].frag_no) + 
                        numOfDigits(packet_array[index].size) + strlen(packet_array[index].filename) + packet_array[index].size + 4;
        char* message = (char*)malloc(sizeof(char) * message_len);
        printf("now we are going to fill the messages\n");
        message_from_packet(packet_array[index], message);

        printf("now we are ready to send the files to the server\n");
        /* ready to send the packet(message) */
        num_bytes_send = sendto(sockfd, message, message_len, 0, res->ai_addr, res->ai_addrlen);
        if(num_bytes_send == -1) {
            printf("Failed to send the file to the server\n");
            exit(1);
        }

        printf("Now we are going to check the acknowledge\n");
        /* check acknowledgement */
        char acknowledgement_str[10];
        num_bytes_recv = recvfrom(sockfd, acknowledgement_str, sizeof(acknowledgement_str), 0, (struct sockaddr*)&src_addr, &src_addrlen);
        if(num_bytes_recv == -1) {
            printf("Failed to receive from the server\n");
            exit(1);
        }

        int acknowledgement = atoi(acknowledgement_str);
        if(acknowledgement == -1) {
            printf("Error in sending to the server, please retry!\n");
            exit(1);
        } else if(acknowledgement == 0) {
            printf("Succeed in sending to the server, please continue!\n");
        } else if(acknowledgement == 1) {
            printf("This file has been successfully transmitted! You can now send other files!\n");
        }
    }
}

void constructPacketsArray(Packet* array, int total_packets, char* data, char* fileName, int remaining_file) {
    for(int index=0; index<total_packets; index++) {
        array[index].total_frag = total_packets;
        array[index].frag_no = index+1;

        printf("now we are in the first if\n");
        if(index==total_packets-1 && remaining_file!=0) {
            array[index].size = remaining_file;
        } else {
            array[index].size = max_package_size;
        }

        array[index].filename = fileName;

        printf("now we are in the second if\n");
        if(index==total_packets-1 && remaining_file!=0) {
            printf("now we are in the second if and first for\n");
            for(int i=0; i<remaining_file; i++) {
                array[index].filedata[i] = data[i+index*max_package_size];
            }
        } else {
            printf("now we are in the second if and second for\n");
            for(int i=0; i<max_package_size; i++) {
                array[index].filedata[i] = data[i+index*max_package_size];
            }
        }
    }
}

void message_from_packet(Packet packet, char* message) {
    int index = sprintf(message, "%d:%d:%d:%s:", packet.total_frag, 
            packet.frag_no, packet.size, packet.filename);  // fill in everything except data region
    for(int i=0; i<packet.size; i++) {
        message[index+i] = packet.filedata[i];  // fill in the data region
    }
}

int numOfDigits(int input) {
    if(input<0) {
        input = -input;     
    }
    
    if(input<10) {
        return 1;
    } else if(input<100) {
        return 2;
    } else if(input<1000) {
        return 3;
    } else if(input<10000) {
        return 4;
    } else if(input<100000) {
        return 5;
    } else if(input<1000000) {
        return 6;
    } else if(input<10000000) {
        return 7;
    } else if(input<100000000) {
        return 8;
    } else if(input<1000000000) {
        return 9;
     }else if(input<10000000000) {
        return 10;
    }
}