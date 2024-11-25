#ifndef LIQUIDITY_POOL_H
#define LIQUIDITY_POOL_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

// Shared hash table structure
// Token structure
typedef struct Token {
    char* symbol;
    uint8_t decimals;
    char* hash;
} Token;

// Liquidity Pool structure
typedef struct LiquidityPool {
    Token token0;
    Token token1;
    char* reserve0;
    char* reserve1;
    char* address;
    struct LiquidityPool* next;
} LiquidityPool;

// Function declarations
void swap(LiquidityPool* pool, const char* amount0In, const char* amount1In,
          const char* amount0Out, const char* amount1Out);
LiquidityPool* create_pool(const char* symbol0, uint8_t decimals0, const char* hash0,
                           const char* symbol1, uint8_t decimals1, const char* hash1,
                           const char* reserve0, const char* reserve1, const char* poolAddress);
void display_pool(const LiquidityPool* pool);
void free_pool(LiquidityPool* pool);
char* double_to_string(double value);
double string_to_double(const char* str);

#endif // LIQUIDITY_POOL_H

