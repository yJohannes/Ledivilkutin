#include "leds.h"

#include <avr/io.h>
#include <avr/sfr_defs.h>
#include <avr/interrupt.h>
#include <avr/common.h>

#include <stdint.h>
#include <string.h>

#include "globals.h"
#include "sim.h"
#include "sim_config.h"



#ifdef LEDS_DOUBLE_BUFFER
uint8_t buf_a[64];
uint8_t buf_b[64];
uint8_t *volatile brightness = buf_a;   // read by ISR
static uint8_t *back_buf  = buf_b;     // written by main
#else
uint8_t brightness[64];
#endif


uint8_t row_tbl[48] = {
    0x08,0x10,0x20,0x40,0x80, 0,0,0,0,0,0,0,0,0,0,0,                  // PORTA
    0,0,0,0,0,0,0,0, 0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01,         // PORTC
    0,0,0,0,0, 0x01,0x02,0x04, 0,0,0,0,0,0,0,0                        // PORTE
};

uint8_t col_tbl[48] = {
    0x03,0x07,0x05,0x07,0x06,0x07,0x07,0x07,
    0x07,0x07,0x07,0x07,0x07,0x07,0x07,0x07,                          // PORTA
    0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1E,0x1F,
    0x1D,0x1F,0x1B,0x1F,0x17,0x1F,0x0F,0x1F,                          // PORTB (bits 0-4)
    0xFF,0x7F,0xFF,0xBF,0xFF,0xDF,0xFF,0xEF,
    0xFF,0xF7,0xFF,0xFB,0xFF,0xFD,0xFF,0xFE                           // PORTD
};


void init_led_pins(void)
{
    // Set to prevent infinite loop in LED interrupt.
    brightness[32+2] = 1u;

    // Signal   Inactive  Active
    // Rows     LOW       HIGH
    // Columns 	HIGH      LOW

    // Set initial inactive output states:
    // Rows = LOW, cols = HIGH
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
}


void set_leds_off(void)
{
    PORTA = (PORTA & ~LED_MASK_A) | COL_MASK_A;
    PORTB = (PORTB & ~LED_MASK_B) | COL_MASK_B;
    PORTC = (PORTC & ~LED_MASK_C) | COL_MASK_C;
    PORTD = (PORTD & ~LED_MASK_D) | COL_MASK_D;
    PORTE = (PORTE & ~LED_MASK_E) | COL_MASK_E;

    // PORTA &= ~ROW_MASK_A;
    // PORTB &= ~ROW_MASK_B;
    // PORTC &= ~ROW_MASK_C;
    // PORTD &= ~ROW_MASK_D;
    // PORTE &= ~ROW_MASK_E;
}


// No const so they stay in SRAM
static uint8_t brightness_masks[4] = {
    0x03, 0x0C, 0x30, 0xC0
};

static uint8_t brightness_increments[4] = {
    0x01, 0x04, 0x10, 0x40
};


// Magic
void add_brightness_v4(uint8_t i)
{
#ifdef LEDS_DOUBLE_BUFFER
    uint8_t *pt = &back_buf[i >> 2];
#else
    uint8_t *pt = &brightness[i >> 2];
#endif

    uint8_t pos = i & 3;

    uint8_t mask = brightness_masks[pos];

    if ((*pt & mask) != mask)
        *pt += brightness_increments[pos];
}

// void add_brightness(uint8_t i)
// {
//     uint8_t shift = (i & 3) << 1;
//     // uint8_t mask = 3 << shift;

//     uint8_t value = (brightness[i >> 2] >> shift) & 3;

//     if (value < 3)
//     {
//         // value += amount;
//         ++value;
//         if (value > 3) value = 3;

//         brightness[i >> 2] =
//             (brightness[i >> 2] & ~(3 << shift)) | (value << shift);
//             // (brightness[i >> 2] & ~mask) | (value << shift);
//     }
// }


void process_particle(uint8_t x, uint8_t y)
{
    uint8_t i = y * LEDS_X + x;

    // Set max brightness
#ifdef LEDS_DOUBLE_BUFFER
    back_buf[i >> 2] |= 3 << ((i & 3) << 1);
#else
    brightness[i >> 2] |= 3 << ((i & 3) << 1);
#endif

    if (x > 0)  add_brightness_v4(i - 1);
    if (x < 15) add_brightness_v4(i + 1);
    if (y > 0)  add_brightness_v4(i - LEDS_Y);
    if (y < 15) add_brightness_v4(i + LEDS_Y);
}

#ifdef LEDS_DOUBLE_BUFFER
void update_brightness(void)
{
    memset(back_buf, 0, 64);

    for (uint8_t i = 0; i < SIM_PARTICLE_COUNT; ++i)
        process_particle(p[i].x >> 4, p[i].y >> 4);

    // Swap atomically
    uint8_t sreg = SREG;
    cli();

    uint8_t *t = brightness;
    brightness = back_buf;
    back_buf = t;
    SREG = sreg;
}
#else


void update_brightness(void)
{
    uint8_t sreg = SREG;
    cli();

    // Ensure LED is off before restoring interrupts.
    set_led_off(LED_COL(current_led), LED_ROW(current_led));

    // Clear the framebuffer.
    memset(&brightness[0], 0, sizeof(brightness));

    for (uint8_t i = 0u; i < SIM_PARTICLE_COUNT; ++i)
    {
        uint8_t led_x = p[i].x >> 4;
        uint8_t led_y = p[i].y >> 4;

        process_particle(led_x, led_y);
    }


    SREG = sreg;
}
#endif

