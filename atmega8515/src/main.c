#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/common.h>
#include <avr/sfr_defs.h>
#include <util/delay.h>

#include <stdbool.h>
#include <stdint.h>

#include "button.h"
#include "sensor.h"
#include "sim_timer.h"
#include "leds.h"
#include "sim.h"


bool is_mpu6050_connected = false;


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
    while (!sensor_connect())
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

    sensor_set_sleep(true);

    button_sleep_until_press();

    sensor_set_sleep(false);

    simulation_init();

    leds_update_brightness(); // Prevent infinite loop in LED interrupt.

    leds_start_timer(); sim_timer_start();
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

        // Check MPU6050 connection without timer interrupts disturbing I2C
        uint8_t old_sreg = SREG;
        cli();

        if (!is_mpu6050_connected)
        {
            // Full init on every attempt, and only then start running
            bool ok = sensor_connect();
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
            bool connected = sensor_is_alive();
            SREG = old_sreg;

            if (!connected)
            {
                leds_stop_timer(); sim_timer_stop();
                leds_set_all_off();
                is_mpu6050_connected = false;
                continue;
            }
        }

        sim_timer_wait_step();

        sensor_update_acceleration();
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

