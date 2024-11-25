#include <stdio.h>
#include <fcntl.h>      // For O_* constants
#include <sys/mman.h>   // For mmap and munmap
#include <sys/stat.h>   // For mode constants
#include <unistd.h>     // For close
#include "liquidity_pool.h"

#define SHM_NAME "/liquidity_pool_shm"
#define SHM_SIZE 1024 * 1024  // 1MB for example
#define NUM_BUCKETS 100       // Number of buckets in the hash table

typedef struct HashTable {
    LiquidityPool* buckets[NUM_BUCKETS];
} HashTable;

int main() {
    int shm_fd;
    void* shm_base;
    HashTable* ht;

    shm_fd = shm_open(SHM_NAME, O_RDONLY, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        return EXIT_FAILURE;
    }

    shm_base = mmap(0, SHM_SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);
    if (shm_base == MAP_FAILED) {
        perror("mmap");
        close(shm_fd);
        return EXIT_FAILURE;
    }

    // Access the hash table
    ht = (HashTable*)shm_base;
    for (int i = 0; i < NUM_BUCKETS; i++) {
        printf("Bucket %d:\n", i);
        LiquidityPool* current = ht->buckets[i];
        if (current == NULL) {
            printf("  [empty]\n");
        }
        while (current != NULL) {
            if (current->address != NULL && current->token0.symbol != NULL && current->token1.symbol != NULL) {
                // Display the liquidity pool
                printf("  Pool Address: %s\n", current->address);
                printf("    Token0: %s, Decimals: %d, Hash: %s, Reserve: %s\n",
                       current->token0.symbol, current->token0.decimals, current->token0.hash, current->reserve0);
                printf("    Token1: %s, Decimals: %d, Hash: %s, Reserve: %s\n",
                       current->token1.symbol, current->token1.decimals, current->token1.hash, current->reserve1);
            } else {
                printf("  [corrupted data]\n");
            }
            current = current->next;
        }
    }

    munmap(shm_base, SHM_SIZE);
    close(shm_fd);

    return EXIT_SUCCESS;
}

