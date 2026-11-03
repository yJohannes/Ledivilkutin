#include <assert.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>

#include <stdbool.h>
#include <stdint.h>


// typedef struct {
//     uint8_t row[48];   // [0..15] PORTA  [16..31] PORTC  [32..47] PORTE   active = 1
//     uint8_t col[48];   // [0..15] PORTA  [16..31] PORTB  [32..47] PORTD   active = 0
// } tbl_t;

// static tbl_t T = {     // non-const -> SRAM (.data), 96 bytes, no alignment needed
//     .row = {
//         0x08,0x10,0x20,0x40,0x80,0,0,0,0,0,0,0,0,0,0,0,                  // PORTA
//         0,0,0,0,0,0,0,0,0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01,         // PORTC
//         0,0,0,0,0,0x01,0x02,0x04,0,0,0,0,0,0,0,0 },                      // PORTE
//     .col = {
//         0x03,0x07,0x05,0x07,0x06,0x07,0x07,0x07,
//         0x07,0x07,0x07,0x07,0x07,0x07,0x07,0x07,                          // PORTA
//         0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1E,0x1F,
//         0x1D,0x1F,0x1B,0x1F,0x17,0x1F,0x0F,0x1F,                          // PORTB (bits 0-4)
//         0xFF,0x7F,0xFF,0xBF,0xFF,0xDF,0xFF,0xEF,
//         0xFF,0xF7,0xFF,0xFB,0xFF,0xFD,0xFF,0xFE }                         // PORTD
// };
#include "i2c.h"
#include "mpu6050.h"
#include "sim.h"
#include "leds.h"
#include "sim_config.h"


#define DEBOUNCE_DELAY_MS 25

// https://ww1.microchip.com/downloads/en/DeviceDoc/doc2512.pdf#G1.1189378
// p. 93 Clock Select bits
#define OCR1A_PS1024_20_HZ 389 // (8 × 10^(6))/(1024 × 20) − 1 = 389.625
#define OCR1A_PS1024_19_HZ 410 // 8 000 000 / (1 024 × 19) − 1 = 410.184


volatile bool button_pressed = false;
volatile bool simulate_step = false;

volatile uint8_t current_led = 0;

bool running = false;



// (INT0) PD2 / External Interrupt Request 0
ISR(INT0_vect)
{
    // Disable INT0 to prevent additional interrupts during debouncing
    GICR &= ~(1 << INT0);

    button_pressed = true;
}


void init_button_interrupt(void)
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
    GIFR = (1 << INTF0);

    // Enable INT0
    // p. 78
    GICR |= (1 << INT0);
}


void wait_for_button(void)
{
    // Disable global interrupts to prevent an ISR from executing
    // while we check the button flag and prepare to sleep.
    cli();

    set_sleep_mode(SLEEP_MODE_PWR_DOWN);

    while (!button_pressed)
    {
        // Enable the sleep mechanism (does not sleep yet).
        sleep_enable();

        // Re-enable interrupts.
        // AVR guarantees that the instruction immediately following
        // sei() executes before any pending interrupt is serviced.
        sei();

        // Enter the selected sleep mode (Power-down).
        // If INT0 is pending, the MCU enters sleep and immediately
        // wakes up to execute the interrupt handler.
        sleep_cpu();

        // Execution resumes here after waking up.
        // Disable the sleep mechanism.
        sleep_disable();

        // Disable interrupts again before checking button_pressed.
        cli();
    }

    // Button was pressed. Re-enable interrupts before returning.
    sei();
}


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
    GIFR = (1 << INTF0);

    // Re-enable INT0
    GICR |= (1 << INT0);

    sei();

    return valid_press;
}


// 0 is a dummy value.
static const uint8_t brightness_ocr[] = {
    0, 64, 99, 199
};

/// @brief Interrupt to update the LEDs.
ISR(TIMER0_COMP_vect)
{
    uint8_t led = current_led; // volatile would make compiler have to reread.
    uint8_t row = led >> 4;
    uint8_t col = led & 0x0F;

    // Signal   Inactive  Active
    // Rows     LOW       HIGH
    // Columns 	HIGH      LOW

    // Set prev row LOW
    *ROW_PINS[row].port &= ~ROW_PINS[row].mask;

    // Set prev col HIGH
    *COL_PINS[col].port |= COL_PINS[col].mask;


    uint8_t brightness;

    // do
    // {
    //     brightness = get_brightness(++current_led);
    // }
    // while (brightness == 0);

    // TODO: ensure this doesn't take too long
    do
    {
        // led wraps around perfectly since LED count is 256.
        brightness = get_brightness(++led);
    }
    while (brightness == 0);

    row = led >> 4;
    col = led & 0x0F;

    // Set new row HIGH
    *ROW_PINS[row].port |= ROW_PINS[row].mask;

    // Set new col LOW
    *COL_PINS[col].port &= ~COL_PINS[col].mask;

    // Should not cause flicker or weird stuff since
    // There's no way there's enough time for the timer to
    // pass over the OCR0 value causing a wait for overflow + count.
    OCR0 = brightness_ocr[brightness];
}


void init_led_timer(void)
{
    TCCR0 = 0;      // Stop timer while configuring it
    TCNT0 = 0;      // Reset counter

    OCR0 = 48;

    // https://ww1.microchip.com/downloads/en/DeviceDoc/doc2512.pdf#G1.1189378
    TCCR0 = (1 << WGM01)        // CTC mode
          | (1 << CS01);        // Prescaler: 8

    TIMSK |= (1 << OCIE0);      // Enable Timer0 compare-match interrupt
}


ISR(TIMER1_COMPA_vect)
{
    simulate_step = true;
}

void init_sim_timer(void)
{

    // Stop Timer1 while configuring
    TCCR1B = 0;
    TCCR1A = 0;

    // CTC mode (TOP = OCR1A)
    TCCR1B |= (1 << WGM12);

    // Compare value
    OCR1A = OCR1A_PS1024_19_HZ;

    // Enable Timer1 Compare Match A interrupt
    TIMSK |= (1 << OCIE1A);

    // Prescaler = 1024, start timer
    TCCR1B |= (1 << CS12) | (1 << CS10);
}


int main()
{
    init_button_interrupt();

    // Prolly be redundant with the wait_for_button
    sei();

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

    cli();
    simulation_init();
    init_sim_timer();

    // Set to prevent infinite loop in LED interrupt.
    update_brightness();
    init_led_pins();
    init_led_timer();
    sei();

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

            // Idle while waiting
            while (!simulate_step)
            {
                set_sleep_mode(SLEEP_MODE_IDLE);
                sleep_mode();
            }

            simulate_step = false;
            simulation_step();
            update_brightness();
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
