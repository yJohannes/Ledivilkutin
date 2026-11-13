#include "leds.h"

#include <avr/io.h>
#include <avr/sfr_defs.h>
#include <avr/interrupt.h>
#include <avr/common.h>

#include <stdint.h>
#include <string.h>

#include "sim.h"
#include "sim_config.h"


/**
 * @brief Row and column masks based on the physical
 * pin configurations.
 */
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

#define LED_ROW(led) ((led) >> 4)
#define LED_COL(led) ((led) & 0x0F)

/// @see https://ww1.microchip.com/downloads/en/DeviceDoc/doc2512.pdf#G1.1189378
#define TIMER0_PRESCALER_BITS (1 << CS01) // Prescaler: 8
#define TIMER0_CS_MASK ((1 << CS02) | (1 << CS01) | (1 << CS00))


#ifdef LEDS_DOUBLE_BUFFER
uint8_t buf_a[64];
uint8_t buf_b[64];
uint8_t *volatile brightness = buf_a;   // read by ISR
static uint8_t *back_buf  = buf_b;     // written by main
#else
uint8_t brightness[64];
#endif // LEDS_DOUBLE_BUFFER


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

// No const so they stay in SRAM
static uint8_t brightness_masks[4] = {
    0x03, 0x0C, 0x30, 0xC0
};

static uint8_t brightness_increments[4] = {
    0x01, 0x04, 0x10, 0x40
};

// 0 is a dummy value for correct indexing.
static uint8_t brightness_ocr0[] = {
    // 0, 64, 99, 199 // old values
    0, 49, 84, 184
};

static volatile uint8_t current_led = 0;


/* ====================================================================
 * LED interrupt
 * ==================================================================== */

ISR(TIMER0_COMP_vect)
{
    uint8_t led = current_led;
    uint8_t b;

    // Find next LED to turn on. Skip empty bytes = 4 LEDs, etc.
    for (;;) {
        led++;                              // wraps 255 -> 0
        uint8_t v = brightness[led >> 2];
        if (v == 0) { led |= 3; continue; } // whole group of 4 is off, skip it
        if (led & 2) v >>= 4;               // unsigned: no asr/ror loop
        if (led & 1) v >>= 2;
        b = v & 3;
        if (b) break;
    }
    current_led = led;

    uint8_t *z, *y;
    uint8_t t0, t1;

    asm volatile(
        // Z = &row_tbl[led >> 4]
        "mov  %A0, %4              \n\t"
        "swap %A0                  \n\t"
        "andi %A0, 0x0F            \n\t"
        "ldi  %B0, 0               \n\t"
        "subi %A0, lo8(-(%[rt]))   \n\t"
        "sbci %B0, hi8(-(%[rt]))   \n\t"
        // Y = &col_tbl[led & 15]
        "mov  %A1, %4              \n\t"
        "andi %A1, 0x0F            \n\t"
        "ldi  %B1, 0               \n\t"
        "subi %A1, lo8(-(%[ct]))   \n\t"
        "sbci %B1, hi8(-(%[ct]))   \n\t"
        // port writes
        "ld   %2, Z                \n\t"
        "ld   %3, Y                \n\t"
        "or   %2, %3               \n\t"
        "out  %[pa], %2            \n\t"
        "in   %2, %[pb]            \n\t"
        "andi %2, 0xE0             \n\t"   // keep PB7..PB5
        "ldd  %3, Y+16             \n\t"
        "or   %2, %3               \n\t"
        "out  %[pb], %2            \n\t"
        "ldd  %2, Z+16             \n\t"
        "out  %[pc], %2            \n\t"
        "ldd  %2, Y+32             \n\t"
        "out  %[pd], %2            \n\t"
        "ldd  %2, Z+32             \n\t"
        "out  %[pe], %2            \n\t"
        : "=&z"(z), "=&y"(y), "=&d"(t0), "=&d"(t1)
        : "r"(led),
          [rt] "i"(row_tbl), [ct] "i"(col_tbl),
          [pa] "I"(_SFR_IO_ADDR(PORTA)), [pb] "I"(_SFR_IO_ADDR(PORTB)),
          [pc] "I"(_SFR_IO_ADDR(PORTC)), [pd] "I"(_SFR_IO_ADDR(PORTD)),
          [pe] "I"(_SFR_IO_ADDR(PORTE))
        : "cc");

    OCR0 = brightness_ocr0[b];
}


/* ====================================================================
 * Public API
 * ==================================================================*/

void leds_init_pins(void)
{
    // Set to prevent infinite loop in LED interrupt.
    // 32+2 because it blinks in the center unnoticeably.
    brightness[32+2] = 1u;

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


void leds_init_timer(void)
{
    TCCR0 = 0;      // Stop timer while configuring it
    TCNT0 = 0;      // Reset counter
    OCR0 = 199;
    TCCR0 = (1 << WGM01);   // CTC mode, timer stopped

    TIMSK &= ~(1 << OCIE0);      // Disable compare-match interrupt
}


void leds_start_timer(void)
{
    TCNT0 = 0;             // Reset counter
    TIFR = (1 << OCF0);    // Clear pending compare flag

    TIMSK |= (1 << OCIE0);        // Enable compare interrupt

    // Clear scaler bits and add new
    TCCR0 = (TCCR0 & ~TIMER0_CS_MASK) | TIMER0_PRESCALER_BITS;
}


void leds_stop_timer(void)
{
    TIMSK &= ~(1 << OCIE0);     // Disable compare-match interrupt
    TCCR0 &= ~TIMER0_CS_MASK;   // Stop timer clock
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

    uint8_t mask = brightness_masks[pos];

    // Perform magic
    if ((*pt & mask) != mask)
        *pt += brightness_increments[pos];
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
    leds_set_led_off(LED_COL(current_led), LED_ROW(current_led));

    // Clear the framebuffer.
    memset(&brightness[0], 0, sizeof(brightness));

    for (uint8_t i = 0u; i < SIM_PARTICLE_COUNT; ++i)
    {
        // Map [0, 255] -> [0, 15]
        uint8_t led_x = p[i].x >> 4;
        uint8_t led_y = p[i].y >> 4;

        leds_add_particle(led_x, led_y);
    }

    // Turn back on for the brief moment to hopefully
    // prevent flicker even slightly.
    leds_set_led_on(LED_COL(current_led), LED_ROW(current_led));

    SREG = sreg;
}
#endif // LEDS_DOUBLE_BUFFER


#if ___________________
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
