#ifndef VEC2_H
#define VEC2_H

#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif


typedef struct
{
    int8_t x;
    int8_t y;
} vec2i;


typedef struct
{
    uint8_t x;
    uint8_t y;
} vec2u;


typedef struct
{
    uint16_t x;
    uint16_t y;
} vec2u16;


#ifdef __cplusplus
}
#endif

#endif // VEC2_H
