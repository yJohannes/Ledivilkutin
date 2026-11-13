/**
 * @file leds.h
 * @author Johannes Ylinen
 * @brief LED display controls.
 * @date 2026-11-13
 */

#ifndef LEDS_H
#define LEDS_H

#include <avr/io.h>
#include <avr/pgmspace.h>

#include <stdint.h>


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


extern uint8_t leds_row_tbl[48];
extern uint8_t leds_col_tbl[48];


// #define LEDS_DOUBLE_BUFFER
#ifdef LEDS_DOUBLE_BUFFER
extern uint8_t *volatile brightness;

extern uint8_t buf_a[64];
extern uint8_t buf_b[64];
#else
/**
 * @brief 2 bits of brightness values per LED.
 *  i       & 0b00000011  // Lowest 2 bits
 * (i >> 2) & 0b00000011  // Bits 2–3
 * (i >> 4) & 0b00000011  // Bits 4–5
 * (i >> 6) & 0b00000011  // Highest 2 bits
 */
extern uint8_t brightness[64];
#endif // LEDS_DOUBLE_BUFFER

/**
 * @brief Initializes all LED pins to output and set off state.
 *
 * Signal   Inactive  Active
 * Rows     LOW       HIGH
 * Columns 	HIGH      LOW
 */
void leds_init_pins(void);

/**
 * @brief Initializes LED timer without starting it.
 */
void leds_init_timer(void);

void leds_start_timer(void);
void leds_stop_timer(void);

/**
 * @brief Sets all leds off.
 */
void leds_set_all_off(void);

/**
 * @brief Get the brightness of a LED at led_idx.
 *
 * @return uint8_t brightness level [0-3]
 */
static inline
uint8_t leds_get_brightness(uint8_t led_idx)
{
    return (brightness[led_idx >> 2] >> ((led_idx & 3) << 1)) & 3;
}

/**
 * @brief Increase the brightness level by one for LED at led_idx.
 */
void leds_add_brightness(uint8_t led_idx);

/**
 * @brief Adds a particle's contribution to the brightness map.
 */
void leds_add_particle(uint8_t x, uint8_t y);

/**
 * @brief Computes the LED brightness map based on particle positions.
 */
void leds_update_brightness(void);

/**
 * @brief Set a LED on. Clears previous LED on.
 *
 * ON: row HIGH, column LOW
 */
static inline
void leds_set_led_on(uint8_t x, uint8_t y)
{
    const uint8_t *r = &leds_row_tbl[y];
    const uint8_t *c = &leds_col_tbl[x];

    PORTA = (PORTA | r[0])  & (c[0]  | 0xF8);   // 0xF8: don't clear the row bits PA3-7
    PORTB =  PORTB          & (c[16] | 0xE0);   // 0xE0: PB5-7 untouched
    PORTC |= r[16];
    PORTD &= c[32];
    PORTE |= r[32];
}

/**
 * @brief Set a LED off. Clears previous LED on.
 *
 * OFF: row LOW, column HIGH
 */
static inline
void leds_set_led_off(uint8_t x, uint8_t y)
{
    const uint8_t *r = &leds_row_tbl[y];
    const uint8_t *c = &leds_col_tbl[x];

    PORTA = (PORTA & ~r[0])  | (~c[0]  & 0x07);
    PORTB =  PORTB           | (~c[16] & 0x1F);
    PORTC &= ~r[16];
    PORTD |= ~c[32];
    PORTE &= ~r[32];
}

#endif // LEDS_H
