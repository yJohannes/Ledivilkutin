#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#include <stdbool.h>

#include "i2c.h"
#include "mpu6050.h"
#include "sim.h"
// Use conventional screen coordinates (0, 0) = top-left corner
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


volatile bool button_pressed = false;
bool running = false;


// (INT0) PD2 / External Interrupt Request 0
ISR(INT0_vect)
{
    // Disable INT0 to prevent additional interrupts during debouncing
    GICR &= ~(1 << INT0);

    button_pressed = true;
}

void button_init(void)
{
    // Enable INT0
    EIMSK;

}


void init_interrupt(void)
{
    // DDRD is the Data Direction Register D
    // DDRD bit = 0 → Input
    // DDRD bit = 1 → Output

    // Set PD2 as input
    DDRD &= ~(1 << PD2);

    // Enable internal pull-up resistor
    PORTD |= (1 << PD2);


    // Configure INT0 for falling edge
    // ISC01 = 1, ISC00 = 0 is Falling edge
    MCUCR |= (1 << ISC01);
    MCUCR &= ~(1 << ISC00);

    // Clear any pending INT0 interrupt flag
    GIFR |= (1 << INTF0);

    // Enable INT0
    // p. 78
    GICR |= (1 << INT0);

    // Enable global interrupts !!!
    sei();
}

int main()
{
    // Wait for button press before starting I2C over PB5 & PB6
    while (!button_pressed)
    {

/// @returns true if the button remained pressed after the debounce interval;
///          false if the interrupt was caused by contact bounce.
bool debounce_button(void)
{
    // Wait for mechanical contact bouncing to settle
    _delay_ms(DEBOUNCE_DELAY_MS);

    // Check whether the button is still pressed (PD2 LOW)
    bool valid_press = !(PIND & (1 << PD2));

    if (valid_press)
    {
        // Wait until button is released
        while (!(PIND & (1 << PD2)))
        {
        }

        // Wait for release bouncing to settle
        _delay_ms(DEBOUNCE_DELAY_MS);
    }

    // Clear button flag and pending interrupt
    cli();

    button_pressed = false;
    GIFR |= (1 << INTF0);

    // Re-enable INT0
    GICR |= (1 << INT0);

    sei();

    return valid_press;
}

int main()
{
    init_interrupt();

    set_sleep_mode(SLEEP_MODE_PWR_DOWN);

    // Wait for button press before starting I2C over pins PB5 & PB6
    do
    {
        wait_for_button();
    }
    while (!debounce_button());

    // Init I2C only after this to allow removing the programming device.
    i2c_init();
    while (!mpu6050_init())
    {
        // Blink something to indicate
        _delay_ms(250);
    }


    simulation_init();

    while (1)
    {
        if (button_pressed)
        {
            if (debounce_button())
            {
                // Toggle running
                running = !running;
            }
        }

        if (!mpu6050_is_connected())
        {
            running = false;

            while (!mpu6050_init())
            {
                _delay_ms(250);
            }
        }

        if (running)
        {
            // Choose acceleration
            // This is moved out of simulation_step for performance.
            if (--sim_accel_timer == 0)
            {
                sim_accel_timer = SIM_ACCELERATION_INTERVAL;

                // On failure set acceleration to zero and if this was
                // caused by MPU6050 disconnecting the code above will
                // handle it.
                if (!mpu6050_read_accel_xy_i8(&a.x, &a.y))
                {
                    a.x = 0u;
                    a.y = 0u;
                }

                // Divide by 4 to map acceleration to [-32, 31]
                // https://ww1.microchip.com/downloads/en/DeviceDoc/AVR-Instruction-Set-Manual-DS40002198A.pdf#_OPENTOPIC_TOC_PROCESSING_d138e17156
                a.x >>= 2;
                a.y >>= 2;
            }
            else
            {
                a.x = 0u;
                a.y = 0u;
            }

            simulation_step();
        }
        else
        {
            // Failure is acceptable: the sensor may already be disconnected.
            mpu6050_set_sleep(true);

            // Ensure I2C pins are released for possible programming.
            i2c_release_pins();

            // Sleep until interrupt
            wait_for_button();

            // If it is disconnected, the next connectivity check handles recovery.
            mpu6050_set_sleep(false);
        }
    }
}
