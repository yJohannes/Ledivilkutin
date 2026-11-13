#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/common.h>
#include <avr/sfr_defs.h>
#include <util/delay.h>

#include <stdbool.h>
#include <stdint.h>

#include "i2c.h"
#include "mpu6050.h"
#include "sim.h"
#include "leds.h"
#include "button.h"
#include "sim_config.h"


// #define DISABLE_I2C

bool is_mpu6050_connected = false;

/// @brief Flag to simulate a step set by timer @ 19 Hz
// volatile bool simulate_step = false;
volatile uint8_t pending_steps = 0;


// ============================================================================
// SIMULATION INTERRUPT
// ============================================================================


ISR(TIMER1_COMPA_vect)
{
    // simulate_step = true;
    ++pending_steps;
}


// https://ww1.microchip.com/downloads/en/DeviceDoc/doc2512.pdf#G1.1189378
// p. 93 Clock Select bits
#define OCR1A_PS1024_20_HZ 389 // (8 × 10^(6))/(1024 × 20) − 1 = 389.625
#define OCR1A_PS1024_19_HZ 410 // 8 000 000 / (1 024 × 19) − 1 = 410.184

#define TIMER1_PRESCALER_BITS ((1 << CS12) | (1 << CS10)) // Prescaler 1024
#define TIMER1_CS_MASK ((1 << CS12) | (1 << CS11) | (1 << CS10))

/// @brief Initialize simulation step timer @ 19 Hz
void init_sim_timer(void)
{

    // Stop Timer1 while configuring
    TCCR1B = 0;
    TCCR1A = 0;
    TCNT1 = 0;      // Reset counter

    // Compare value
    OCR1A = OCR1A_PS1024_19_HZ;

    TCCR1B = (1 << WGM12);      // CTC mode, timer stopped
    TIMSK &= ~(1 << OCIE1A);    // Disable compare interrupt
}


static void start_sim_timer(void)
{
    TCNT1 = 0;              // Reset counter
    TIFR = (1 << OCF1A);    // Clear pending compare flag

    // Enable Timer1 compare-match A interrupt.
    TIMSK |= (1 << OCIE1A);

    TCCR1B = (TCCR1B & ~TIMER1_CS_MASK) | TIMER1_PRESCALER_BITS;
}


static void stop_sim_timer(void)
{
    TCCR1B &= ~TIMER1_CS_MASK;    // Stop timer clock
    TIMSK &= ~(1 << OCIE1A);      // Disable compare interrupt

    // Discard any pending simulation step
    // simulate_step = false;
    pending_steps = 0u;
}


/* ====================================================================
 * Main
 * ==================================================================*/


static void ledivilkutin_initialize(void)
{
    leds_init_pins();

    // Blink to indicate startup
    leds_set_led_on(0, 0);  _delay_ms(500);
    leds_set_led_off(0, 0); _delay_ms(500);

    button_init();

    // Wait for button press before starting I2C over pins PB5 & PB6
    // to allow removing the programming device.
    button_sleep_until_press();

#ifndef DISABLE_I2C
    i2c_init();
    while (!mpu6050_init())
    {
        leds_set_led_on(15, 15);  _delay_ms(250);
        leds_set_led_off(15, 15); _delay_ms(250);

        if (button_poll_press())
        {
            button_sleep_until_press();
        }
    }

    is_mpu6050_connected = true;
#endif // DISABLE_I2C

    cli();
    simulation_init();
    sim_timer_init();
    sim_timer_start();

    // Set to prevent infinite loop in LED interrupt.
    leds_update_brightness();

    leds_init_timer();
    leds_start_timer();
    sei();
}

