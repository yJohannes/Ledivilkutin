#ifndef LEDS_H
#define LEDS_H

#include <avr/io.h>

#include <stdint.h>

#include "sim_config.h"


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


void init_led_pins(void);
uint8_t get_brightness(uint8_t i);
void add_brightness(uint8_t i);
void add_brightness_v4(uint8_t i);
void process_particle(uint8_t x, uint8_t y);
void update_brightness(void);

#endif // LEDS_H
