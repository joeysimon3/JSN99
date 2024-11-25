#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>      // For O_* constants
#include <sys/mman.h>   // For mmap and munmap
#include <sys/stat.h>   // For mode constants
#include <unistd.h>     // For close and unlink
#include "liquidity_pool.h"

#define SHM_NAME "/liquidity_pool_shm"
#define SHM_SIZE 1024 * 1024  // 1MB for example

int cleanup_shared_memory() {
    int shm_fd;
    void* shm_base;

    // Open the shared memory object
    shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("Failed to open shared memory for cleanup");
        return EXIT_FAILURE;
    }

    // Map the shared memory object
    shm_base = mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_base == MAP_FAILED) {
        perror("Failed to map shared memory for cleanup");
        close(shm_fd);
        return EXIT_FAILURE;
    }

    // Unmap the shared memory
    if (munmap(shm_base, SHM_SIZE) == -1) {
        perror("Failed to unmap shared memory");
        close(shm_fd);
        return EXIT_FAILURE;
    }

    // Close the shared memory object
    if (close(shm_fd) == -1) {
        perror("Failed to close shared memory");
        return EXIT_FAILURE;
    }

    // Unlink (remove) the shared memory object
    if (shm_unlink(SHM_NAME) == -1) {
        perror("Failed to unlink (remove) shared memory");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int main() {
    if (cleanup_shared_memory() == EXIT_SUCCESS) {
        printf("Shared memory cleaned up and removed successfully.\n");
    } else {
        printf("Failed to clean up and remove shared memory.\n");
    }
    return 0;
}

