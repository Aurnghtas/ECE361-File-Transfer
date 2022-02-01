#include <stdio.h>
#include <string.h> 
#include <stdlib.h>

#define max_package_size 1000

typedef struct packet{
    unsigned int total_frag;
    unsigned int frag_no;
    unsigned int size;
    char* filename;
    char filedata[max_package_size];
}Packet;