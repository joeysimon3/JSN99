#include "uniswapv2.h"
#include <stdio.h>
#include <math.h>

// Calculate the price for a given direction
double calculate_price(const LiquidityPool* pool, bool isToken0ToToken1) {
    if (!pool || !pool->reserve0 || !pool->reserve1) {
        fprintf(stderr, "Invalid pool reserves.\n");
        return -1.0;
    }

    // Convert reserves to doubles
    double reserveA, reserveB;
    int decimalsA, decimalsB;

    if (isToken0ToToken1) {
        reserveA = string_to_double(pool->reserve0);
        reserveB = string_to_double(pool->reserve1);
        decimalsA = pool->token0.decimals;
        decimalsB = pool->token1.decimals;
    } else {
        reserveA = string_to_double(pool->reserve1);
        reserveB = string_to_double(pool->reserve0);
        decimalsA = pool->token1.decimals;
        decimalsB = pool->token0.decimals;
    }

    // Debugging: Print raw reserves
    printf("Raw Reserves: reserveA = %.0f, reserveB = %.0f\n", reserveA, reserveB);

    // Scale reserves by token decimals
    double scaledReserveA = reserveA / pow(10, decimalsA);
    double scaledReserveB = reserveB / pow(10, decimalsB);

    // Debugging: Print scaled reserves
    printf("Scaled Reserves: scaledReserveA = %.12f, scaledReserveB = %.12f\n", scaledReserveA, scaledReserveB);

    // Calculate the price
    double price = scaledReserveB / scaledReserveA;

    // Debugging: Print the calculated price
    printf("Calculated Price: %.12f\n", price);

    return price;
}

