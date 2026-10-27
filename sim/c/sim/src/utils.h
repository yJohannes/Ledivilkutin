#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>


static inline uint16_t isqrt_u32(uint32_t n)
{
    uint32_t x = 0;
    uint32_t bit = 1u << 30;

    while (bit > n)
        bit >>= 2;

    while (bit != 0)
    {
        if (n >= x + bit)
        {
            n -= x + bit;
            x = (x >> 1) + bit;
        }
        else
        {
            x >>= 1;
        }

        bit >>= 2;
    }

    return (uint16_t)x;
}


#endif // UTILS_H
