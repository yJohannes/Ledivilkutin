/// @file saturation_arithmetic.h
/// @author Johannes Ylinen
/// @brief Saturation arithmetic using __builtin_add_overflow.
/// @see https://en.wikipedia.org/wiki/Saturation_arithmetic


#ifndef SATURATION_ARITHMETIC_H
#define SATURATION_ARITHMETIC_H

#include <stdint.h>


uint8_t sat_add_u8_s8(uint8_t x, int8_t y);
int8_t sat_add_s8(int8_t x, int8_t y);

/// Skip variants: skips addition and returns x.

int8_t sat_skip_add_s8(int8_t x, int8_t y);


#endif // SATURATION_ARITHMETIC_H
