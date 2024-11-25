#include <string.h>
#include "liquidity_pool.h"
#include <stdio.h>
#include <fcntl.h>      // For O_* constants
#include <sys/mman.h>   // For mmap and munmap
#include <sys/stat.h>   // For mode constants
#include <unistd.h>     // For ftruncate and close

#define SHM_NAME "/liquidity_pool_shm"
#define SHM_SIZE 1024 * 1024  // 1MB for example
#define NUM_BUCKETS 100       // Number of buckets in the hash table
#define POOL_SIZE sizeof(LiquidityPool)

typedef struct HashTable {
    LiquidityPool* buckets[NUM_BUCKETS];
} HashTable;

// Simple hash function based on the address of the pool
unsigned int hash(const char* address) {
    unsigned long h = 0;
    while (*address) {
        h = (h * 65599) + (unsigned char)(*address++);
    }
    return h % NUM_BUCKETS;
}

// Custom memory allocator for liquidity pools within shared memory
LiquidityPool* allocate_pool(void* shm_base, size_t offset, const char* symbol0, uint8_t decimals0, const char* hash0,
                             const char* symbol1, uint8_t decimals1, const char* hash1, const char* reserve0, const char* reserve1, const char* address) {
    LiquidityPool* pool = (LiquidityPool*)((char*)shm_base + offset);
    if (offset + POOL_SIZE > SHM_SIZE) {
        return NULL; // Out of bounds error
    }

    // Manually copy data into the new pool object
    pool->token0.symbol = strdup(symbol0); // Note: strdup isn't shared-memory safe, adjust as needed
    pool->token0.decimals = decimals0;
    pool->token0.hash = strdup(hash0);     // Same note as above
    pool->token1.symbol = strdup(symbol1);
    pool->token1.decimals = decimals1;
    pool->token1.hash = strdup(hash1);
    pool->reserve0 = strdup(reserve0);
    pool->reserve1 = strdup(reserve1);
    pool->address = strdup(address);
    pool->next = NULL;

    return pool;
}

int main() {
    int shm_fd;
    void* shm_base;
    HashTable* ht;

    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        return EXIT_FAILURE;
    }

    if (ftruncate(shm_fd, SHM_SIZE) == -1) {
        perror("ftruncate");
        close(shm_fd);
        return EXIT_FAILURE;
    }

    shm_base = mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_base == MAP_FAILED) {
        perror("mmap");
        close(shm_fd);
        return EXIT_FAILURE;
    }

    // Initialize the hash table
    ht = (HashTable*)shm_base;
    memset(ht->buckets, 0, NUM_BUCKETS * sizeof(LiquidityPool*));

    // Allocate and initialize a LiquidityPool
    size_t offset = sizeof(HashTable); // Start after the hash table
    LiquidityPool* pool = allocate_pool(shm_base, offset, "SENATE", 18, "0x34Be5b8C30eE4fDe069DC878989686aBE9884470",
                                        "WETH", 18, "0xC02aaA39b223FE8D0A0e5C4F27eAD9083C756Cc2",
                                        "4473167802659135395715666", "55266580100008427521", "0x9572e4C0c7834F39b5B8dFF95F211d79F92d7F23");
    if (!pool) {
        printf("Failed to allocate memory for liquidity pool.\n");
        munmap(shm_base, SHM_SIZE);
        close(shm_fd);
        return EXIT_FAILURE;
    }

    // Insert into the hash table
    unsigned int bucket_index = hash(pool->address);
    pool->next = ht->buckets[bucket_index];
    ht->buckets[bucket_index] = pool;
    for (int i = 0; i < NUM_BUCKETS; i++) {
        printf("Bucket %d:\n", i);
        LiquidityPool* current = ht->buckets[i];
        if (current == NULL) {
            printf("  [empty]\n");
        }
        while (current != NULL) {
            // Display the liquidity pool
            printf("  Pool Address: %s\n", current->address);
            printf("    Token0: %s, Decimals: %d, Hash: %s, Reserve: %s\n",
                   current->token0.symbol, current->token0.decimals, current->token0.hash, current->reserve0);
            printf("    Token1: %s, Decimals: %d, Hash: %s, Reserve: %s\n",
                   current->token1.symbol, current->token1.decimals, current->token1.hash, current->reserve1);
            current = current->next;
        }
    }

    printf("Initialization complete. Hash table set up in shared memory.\n");

    // Do not unmap or close the shared memory if other processes need to access it
    // munmap(shm_base, SHM_SIZE);
    // close(shm_fd);

    return EXIT_SUCCESS;
}

