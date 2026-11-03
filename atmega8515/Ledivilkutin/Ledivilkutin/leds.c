#include "leds.h"

#include <avr/io.h>

#include <stdint.h>
#include <string.h>

const LedPin ROW_PINS[LEDS_Y] = {
    {&PORTA, PA3}, // PIN_ROW_0
    {&PORTA, PA4}, // PIN_ROW_1
    {&PORTA, PA5}, // PIN_ROW_2
    {&PORTA, PA6}, // PIN_ROW_3
    {&PORTA, PA7}, // PIN_ROW_4
    {&PORTD, PB5}, // PIN_ROW_5
    {&PORTD, PB6}, // PIN_ROW_6
    {&PORTD, PB7}, // PIN_ROW_7
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

// const uint8_t row_pins[16] = {
//     PIN_ID(PORT_A, PA3),
//     PIN_ID(PORT_A, PA4),
//     PIN_ID(PORT_A, PA5),
//     PIN_ID(PORT_A, PA6),
//     PIN_ID(PORT_A, PA7),
//     PIN_ID(PORT_E, PE0),
//     PIN_ID(PORT_E, PE1),
//     PIN_ID(PORT_E, PE2),
//     PIN_ID(PORT_C, PC7),
//     PIN_ID(PORT_C, PC6),
//     PIN_ID(PORT_C, PC5),
//     PIN_ID(PORT_C, PC4),
//     PIN_ID(PORT_C, PC3),
//     PIN_ID(PORT_C, PC2),
//     PIN_ID(PORT_C, PC1),
//     PIN_ID(PORT_C, PC0)
// };

// const uint8_t col_pins[16] = {
//     PIN_ID(PORT_A, PA2),
//     PIN_ID(PORT_D, PD7),
//     PIN_ID(PORT_A, PA1),
//     PIN_ID(PORT_D, PD6),
//     PIN_ID(PORT_A, PA0),
//     PIN_ID(PORT_D, PD5),
//     PIN_ID(PORT_B, PB0),
//     PIN_ID(PORT_D, PD4),
//     PIN_ID(PORT_B, PB1),
//     PIN_ID(PORT_D, PD3),
//     PIN_ID(PORT_B, PB2),
//     PIN_ID(PORT_D, PD2),
//     PIN_ID(PORT_B, PB3),
//     PIN_ID(PORT_D, PD1),
//     PIN_ID(PORT_B, PB4),
//     PIN_ID(PORT_D, PD0)
// };


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

    // 3 4 5 6 7 2 1 0
    DDRA = 0b11111111; // All PORT A pins are outputs

    // 0 1 2 3 4
    DDRB |= 0b11101111; // PB0-PB4 outputs

    // 7 6 5 4 3 2 1 0
    DDRC = 0b11111111; // All PORT C pins are outputs

    // 7 6 5 4 3 2 1 0
    DDRD = 0b11111111; // All PORT D pins are outputs
}


uint8_t get_brightness(uint8_t i)
{
    return (brightness[i >> 2] >> ((i & 3) << 1)) & 3;
}


uint8_t masks[4] = {
    0x03, 0x0C, 0x30, 0xC0
};

uint8_t increments[4] = {
    0x01, 0x04, 0x10, 0x40
};


// Dark magic
static void add_brightness_v4(uint8_t i)
{
    uint8_t *p = &brightness[i >> 2];
    uint8_t pos = i & 3;

    uint8_t mask = masks[pos];

    if ((*p & mask) != mask)
        *p += increments[pos];
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
