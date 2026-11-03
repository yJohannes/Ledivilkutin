#ifndef LEDS_H
#define LEDS_H

#include <avr/io.h>

#include <stdint.h>

#include "sim.h"
#include "sim_config.h"

// #define N          256
// #define F          96
// #define T          (1.0f / F)
// #define T_ON       ((T / N))
// #define T_OFF      (T - T_ON)
// #define T_ON_US    (T_ON * 1000000UL)
// #define T_OFF_US   (T_OFF * 1000000UL)

// #define TIMER_F = 1 / T_ON_US

// Use conventional screen coordinates (0, 0) = top-left corner
#define PIN_ROW_0   PA3
#define PIN_ROW_1   PA4
#define PIN_ROW_2   PA5
#define PIN_ROW_3   PA6
#define PIN_ROW_4   PA7
#define PIN_ROW_5   PB5
#define PIN_ROW_6   PB6
#define PIN_ROW_7   PB7
#define PIN_ROW_8   PC7
#define PIN_ROW_9   PC6
#define PIN_ROW_10  PC5
#define PIN_ROW_11  PC4
#define PIN_ROW_12  PC3
#define PIN_ROW_13  PC2
#define PIN_ROW_14  PC1
#define PIN_ROW_15  PC0

#define PIN_COL_0   PA2
#define PIN_COL_1   PD7
#define PIN_COL_2   PA1
#define PIN_COL_3   PD6
#define PIN_COL_4   PA0
#define PIN_COL_5   PD5
#define PIN_COL_6   PB0
#define PIN_COL_7   PD4
#define PIN_COL_8   PB1
#define PIN_COL_9   PD3
#define PIN_COL_10  PB2
#define PIN_COL_11  PD2
#define PIN_COL_12  PB3
#define PIN_COL_13  PD1
#define PIN_COL_14  PB4
#define PIN_COL_15  PD0


typedef struct {
    volatile uint8_t *port;
    uint8_t mask;
} LedPin;

extern const LedPin ROW_PINS[LEDS_Y];
extern const LedPin COL_PINS[LEDS_X];

// extern const uint8_t ROW_PIN[LEDS_Y];
// extern const uint8_t COL_PIN[LEDS_X];

/// @brief 2 bits of brightness values per LED.
extern uint8_t brightness[64];

//  i       & 0b00000011  // Lowest 2 bits
// (i >> 2) & 0b00000011  // Bits 2–3
// (i >> 4) & 0b00000011  // Bits 4–5
// (i >> 6) & 0b00000011  // Highest 2 bits

void init_led_pins(void);
uint8_t get_brightness(uint8_t i);
// void add_brightness(uint8_t i, uint8_t amount);
void add_brightness(uint8_t i);
void process_particle(uint8_t x, uint8_t y);
void update_brightness(void);

#endif // LEDS_H
