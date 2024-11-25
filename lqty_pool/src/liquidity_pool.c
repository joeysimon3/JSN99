#include "liquidity_pool.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Convert a string to a double
double string_to_double(const char* str) {
    if (!str) {
        fprintf(stderr, "Error: NULL string provided to string_to_double\n");
        return 0.0;
    }
    char* endptr;
    double value = strtod(str, &endptr);
    if (*endptr != '\0') {
        fprintf(stderr, "Warning: Non-numeric characters found in string: '%s'\n", str);
    }
    return value;
}

// Convert a double to a string
char* double_to_string(double value) {
    char* buffer = (char*)malloc(50); // Allocate memory for the string
    if (!buffer) {
        perror("Failed to allocate memory for string conversion");
        exit(EXIT_FAILURE);
    }
    snprintf(buffer, 50, "%.0f", value); // Convert double to string
    return buffer;
}

void display_pool(const LiquidityPool* pool) {
    if (!pool) {
        fprintf(stderr, "Error: Null pool in display_pool.\n");
        return;
    }

    printf("Liquidity Pool Details:\n");
    printf("  Address: %s\n", pool->address);
    printf("  Token0: %s (Hash: %s, Decimals: %d, Reserve: %s)\n",
           pool->token0.symbol, pool->token0.hash, pool->token0.decimals, pool->reserve0);
    printf("  Token1: %s (Hash: %s, Decimals: %d, Reserve: %s)\n",
           pool->token1.symbol, pool->token1.hash, pool->token1.decimals, pool->reserve1);
}

// Perform a swap on the liquidity pool
void swap(LiquidityPool* pool, const char* amount0In, const char* amount1In,
          const char* amount0Out, const char* amount1Out) {
    if (!pool || !pool->reserve0 || !pool->reserve1) {
        fprintf(stderr, "Invalid pool or reserves for swap.\n");
        return;
    }

    // Convert reserves and amounts to double
    double reserve0 = string_to_double(pool->reserve0);
    double reserve1 = string_to_double(pool->reserve1);
    double in0 = string_to_double(amount0In);
    double in1 = string_to_double(amount1In);
    double out0 = string_to_double(amount0Out);
    double out1 = string_to_double(amount1Out);

    // Update reserves
    reserve0 = reserve0 + in0 - out0;
    reserve1 = reserve1 + in1 - out1;

    // Convert updated reserves back to strings
    free(pool->reserve0);
    free(pool->reserve1);
    pool->reserve0 = double_to_string(reserve0);
    pool->reserve1 = double_to_string(reserve1);

    printf("Performed swap:\n");
    printf("  Token0: +%s in, -%s out -> New Reserve: %s\n", amount0In, amount0Out, pool->reserve0);
    printf("  Token1: +%s in, -%s out -> New Reserve: %s\n", amount1In, amount1Out, pool->reserve1);
}

LiquidityPool* create_pool(const char* symbol0, uint8_t decimals0, const char* hash0,
                           const char* symbol1, uint8_t decimals1, const char* hash1,
                           const char* reserve0, const char* reserve1, const char* poolAddress) {
    if (!symbol0 || !hash0 || !symbol1 || !hash1 || !reserve0 || !reserve1 || !poolAddress) {
        fprintf(stderr, "Error: Invalid arguments to create_pool.\n");
        return NULL;
    }

    LiquidityPool* pool = (LiquidityPool*)malloc(sizeof(LiquidityPool));
    if (!pool) {
        perror("Failed to allocate memory for LiquidityPool");
        return NULL;
    }

    pool->token0.symbol = strdup(symbol0);
    pool->token0.decimals = decimals0;
    pool->token0.hash = strdup(hash0);

    pool->token1.symbol = strdup(symbol1);
    pool->token1.decimals = decimals1;
    pool->token1.hash = strdup(hash1);

    pool->reserve0 = strdup(reserve0);
    pool->reserve1 = strdup(reserve1);
    pool->address = strdup(poolAddress);

    pool->next = NULL;

    printf("Created pool with address: %s\n", pool->address);
    return pool;
}


// Free a liquidity pool
void free_pool(LiquidityPool* pool) {
    if (!pool) return;

    // Free token0 resources
    free(pool->token0.symbol);
    free(pool->token0.hash);

    // Free token1 resources
    free(pool->token1.symbol);
    free(pool->token1.hash);

    // Free reserves and address
    free(pool->reserve0);
    free(pool->reserve1);
    free(pool->address);

    // Free the pool structure
    free(pool);
}

