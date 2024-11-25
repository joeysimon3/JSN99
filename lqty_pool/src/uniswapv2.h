#ifndef UNISWAPV2_H
#define UNISWAPV2_H

#include "liquidity_pool.h"
#include <stdbool.h>

// Function declarations
double calculate_price(const LiquidityPool* pool, bool isToken0ToToken1);
void display_pool_price(const LiquidityPool* pool);

#endif // UNISWAPV2_H

