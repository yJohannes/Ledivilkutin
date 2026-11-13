/**
 * @file ledivilkutin.h
 * @author Johannes Ylinen
 * @date 2026-11-14
 */

#include "ledivilkutin.h"

#include <stdbool.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "button.h"
#include "leds.h"
#include "sensor.h"
#include "sim.h"
#include "sim_timer.h"


/*
 *   STANDBY --(button)--> CONNECTING --(sensor ok)--> RUNNING
 *      ^                      |                          |
 *      +------(button)--------+--------(button)----------+
 *
 *   RUNNING --(sensor lost)--> CONNECTING
 *
 * Every state polls the button, so it can always put the device to sleep.
 */
typedef enum {
    STATE_STANDBY,
    STATE_CONNECTING,
    STATE_RUNNING,
} AppState;

static AppState state = STATE_STANDBY;


/* ====================================================================
 * Helpers
 * ==================================================================== */

#define BLINK_SLICE_MS 10
#define BLINK_PHASE_SLICES 25  /* 25 * 10 ms = 250 ms */

/// Blink the "sensor missing" LED for 500 ms while polling the button
/// every 10 ms. Returns true if the button was pressed.
static bool blink_sensor_error(void)
{
    for (uint8_t i = 0; i < BLINK_PHASE_SLICES; i++)
    {
        leds_set_led_on(15, 15);
        _delay_ms(BLINK_SLICE_MS - 1);
        // button_poll_press() turns all LEDs off before sampling
        if (button_poll_press())
        {
            return true;
        }
    }

    for (uint8_t i = 0; i < BLINK_PHASE_SLICES; i++)
    {
        _delay_ms(BLINK_SLICE_MS);
        if (button_poll_press())
        {
            return true;
        }
    }

    return false;
}


static void stop_running_hardware(void)
{
    leds_stop_timer(); sim_timer_stop();
    leds_set_all_off();
}


static void start_running_hardware(void)
{
    simulation_init();
    leds_update_brightness(); // prevent infinite loop in the LED ISR

    leds_start_timer(); sim_timer_start();
}


/* ====================================================================
 * State handlers
 * ==================================================================== */

static void state_standby(void)
{
    stop_running_hardware();
    sensor_set_sleep(true); // sensor_connect() will wake it up

    button_sleep_until_press();

    state = STATE_CONNECTING;
}


static void state_connecting(void)
{
    while (!sensor_connect())
    {
        if (blink_sensor_error())
        {
            state = STATE_STANDBY;
            return;
        }
    }

    start_running_hardware();
    state = STATE_RUNNING;
}


static void state_running(void)
{
    if (button_poll_press())
    {
        state = STATE_STANDBY;
        return;
    }

    if (!sensor_is_alive())
    {
        stop_running_hardware();
        state = STATE_CONNECTING;
        return;
    }

    sim_timer_wait_step();
    sensor_update_acceleration();
    simulation_step();
    leds_update_brightness();
}


/* ====================================================================
 * Public API
 * ==================================================================== */

void ledivilkutin_init(void)
{
    leds_init_pins();

    button_init();

    leds_init_timer();
    sim_timer_init();

    // Boot indicator
    leds_set_led_on(0, 0);  _delay_ms(500);
    leds_set_led_off(0, 0); _delay_ms(500);

    // Wait for first button press.
    // Also ensures programmer can be disconnected.
    button_sleep_until_press();

    state = STATE_CONNECTING;
}


void ledivilkutin_run_once(void)
{
    switch (state)
    {
        case STATE_STANDBY:    state_standby();    break;
        case STATE_CONNECTING: state_connecting(); break;
        case STATE_RUNNING:    state_running();    break;
    }
}
