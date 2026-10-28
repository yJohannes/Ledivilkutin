/// @file int_arithmetic.h
/// @author Johannes Ylinen
/// @brief Functions to prevent under/overflow.

#ifndef INT_ARITHMETIC_H
#define INT_ARITHMETIC_H

#include <stdint.h>


/// @brief Saturation arithmetic using __builtin_add_overflow.
/// @see https://en.wikipedia.org/wiki/Saturation_arithmetic
uint8_t sat_add_u8_s8(uint8_t x, int8_t y);
int8_t sat_add_s8(int8_t x, int8_t y);

/// @brief Try variants: Returns x if overflowed else x + y.
static inline uint8_t try_add_u8_s8(uint8_t x, int8_t y)
{
    uint8_t result;
    return __builtin_add_overflow(x, y, &result) ? x : result;
}

static inline int8_t try_add_s8(int8_t x, int8_t y)
{
    int8_t result;
    return __builtin_add_overflow(x, y, &result) ? x : result;
}


#endif // INT_ARITHMETIC_H
