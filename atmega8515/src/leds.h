#ifndef LEDS_H
#define LEDS_H

#include <avr/io.h>
#include <avr/pgmspace.h>


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

// PORTA =  0b00000111; // PA0-2 columns HIGH, PA3-7 rows LOW
// PORTB &= 0b00011111; // PB0-4 columns HIGH
// PORTC =  0b00000000; // PC0-7 rows LOW
// PORTD =  0b11111111; // PD0-7 columns HIGH
// PORTE &= 0b00000000; // PE0-2 rows LOW

// DDRA = 0b11111111; // All PORT A pins are outputs
// DDRB |= 0b00011111; // PB0-PB4 outputs
// DDRC = 0b11111111; // All PORT C pins are outputs
// DDRD = 0b11111111; // All PORT D pins are outputs
// DDRE |= 0b00000111; // PE0-PE2 outputs

#define ROW_MASK_A 0b11111000
#define COL_MASK_A 0b00000111

#define ROW_MASK_B 0b00000000
#define COL_MASK_B 0b00011111

#define ROW_MASK_C 0b11111111
#define COL_MASK_C 0b00000000

#define ROW_MASK_D 0b00000000
#define COL_MASK_D 0b11111111

#define ROW_MASK_E 0b00000111
#define COL_MASK_E 0b00000000

#define LED_MASK_A (ROW_MASK_A | COL_MASK_A)
#define LED_MASK_B (ROW_MASK_B | COL_MASK_B)
#define LED_MASK_C (ROW_MASK_C | COL_MASK_C)
#define LED_MASK_D (ROW_MASK_D | COL_MASK_D)
#define LED_MASK_E (ROW_MASK_E | COL_MASK_E)


typedef struct {
    volatile uint8_t *port;
    uint8_t mask;
} LedPin;


// #define LEDS_DOUBLE_BUFFER


static uint8_t row_tbl[48] = {
    0x08,0x10,0x20,0x40,0x80, 0,0,0,0,0,0,0,0,0,0,0,                  // PORTA
    0,0,0,0,0,0,0,0, 0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01,         // PORTC
    0,0,0,0,0, 0x01,0x02,0x04, 0,0,0,0,0,0,0,0                        // PORTE
};
static uint8_t col_tbl[48] = {
    0x03,0x07,0x05,0x07,0x06,0x07,0x07,0x07,
    0x07,0x07,0x07,0x07,0x07,0x07,0x07,0x07,                          // PORTA
    0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1E,0x1F,
    0x1D,0x1F,0x1B,0x1F,0x17,0x1F,0x0F,0x1F,                          // PORTB (bits 0-4)
    0xFF,0x7F,0xFF,0xBF,0xFF,0xDF,0xFF,0xEF,
    0xFF,0xF7,0xFF,0xFB,0xFF,0xFD,0xFF,0xFE                           // PORTD
};


/// @brief 2 bits of brightness values per LED.
///  i       & 0b00000011  // Lowest 2 bits
/// (i >> 2) & 0b00000011  // Bits 2–3
/// (i >> 4) & 0b00000011  // Bits 4–5
/// (i >> 6) & 0b00000011  // Highest 2 bits

#ifdef LEDS_DOUBLE_BUFFER
extern uint8_t *volatile brightness;

extern uint8_t buf_a[64];
extern uint8_t buf_b[64];
#else
extern uint8_t brightness[64];
#endif



void init_led_pins(void);
void set_led_rows_low(void);

static inline uint8_t get_brightness(uint8_t i)
{
    return (brightness[i >> 2] >> ((i & 3) << 1)) & 3;
}

// void add_brightness(uint8_t i);
void add_brightness_v4(uint8_t i);
void process_particle(uint8_t x, uint8_t y);

/// @brief Computes the LED brightness map based on particle positions.
void update_brightness(void);

// ON: row HIGH, col LOW
static inline void set_led_on(uint8_t x, uint8_t y)
{
    const uint8_t *r = &row_tbl[y];
    const uint8_t *c = &col_tbl[x];

    PORTA = (PORTA | r[0])  & (c[0]  | 0xF8);   // 0xF8: don't clear the row bits PA3-7
    PORTB =  PORTB          & (c[16] | 0xE0);   // 0xE0: PB5-7 untouched
    PORTC |= r[16];
    PORTD &= c[32];
    PORTE |= r[32];
}

// OFF: row LOW, col HIGH
static inline void set_led_off(uint8_t x, uint8_t y)
{
    const uint8_t *r = &row_tbl[y];
    const uint8_t *c = &col_tbl[x];

    PORTA = (PORTA & ~r[0])  | (~c[0]  & 0x07);
    PORTB =  PORTB           | (~c[16] & 0x1F);
    PORTC &= ~r[16];
    PORTD |= ~c[32];
    PORTE &= ~r[32];
}

#endif // LEDS_H
