#include "button.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/common.h>
#include <util/delay.h>

#include <stdbool.h>
#include <stdint.h>

#include "leds.h"


#define BUTTON_DEBOUNCE_MS 50
#define BUTTON_PIN PD2


static volatile bool button_pressed = false;


static bool pd2_is_low(void)
{
    return !(PIND & (1 << BUTTON_PIN));
}


/**
 * @brief
 *
 * @details Calls leds_set_all_off() to keep current safe.
 */
static void pd2_set_button_mode(void)
{
    leds_set_all_off();

    DDRD &= ~(1 << BUTTON_PIN); // Configure as input
    PORTD |= (1 << BUTTON_PIN); // Enable internal pull-up
}


static void pd2_set_led_mode(void)
{
    PORTD |= (1 << BUTTON_PIN); // Set output latch HIGH (LED inactive)
    DDRD |= (1 << BUTTON_PIN); // Configure as output
}


static void enable_int0(void)
{
    uint8_t old_sreg = SREG;
    cli();

    // First make sure all LED rows are inactive.
    // This prevents current flowing through the matrix
    // while PD2 is temporarily configured as an input.
    leds_set_all_off();

    pd2_set_button_mode();

    // Reset the event flag and clear any pending interrupt.
    button_pressed = false;
    GIFR = (1 << INTF0);

    // Handle a button that is already pressed.
    if (!(PIND & (1 << BUTTON_PIN)))
    {
        button_pressed = true;
    }

    // Enable INT0.
    GICR |= (1 << INT0);

    SREG = old_sreg;
}


static void disable_int0(void)
{
    // Disable INT0.
    GICR &= ~(1 << INT0);

    // Clear any pending interrupt flag.
    GIFR = (1 << INTF0);

    // Restore PD2 as an LED output, inactive HIGH.
    pd2_set_led_mode();
}


/**
 * @brief Requires PD2 in button mode.
 */
static void wait_for_release(void)
{
    for (;;)
    {
        while (pd2_is_low())
        {
        }

        _delay_ms(BUTTON_DEBOUNCE_MS);

        if (!pd2_is_low())
        {
            break;
        }
    }
}

static void sleep_until_int0(void)
{
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);

    cli();

    while (!button_pressed)
    {
        sleep_enable();

        sei();
        sleep_cpu();

        sleep_disable();
        cli();
    }

    sei();
}


/* ====================================================================
 * Button interrupt
 * ==================================================================*/

/**
 * @brief (INT0) PD2 / External Interrupt Request 0
 *
 * @warning ONLY TO BE USED WHEN WAKING FROM POWER-DOWN.
 */
ISR(INT0_vect)
{
    // Disable INT0 to prevent additional interrupts during debouncing
    GICR &= ~(1 << INT0);

    button_pressed = true;
}


/* ====================================================================
 * Public API
 * ==================================================================*/

void button_init(void)
{
    GICR &= ~(1 << INT0); // Disable INT0 while configuring it.

    // INT0: Trigger on low-level (required to wake from power-down).
    MCUCR &= ~(1 << ISC01);
    MCUCR &= ~(1 << ISC00);

    GIFR = (1 << INTF0); // Clear pending interrupt flags.
}


bool button_poll_press(void)
{
    uint8_t old_sreg = SREG;
    cli(); // Prevent LED ISR from driving PD2 meanwhile

    leds_set_all_off(); // Ensure all LED rows are inactive.

    // Switch PD2 to input with pull-up.
    pd2_set_button_mode();
    _delay_us(10);

    // Button pressed = LOW
    bool pressed = pd2_is_low();

    if (pressed)
    {
        _delay_ms(BUTTON_DEBOUNCE_MS);
        wait_for_release();
    }

    pd2_set_led_mode();

    SREG = old_sreg;
    return pressed;
}


void button_sleep_until_press(void)
{
    enable_int0();
    sleep_until_int0();

    _delay_ms(BUTTON_DEBOUNCE_MS);
    wait_for_release();

    disable_int0();
}