static void ledivilkutin_sleep(void)
{
    leds_stop_timer(); sim_timer_stop();

    // Reset LED pins to inactive states.
    leds_set_all_off();

#ifndef DISABLE_I2C
    // Failure is acceptable: the sensor may already be disconnected.
    mpu6050_set_sleep(true);

    // Ensure I2C pins are released for possible programming.
    i2c_release_pins();
#endif // DISABLE_I2C

    button_sleep_until_press();

#ifndef DISABLE_I2C
    // If disconnected, next connectivity check handles recovery.
    mpu6050_set_sleep(false);
#endif // DISABLE_I2C
    simulation_init();

    leds_update_brightness(); // Prevent infinite loop in LED interrupt.

    leds_start_timer(); sim_timer_start();
}

static void ledivilkutin_update_acceleration(void)
{
    // Choose acceleration
    // This is moved out of simulation_step for performance.
    if (--sim_accel_timer == 0)
    {
        sim_accel_timer = SIM_ACCELERATION_INTERVAL;

        {
            // Don't allow timers to disturb I2C
            uint8_t old_sreg = SREG;
            cli();

            // SWAP X- AND Y-AXIS
            if (!mpu6050_read_accel_xy_i8(&a.y, &a.x))
            {
                // On failure set acceleration to zero and if this was
                // caused by MPU6050 disconnecting the code above will
                // handle it.
                a.x = 0;
                a.y = 0;
            }

            // Change x-axis direction
            a.x = -a.x;

            SREG = old_sreg;
        }

        // // Divide by 4 to map acceleration to [-32, 31]
        // Divide by 8 to map acceleration to [-16, 15]
        // https://ww1.microchip.com/downloads/en/DeviceDoc/AVR-Instruction-Set-Manual-DS40002198A.pdf#_OPENTOPIC_TOC_PROCESSING_d138e17156
        a.x >>= 3;
        a.y >>= 3;

        a.x = -a.x;
        a.y = -a.y;
    }
    else
    {
        a.x = 0;
        a.y = 0;
    }
}


static void wait_for_simulation_step(void)
{
    while (1)
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


int main()
{
    ledivilkutin_initialize();

    while (1)
    {
        bool running = !button_poll_press();

        if (!running)
        {
            ledivilkutin_sleep();
            continue;
        }

#ifndef DISABLE_I2C
        // Check MPU6050 connection without timer interrupts disturbing I2C
        uint8_t old_sreg = SREG;
        cli();

        if (!is_mpu6050_connected)
        {
            // Full init on every attempt, and only then start running
            bool ok = mpu6050_init();
            SREG = old_sreg;

            if (!ok)
            {
                leds_set_all_off(); // Ensure no LED was left on

                leds_set_led_on(15, 15);  _delay_ms(250);
                leds_set_led_off(15, 15); _delay_ms(250);
                continue;
            }

            is_mpu6050_connected = true;
            simulation_init();
            leds_update_brightness();

            leds_start_timer(); sim_timer_start();
        }
        else
        {
            bool connected = mpu6050_is_connected();
            SREG = old_sreg;

            if (!connected)
            {
                leds_stop_timer(); sim_timer_stop();
                leds_set_all_off();
                is_mpu6050_connected = false;
                continue;
            }
        }
#endif // DISABLE_I2C
        wait_for_simulation_step();
#ifndef DISABLE_I2C
        ledivilkutin_update_acceleration();
#endif // DISABLE_I2C
        simulation_step();
        leds_update_brightness();
    }
}



// // Timer 0
// #define TIMER0_PRESCALER_1    (1 << CS00)
// #define TIMER0_PRESCALER_8    (1 << CS01)
// #define TIMER0_PRESCALER_64   ((1 << CS01) | (1 << CS00))
// #define TIMER0_PRESCALER_256  (1 << CS02)
// #define TIMER0_PRESCALER_1024 ((1 << CS02) | (1 << CS00))

// // Timer 1
// #define TIMER1_PRESCALER_1    (1 << CS10)
// #define TIMER1_PRESCALER_8    (1 << CS11)
// #define TIMER1_PRESCALER_64   ((1 << CS11) | (1 << CS10))
// #define TIMER1_PRESCALER_256  (1 << CS12)
// #define TIMER1_PRESCALER_1024 ((1 << CS12) | (1 << CS10))

