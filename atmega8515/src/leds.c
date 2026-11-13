#include "leds.h"

#include <avr/io.h>
#include <avr/sfr_defs.h>
#include <avr/interrupt.h>
#include <avr/common.h>

#include <stdint.h>
#include <string.h>

#include "sim.h"
#include "sim_config.h"


#ifdef LEDS_DOUBLE_BUFFER
uint8_t buf_a[64];
uint8_t buf_b[64];
uint8_t *volatile brightness = buf_a;   // read by ISR
static uint8_t *back_buf  = buf_b;     // written by main
#else
uint8_t brightness[64];
#endif // LEDS_DOUBLE_BUFFER


uint8_t leds_row_tbl[48] = {
    0x08,0x10,0x20,0x40,0x80, 0,0,0,0,0,0,0,0,0,0,0,                  // PORTA
    0,0,0,0,0,0,0,0, 0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01,         // PORTC
    0,0,0,0,0, 0x01,0x02,0x04, 0,0,0,0,0,0,0,0                        // PORTE
};

uint8_t leds_col_tbl[48] = {
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


void leds_set_all_off(void)
{
    PORTA = (PORTA & ~LED_MASK_A) | COL_MASK_A;
    PORTB = (PORTB & ~LED_MASK_B) | COL_MASK_B;
    PORTC = (PORTC & ~LED_MASK_C) | COL_MASK_C;
    PORTD = (PORTD & ~LED_MASK_D) | COL_MASK_D;
    PORTE = (PORTE & ~LED_MASK_E) | COL_MASK_E;
}


void leds_add_brightness(uint8_t led_idx)
{
#ifdef LEDS_DOUBLE_BUFFER
    uint8_t *pt = &back_buf[led_idx >> 2];
#else
    uint8_t *pt = &brightness[led_idx >> 2];
#endif // LEDS_DOUBLE_BUFFER

    uint8_t pos = led_idx & 3;

    uint8_t mask = leds_brightness_masks[pos];

    // Perform magic
    if ((*pt & mask) != mask)
        *pt += leds_brightness_increments[pos];
}


void leds_add_particle(uint8_t x, uint8_t y)
{
    uint8_t i = y * LEDS_X + x;

    // Set max brightness
#ifdef LEDS_DOUBLE_BUFFER
    back_buf[i >> 2] |= 3 << ((i & 3) << 1);
#else
    brightness[i >> 2] |= 3 << ((i & 3) << 1);
#endif // LEDS_DOUBLE_BUFFER

    if (x > 0)  leds_add_brightness(i - 1);
    if (x < 15) leds_add_brightness(i + 1);
    if (y > 0)  leds_add_brightness(i - LEDS_Y);
    if (y < 15) leds_add_brightness(i + LEDS_Y);
}

#ifdef LEDS_DOUBLE_BUFFER
void leds_update_brightness(void)
{
    memset(back_buf, 0, 64);

    for (uint8_t i = 0; i < SIM_PARTICLE_COUNT; ++i)
        leds_add_particle(p[i].x >> 4, p[i].y >> 4);

    // Swap atomically
    uint8_t sreg = SREG;
    cli();

    uint8_t *t = brightness;
    brightness = back_buf;
    back_buf = t;
    SREG = sreg;
}
#else
void leds_update_brightness(void)
{
    uint8_t sreg = SREG;
    cli();

    // Ensure LED is off before restoring interrupts.
    leds_set_led_off(LED_COL(leds_current_led), LED_ROW(leds_current_led));

    // Clear the framebuffer.
    memset(&brightness[0], 0, sizeof(brightness));

    for (uint8_t i = 0u; i < SIM_PARTICLE_COUNT; ++i)
    {
        uint8_t led_x = p[i].x >> 4;
        uint8_t led_y = p[i].y >> 4;

        leds_add_particle(led_x, led_y);
    }

    // Turn back on for the brief moment to hopefully
    // prevent flicker even slightly.
    leds_set_led_on(LED_COL(leds_current_led), LED_ROW(leds_current_led));

    SREG = sreg;
}
#endif

