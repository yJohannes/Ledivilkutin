#include <stdint.h>


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
