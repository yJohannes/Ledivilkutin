#include "int_arithmetic.h"

#include <stdint.h>
#include <limits.h>


uint8_t sat_add_u8_s8(uint8_t x, int8_t y)
{
    if (y < 0) {
        uint8_t magnitude = (uint8_t)(-(y + 1)) + 1u;
        return x < magnitude ? 0 : x - magnitude;
    }

    uint8_t result;
    if (__builtin_add_overflow(x, (uint8_t)y, &result))
        return UINT8_MAX;

    return result;
}


int8_t sat_add_s8(int8_t x, int8_t y)
{
    int8_t result;

    if (__builtin_add_overflow(x, y, &result))
        return x < 0 ? INT8_MIN : INT8_MAX;

    return result;
}

