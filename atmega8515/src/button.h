/**
 * @file button.h
 * @author Johannes Ylinen
 * @brief Button press handling and power-down sleep functionality
 *        while managing the shared LED matrix pin.
 * @date 2026-11-13
 *
 * The on/sleep button shares pin PD2 with an LED matrix column, so the
 * button can only be read while all LEDs are off and PD2 is an input.
 * This module hides that: callers never touch PD2 directly.
 */

#ifndef BUTTON_H
#define BUTTON_H

#include <stdbool.h>


/// @brief Configure INT0 (PD2) for low-level trigger. Call once at startup.
void button_init(void);

/// @brief Poll the button. Safe to call while the LED timer is running.
bool button_poll_press(void);

/**
 * @brief Turn all LEDs off and enter power-down until the button is pressed.
 * Returns after the press has been debounced and released.
 */
void button_sleep_until_press(void);


#endif // BUTTON_H
