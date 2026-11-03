#include "leds.h"

#include <avr/io.h>

#include <stdint.h>
#include <string.h>

#include "sim.h"
#include "sim_config.h"


const LedPin ROW_PINS[LEDS_Y] = {
    {&PORTA, PA3}, // PIN_ROW_0
    {&PORTA, PA4}, // PIN_ROW_1
    {&PORTA, PA5}, // PIN_ROW_2
    {&PORTA, PA6}, // PIN_ROW_3
    {&PORTA, PA7}, // PIN_ROW_4
    {&PORTE, PE0}, // PIN_ROW_5
    {&PORTE, PE1}, // PIN_ROW_6
    {&PORTE, PE2}, // PIN_ROW_7
    {&PORTC, PC7}, // PIN_ROW_8
    {&PORTC, PC6}, // PIN_ROW_9
    {&PORTC, PC5}, // PIN_ROW_10
    {&PORTC, PC4}, // PIN_ROW_11
    {&PORTC, PC3}, // PIN_ROW_12
    {&PORTC, PC2}, // PIN_ROW_13
    {&PORTC, PC1}, // PIN_ROW_14
    {&PORTC, PC0}  // PIN_ROW_15
};

const LedPin COL_PINS[LEDS_X] = {
    {&PORTA, PA2}, // PIN_COL_0
    {&PORTD, PD7}, // PIN_COL_1
    {&PORTA, PA1}, // PIN_COL_2
    {&PORTD, PD6}, // PIN_COL_3
    {&PORTA, PA0}, // PIN_COL_4
    {&PORTD, PD5}, // PIN_COL_5
    {&PORTB, PB0}, // PIN_COL_6
    {&PORTD, PD4}, // PIN_COL_7
    {&PORTB, PB1}, // PIN_COL_8
    {&PORTD, PD3}, // PIN_COL_9
    {&PORTB, PB2}, // PIN_COL_10
    {&PORTD, PD2}, // PIN_COL_11
    {&PORTB, PB3}, // PIN_COL_12
    {&PORTD, PD1}, // PIN_COL_13
    {&PORTB, PB4}, // PIN_COL_14
    {&PORTD, PD0}  // PIN_COL_15
};


uint8_t brightness[64];


void init_led_pins(void)
{
    // Set to prevent infinite loop in LED interrupt.
    brightness[0] = 1u;

    // Signal   Inactive  Active
    // Rows     LOW       HIGH
    // Columns 	HIGH      LOW

    // Initial output states
    // Rows = HIGH, cols = LOW
    PORTA = 0b00000111; // PA0-2 columns HIGH, PA3-7 rows LOW
    PORTB &= 0b00011111; // PB0-4 columns HIGH
    PORTC = 0b00000000; // PC0-7 rows LOW
    PORTD = 0b11111111; // PD0-7 columns HIGH
    PORTE &= 0b00000000; // PE0-2 rows LOW

    // 3 4 5 6 7 2 1 0
    DDRA = 0b11111111; // All PORT A pins are outputs

    // 0 1 2 3 4
    DDRB |= 0b00001111; // PB0-PB4 outputs

    // 7 6 5 4 3 2 1 0
    DDRC = 0b11111111; // All PORT C pins are outputs

    // 7 6 5 4 3 2 1 0
    DDRD = 0b11111111; // All PORT D pins are outputs

    // 0 1 2
    DDRE |= 0b00000111; // PE0-PE2 outputs
}


uint8_t brightness_masks[4] = {
    0x03, 0x0C, 0x30, 0xC0
};

uint8_t brightness_increments[4] = {
    0x01, 0x04, 0x10, 0x40
};


// Magic
void add_brightness_v4(uint8_t i)
{
    uint8_t *p = &brightness[i >> 2];
    uint8_t pos = i & 3;

    uint8_t mask = brightness_masks[pos];

    if ((*p & mask) != mask)
        *p += brightness_increments[pos];
}

// void add_brightness(uint8_t i, uint8_t amount)
void add_brightness(uint8_t i)
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

void process_particle(uint8_t x, uint8_t y)
{
    uint8_t i = y * LEDS_X + x;

    // Set max brightness
    brightness[i >> 2] |= 3 << ((i & 3) << 1);

    if (x > 0)  add_brightness_v4(i - 1);
    if (x < 15) add_brightness_v4(i + 1);
    if (y > 0)  add_brightness_v4(i - 16);
    if (y < 15) add_brightness_v4(i + 16);
}


void update_brightness(void)
{
    // Clear frame buffer
    memset(brightness, 0, sizeof(brightness));

    for (uint8_t i = 0u; i < SIM_PARTICLE_COUNT; ++i)
    {
        uint8_t led_x = p[i].x >> 4;
        uint8_t led_y = p[i].y >> 4;

        process_particle(led_x, led_y);
    }
}


#if 0
// Use conventional screen coordinates (0, 0) = top-left corner
#define PIN_ROW_0   PA3
#define PIN_ROW_1   PA4
#define PIN_ROW_2   PA5
#define PIN_ROW_3   PA6
#define PIN_ROW_4   PA7
#define PIN_ROW_5   PE0
#define PIN_ROW_6   PE1
#define PIN_ROW_7   PE2
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
#endif
