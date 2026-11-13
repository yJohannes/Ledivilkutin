#ifndef LEDS_H
#define LEDS_H

#include <avr/io.h>

#include <stdint.h>

#include "sim_config.h"


// #define PIN_ROW_0   PA3
// #define PIN_ROW_1   PA4
// #define PIN_ROW_2   PA5
// #define PIN_ROW_3   PA6
// #define PIN_ROW_4   PA7
// #define PIN_ROW_5   PE0
// #define PIN_ROW_6   PE1
// #define PIN_ROW_7   PE2
// #define PIN_ROW_8   PC7
// #define PIN_ROW_9   PC6
// #define PIN_ROW_10  PC5
// #define PIN_ROW_11  PC4
// #define PIN_ROW_12  PC3
// #define PIN_ROW_13  PC2
// #define PIN_ROW_14  PC1
// #define PIN_ROW_15  PC0

// #define PIN_COL_0   PA2
// #define PIN_COL_1   PD7
// #define PIN_COL_2   PA1
// #define PIN_COL_3   PD6
// #define PIN_COL_4   PA0
// #define PIN_COL_5   PD5
// #define PIN_COL_6   PB0
// #define PIN_COL_7   PD4
// #define PIN_COL_8   PB1
// #define PIN_COL_9   PD3
// #define PIN_COL_10  PB2
// #define PIN_COL_11  PD2
// #define PIN_COL_12  PB3
// #define PIN_COL_13  PD1
// #define PIN_COL_14  PB4
// #define PIN_COL_15  PD0

#define ROW_MASK_A 0b11111000
#define COL_MASK_A 0b00000111

#define ROW_MASK_B 0b11100000
#define COL_MASK_B 0b00011111

#define ROW_MASK_C 0b11111111
#define COL_MASK_C 0b00000000

#define ROW_MASK_D 0b00000000
#define COL_MASK_D 0b11111111


#define LED_MASK_A (ROW_MASK_A | COL_MASK_A)
#define LED_MASK_B (ROW_MASK_B | COL_MASK_B)
#define LED_MASK_C (ROW_MASK_C | COL_MASK_C)
#define LED_MASK_D (ROW_MASK_D | COL_MASK_D)


typedef struct {
    volatile uint8_t *port;
    uint8_t mask;
} LedPin;

extern const LedPin ROW_PINS[LEDS_Y];
extern const LedPin COL_PINS[LEDS_X];


/// @brief 2 bits of brightness values per LED.
///  i       & 0b00000011  // Lowest 2 bits
/// (i >> 2) & 0b00000011  // Bits 2–3
/// (i >> 4) & 0b00000011  // Bits 4–5
/// (i >> 6) & 0b00000011  // Highest 2 bits
extern uint8_t brightness[64];


void leds_init_pins(void);
void set_led_rows_low(void);

static inline uint8_t leds_get_brightness(uint8_t i)
{
    return (brightness[i >> 2] >> ((i & 3) << 1)) & 3;
}

void leds_add_brightness(uint8_t i);
void leds_add_brightness_v4(uint8_t i);
void leds_add_particle(uint8_t x, uint8_t y);

/// @brief Computes the LED brightness map based on particle positions.
void leds_update_brightness(void);

#endif // LEDS_H
