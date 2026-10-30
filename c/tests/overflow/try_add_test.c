#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <limits.h>

uint8_t try_add_u8_s8(uint8_t x, int8_t y)
{
    if (y < 0) {
        uint8_t magnitude = (uint8_t)(-(y + 1)) + 1u;
        return x < magnitude ? x : x - magnitude;
    }

    uint8_t result;
    if (__builtin_add_overflow(x, (uint8_t)y, &result))
        return x;

    return result;
}

int main(void)
{
    // Positive additions
    assert(try_add_u8_s8(10, 5) == 15);
    assert(try_add_u8_s8(0, 127) == 127);
    assert(try_add_u8_s8(128, 127) == 255);

    // Positive overflow: return original x
    assert(try_add_u8_s8(250, 10) == 250);
    assert(try_add_u8_s8(255, 1) == 255);
    assert(try_add_u8_s8(200, 127) == 200);

    // Negative additions
    assert(try_add_u8_s8(10, -5) == 5);
    assert(try_add_u8_s8(128, -128) == 0);
    assert(try_add_u8_s8(255, -1) == 254);
    assert(try_add_u8_s8(0, 0) == 0);

    // Negative underflow: return original x
    assert(try_add_u8_s8(4, -5) == 4);
    assert(try_add_u8_s8(0, -1) == 0);
    assert(try_add_u8_s8(100, -128) == 100);

    // Boundary cases
    assert(try_add_u8_s8(255, 0) == 255);
    assert(try_add_u8_s8(128, -128) == 0);
    assert(try_add_u8_s8(127, -128) == 127);
    assert(try_add_u8_s8(128, 127) == 255);
    assert(try_add_u8_s8(129, 127) == 129);

    puts("All tests passed.");
    return 0;
}
