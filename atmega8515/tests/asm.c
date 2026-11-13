#include <stdint.h>
#include <avr/io.h>

#define max_(a, b) ((a) > (b) ? (a) : (b))
#define min_(a, b) ((a) < (b) ? (a) : (b))

#define I2C_PIN_SDA PB5
#define I2C_PIN_SCL PB6

#define I2C_SDA_MASK _BV(I2C_PIN_SDA)
#define I2C_SCL_MASK _BV(I2C_PIN_SCL)


#define IO_SBI(reg, bit) __asm__ __volatile__("sbi %0,%1" :: "I"(_SFR_IO_ADDR(reg)), "I"(bit))
#define IO_CBI(reg, bit) __asm__ __volatile__("cbi %0,%1" :: "I"(_SFR_IO_ADDR(reg)), "I"(bit))

__attribute__((noinline, used))
static void i2c_sda_low_asm(void)     { IO_CBI(PORTB, I2C_PIN_SDA); IO_SBI(DDRB, I2C_PIN_SDA); }

__attribute__((noinline, used))
static void i2c_sda_low(void)
{
    PORTB &= ~I2C_SDA_MASK;
    DDRB |= I2C_SDA_MASK;
}


__attribute__((noinline, used))
static uint8_t division(uint16_t x)
{
    uint8_t com_x = x / 75;
    return com_x;
}


__attribute__((noinline, used))
static uint8_t cheap(uint16_t x)
{
    uint16_t com_x = ((uint32_t)x * 3496) >> 18;
    return com_x;
}



uint8_t brightness[64];


static uint8_t masks[4] = {
    0x03, 0x0C, 0x30, 0xC0
};

static uint8_t increments[4] = {
    0x01, 0x04, 0x10, 0x40
};

__attribute__((noinline, used))
static void leds_add_brightness_v4(uint8_t i)
{
    uint8_t *p = &brightness[i >> 2];
    uint8_t pos = i & 3;

    uint8_t mask = masks[pos];

    if ((*p & mask) != mask)
        *p += increments[pos];
}

__attribute__((noinline, used))
static void leds_add_brightness_v3(uint8_t i)
{
    uint8_t shift = (i & 3) << 1;
    uint8_t *p = &brightness[i >> 2];

    uint8_t mask = 3u << shift;
    uint8_t value = *p & mask;

    if (value != mask)
        *p += 1u << shift;
}

__attribute__((noinline, used))
static void leds_add_brightness_v2(uint8_t i)
{
    uint8_t shift = (i & 3) << 1;
    uint8_t *p = &brightness[i >> 2];

    uint8_t mask = 3u << shift;
    *p += ((*p & mask) != mask) << shift;
}


// void leds_add_brightness(uint8_t i, uint8_t amount)
__attribute__((noinline, used))
static void leds_add_brightness(uint8_t i)
{
    uint8_t shift = (i & 3) << 1;
    // uint8_t mask = 3 << shift;

    uint8_t value = (brightness[i >> 2] >> shift) & 3;

    if (value < 3)
    {
        // value += amount;
        ++value;
        if (value > 3) value = 3;

        brightness[i >> 2] =
            (brightness[i >> 2] & ~(3 << shift)) | (value << shift);
            // (brightness[i >> 2] & ~mask) | (value << shift);
    }
}

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
