#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shared_memory.h"

#define BLOCK_SIZE 4096

int main(int argc, char *argv[]){
    if (argc != 1) {
        printf("usage -%s //no args",argv[0]);
        return -1;
    }

    if (destroy_memory_block(FILENAME)){
        printf("Destroyed block: %s\n",FILENAME);
    } else {
        printf("Could not destroy block: %s\n",FILENAME);
    }

    return 0;

}
