#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#include <stdbool.h>

char * attach_memory_block(char *filename, int size);
bool detach_memory_block(char *block);
bool destroy_memory_block(char *filename);

#define BLOCK_SIE 4096
#define FILENAME "/Users/joeysimon/Code/JSN99/shared_memory/src/writeshmem.c"

#endif

