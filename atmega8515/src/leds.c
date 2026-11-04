#include "leds.h"

#include <avr/io.h>
#include <avr/sfr_defs.h>

#include <stdint.h>
#include <string.h>

#include "sim.h"
#include "sim_config.h"


uint8_t brightness[64];

const LedPin ROW_PINS[LEDS_Y] = {
    {&PORTA, _BV(PA3)}, // PIN_ROW_0
    {&PORTA, _BV(PA4)}, // PIN_ROW_1
    {&PORTA, _BV(PA5)}, // PIN_ROW_2
    {&PORTA, _BV(PA6)}, // PIN_ROW_3
    {&PORTA, _BV(PA7)}, // PIN_ROW_4
    {&PORTE, _BV(PE0)}, // PIN_ROW_5
    {&PORTE, _BV(PE1)}, // PIN_ROW_6
    {&PORTE, _BV(PE2)}, // PIN_ROW_7
    {&PORTC, _BV(PC7)}, // PIN_ROW_8
    {&PORTC, _BV(PC6)}, // PIN_ROW_9
    {&PORTC, _BV(PC5)}, // PIN_ROW_10
    {&PORTC, _BV(PC4)}, // PIN_ROW_11
    {&PORTC, _BV(PC3)}, // PIN_ROW_12
    {&PORTC, _BV(PC2)}, // PIN_ROW_13
    {&PORTC, _BV(PC1)}, // PIN_ROW_14
    {&PORTC, _BV(PC0)}  // PIN_ROW_15
};

const LedPin COL_PINS[LEDS_X] = {
    {&PORTA, _BV(PA2)}, // PIN_COL_0
    {&PORTD, _BV(PD7)}, // PIN_COL_1
    {&PORTA, _BV(PA1)}, // PIN_COL_2
    {&PORTD, _BV(PD6)}, // PIN_COL_3
    {&PORTA, _BV(PA0)}, // PIN_COL_4
    {&PORTD, _BV(PD5)}, // PIN_COL_5
    {&PORTB, _BV(PB0)}, // PIN_COL_6
    {&PORTD, _BV(PD4)}, // PIN_COL_7
    {&PORTB, _BV(PB1)}, // PIN_COL_8
    {&PORTD, _BV(PD3)}, // PIN_COL_9
    {&PORTB, _BV(PB2)}, // PIN_COL_10
    {&PORTD, _BV(PD2)}, // PIN_COL_11
    {&PORTB, _BV(PB3)}, // PIN_COL_12
    {&PORTD, _BV(PD1)}, // PIN_COL_13
    {&PORTB, _BV(PB4)}, // PIN_COL_14
    {&PORTD, _BV(PD0)}  // PIN_COL_15
};


void init_led_pins(void)
{
    // Set to prevent infinite loop in LED interrupt.
    brightness[0] = 1u;

    // Signal   Inactive  Active
    // Rows     LOW       HIGH
    // Columns 	HIGH      LOW

    // Set initial inactive output states:
    // Rows = HIGH, cols = LOW

    PORTA = (PORTA & ~LED_MASK_A) | COL_MASK_A;
    PORTB = (PORTB & ~LED_MASK_B) | COL_MASK_B;
    PORTC = (PORTC & ~LED_MASK_C) | COL_MASK_C;
    PORTD = (PORTD & ~LED_MASK_D) | COL_MASK_D;
    PORTE = (PORTE & ~LED_MASK_E) | COL_MASK_E;

    // Configure LED pins as outputs.

    DDRA |= LED_MASK_A;
    DDRB |= LED_MASK_B;
    DDRC |= LED_MASK_C;
    DDRD |= LED_MASK_D;
    DDRE |= LED_MASK_E;

    // PORTA =  0b00000111; // PA0-2 columns HIGH, PA3-7 rows LOW
    // PORTB &= 0b00011111; // PB0-4 columns HIGH
    // PORTC =  0b00000000; // PC0-7 rows LOW
    // PORTD =  0b11111111; // PD0-7 columns HIGH
    // PORTE &= 0b00000000; // PE0-2 rows LOW


    // Configure LED pins as outputs without changing PB5-7

    // DDRA = 0b11111111; // All PORT A pins are outputs
    // DDRB |= 0b00011111; // PB0-PB4 outputs
    // DDRC = 0b11111111; // All PORT C pins are outputs
    // DDRD = 0b11111111; // All PORT D pins are outputs
    // DDRE |= 0b00000111; // PE0-PE2 outputs
}


void set_led_rows_low(void)
{
    PORTA &= ~ROW_MASK_A;
    PORTB &= ~ROW_MASK_B;
    PORTC &= ~ROW_MASK_C;
    PORTD &= ~ROW_MASK_D;
    PORTE &= ~ROW_MASK_E;
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
