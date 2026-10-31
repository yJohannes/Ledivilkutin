#include <stdint.h>

#define max_(a, b) ((a) > (b) ? (a) : (b))
#define min_(a, b) ((a) < (b) ? (a) : (b))


// Version 1: Explicit overflow check
__attribute__((noinline, used))
static uint8_t approx_dist_manual(uint8_t ax, uint8_t ay, uint8_t *L)
{
    uint8_t hi = max_(ax, ay);
    uint8_t lo = min_(ax, ay) >> 1;

    if (hi > UINT8_MAX - lo)
        return 0;

    *L = hi + lo;
    return 1;
}


// Version 2: GCC builtin
__attribute__((noinline, used))
static uint8_t approx_dist_builtin(uint8_t ax, uint8_t ay, uint8_t *L)
{
    return !__builtin_add_overflow(
        max_(ax, ay),
        min_(ax, ay) >> 1,
        L
    );
}

__attribute__((noinline, used))
static uint8_t try_add_u8_s8_of(uint8_t x, int8_t y)
{
    uint8_t result;
    return __builtin_add_overflow(x, y, &result) ? x : result;
}

__attribute__((noinline, used))
static uint8_t try_add_u8_s8(uint8_t x, int8_t y)
{
    if (y < 0) {
        if (x < (uint8_t)-y)
            return x;
    } else {
        if (x > UINT8_MAX - (uint8_t)y)
            return x;
    }

    return x + y;
}

__attribute__((noinline, used))
static uint8_t should_skip(uint8_t a, uint8_t b)
{
    uint8_t diff;
    return __builtin_sub_overflow(a, b, &diff);
}

__attribute__((noinline, used))
static uint8_t abs_diff_u8(uint8_t a, uint8_t b)
{
    return a > b ? a - b : b - a;
}

__attribute__((noinline, used))
static uint8_t abs_diff_u16(uint8_t a, uint8_t b)
{
    int16_t val = a - b;
    return val > 0 ? val : -val;
}


int main()
{

}
