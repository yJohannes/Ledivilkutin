
#include <stdint.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

uint8_t sat_add_u8_s8(uint8_t x, int8_t y)
{
    if (y < 0)
    {
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

// Skip variants
int8_t sat_skip_add_s8(int8_t x, int8_t y)
{
    int8_t result;

    if (__builtin_add_overflow(x, y, &result))
        return x;

    return result;
}

static int test_count = 0;
static int fail_count = 0;

#define CHECK_U8(x, y, expected) do { \
    uint8_t a = (x); \
    int8_t b = (y); \
    uint8_t exp = (expected); \
    uint8_t got = sat_add_u8_s8(a, b); \
    int pass = got == exp; \
    printf("%s: %u + (%d) = %u, expected %u\n", \
           pass ? "PASS" : "FAIL", \
           (unsigned)a, (int)b, (unsigned)got, (unsigned)exp); \
    ++test_count; \
    if (!pass) ++fail_count; \
} while (0)

#define CHECK_S8(x, y, expected) do { \
    int8_t a = (x); \
    int8_t b = (y); \
    int8_t exp = (expected); \
    int8_t got = sat_add_s8(a, b); \
    int pass = got == exp; \
    printf("%s: %d + (%d) = %d, expected %d\n", \
           pass ? "PASS" : "FAIL", \
           (int)a, (int)b, (int)got, (int)exp); \
    ++test_count; \
    if (!pass) ++fail_count; \
} while (0)

int main(void)
{
    puts("=== uint8_t + int8_t (saturating) ===");

    CHECK_U8(10, -1, 9);
    CHECK_U8(10, -10, 0);
    CHECK_U8(10, -11, 0);
    CHECK_U8(0, INT8_MIN, 0);
    CHECK_U8(255, INT8_MIN, 127);
    CHECK_U8(250, 10, 255);
    CHECK_U8(255, 1, 255);
    CHECK_U8(100, 20, 120);
    CHECK_U8(0, 127, 127);

    puts("\n=== int8_t + int8_t (saturating) ===");

    CHECK_S8(10, 20, 30);
    CHECK_S8(127, 1, INT8_MAX);
    CHECK_S8(-128, -1, INT8_MIN);
    CHECK_S8(100, 50, INT8_MAX);
    CHECK_S8(-100, -50, INT8_MIN);
    CHECK_S8(-100, 50, -50);
    CHECK_S8(100, -50, 50);
    CHECK_S8(-128, 127, -1);
    CHECK_S8(127, -128, -1);

    printf("\n%d tests, %d failures\n",
           test_count, fail_count);

    return fail_count ? EXIT_FAILURE : EXIT_SUCCESS;
}
