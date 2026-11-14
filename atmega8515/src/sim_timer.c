/**
 * @file sim_timer.h
 * @author Johannes Ylinen
 * @date 2026-11-13
 */

#include "sim_timer.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#include <stdbool.h>
#include <stdint.h>

#include "sim_config.h"


/// @see https://ww1.microchip.com/downloads/en/DeviceDoc/doc2512.pdf#G1.1189378
/// p. 93 Clock Select bits
#define OCR1A_PS1024_20_HZ 389 // (8 × 10^(6))/(1024 × 20) − 1 = 389.625
#define OCR1A_PS1024_19_HZ 410 // 8 000 000 / (1 024 × 19) − 1 = 410.184

#define TIMER1_PRESCALER_BITS ((1 << CS12) | (1 << CS10)) // Prescaler 1024
#define TIMER1_CS_MASK ((1 << CS12) | (1 << CS11) | (1 << CS10))

/// @brief Pending simulation steps
static volatile uint8_t pending_steps = 0;


/* ====================================================================
 * Simulation interrupt
 * ==================================================================*/

ISR(TIMER1_COMPA_vect)
{
    ++pending_steps;
}


/* ====================================================================
 * Public API
 * ==================================================================*/

void sim_timer_init(void)
{
    TCCR1B = 0;     // Stop Timer1 while configuring
    TCCR1A = 0;
    TCNT1 = 0;      // Reset counter

#if (SIM_FPS == 19)
    OCR1A = OCR1A_PS1024_19_HZ;
#else
    OCR1A = OCR1A_PS1024_20_HZ;
#endif

    TCCR1B = (1 << WGM12);      // CTC mode, timer stopped
    TIMSK &= ~(1 << OCIE1A);    // Disable compare interrupt
}


void sim_timer_start(void)
{
    TCNT1 = 0;              // Reset counter
    TIFR = (1 << OCF1A);    // Clear pending compare flag
    TIMSK |= (1 << OCIE1A); // Enable Timer1 compare-match A interrupt.
    TCCR1B = (TCCR1B & ~TIMER1_CS_MASK) | TIMER1_PRESCALER_BITS;
}


void sim_timer_stop(void)
{
    TCCR1B &= ~TIMER1_CS_MASK;    // Stop timer clock
    TIMSK &= ~(1 << OCIE1A);      // Disable compare interrupt

    pending_steps = 0; // Discard any pending simulation step
}


void sim_timer_wait_step()
{
    for (;;)
    {
        cli();

        if (pending_steps > 0)
        {
            --pending_steps;
            sei();
            return;  // A step is ready. Start immediately.
        }

        set_sleep_mode(SLEEP_MODE_IDLE);
        sleep_enable();

        sei();
        sleep_cpu();
        sleep_disable();
    }
}
