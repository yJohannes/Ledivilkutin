#include <assert.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/common.h>


#include <util/delay.h>

#include <stdbool.h>
#include <stdint.h>

#include "i2c.h"
#include "mpu6050.h"
#include "sim.h"
#include "leds.h"
#include "sim_config.h"


volatile bool button_pressed = false;

/// @brief Flag to simulate a step set by timer @ 19 Hz
// volatile bool simulate_step = false;
volatile uint8_t pending_steps = 0;

volatile uint8_t current_led = 0;

bool running = false;


// ============================================================================
// BUTTON INTERRUPT
// ============================================================================

#define BUTTON_PIN PD2


// (INT0) PD2 / External Interrupt Request 0
// ONLY TO BE USED WHEN POWERED DOWN
ISR(INT0_vect)
{
    // Disable INT0 to prevent additional interrupts during debouncing
    GICR &= ~(1 << INT0);

    button_pressed = true;
}

void init_button_interrupt(void)
{
    // Disable INT0 while configuring it.
    GICR &= ~(1 << INT0);

    // Configure INT0 for falling-edge triggering.
    // ISC01 = 1, ISC00 = 0 is Falling edge
    MCUCR |= (1 << ISC01);
    MCUCR &= ~(1 << ISC00);

    // Clear any pending interrupt flag.
    GIFR = (1 << INTF0);
}


/// @warning set_led_rows_low() MUST BE CALLED FIRST!!!!!!!!
void pd2_set_button_mode(void)
{
    // Configure as input
    DDRD &= ~(1 << BUTTON_PIN);

    // Enable internal pull-up
    PORTD |= (1 << BUTTON_PIN);
}


void pd2_set_led_mode(void)
{
    // Set output latch HIGH (LED inactive)
    PORTD |= (1 << BUTTON_PIN);

    // Configure as output
    DDRD |= (1 << BUTTON_PIN);
}


void enable_button_interrupt(void)
{
    uint8_t old_sreg = SREG;
    cli();

    // First make sure all LED rows are inactive.
    // This prevents current flowing through the matrix
    // while PD2 is temporarily configured as an input.
    set_led_rows_low();

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


void disable_button_interrupt(void)
{
    // Disable INT0.
    GICR &= ~(1 << INT0);

    // Clear any pending interrupt flag.
    GIFR = (1 << INTF0);

    // Restore PD2 as an LED output, inactive HIGH.
    pd2_set_led_mode();
}


static void wait_for_button_release(void)
{
    while (1)
    {
        // Wait until the button is released.
        while (!(PIND & (1 << BUTTON_PIN)))
        {
        }

        // Debounce the release.
        _delay_ms(50);

        if (PIND & (1 << BUTTON_PIN))
        {
            break;
        }
    }
}


void wait_for_button_interrupt(void)
{
    // Disable global interrupts to prevent an ISR from executing
    // while we check the button flag and prepare to sleep.
    cli();

    set_sleep_mode(SLEEP_MODE_PWR_DOWN);

    while (!button_pressed)
    {
        sleep_enable(); // Enable the sleep mechanism (does not sleep yet).

        // Re-enable interrupts.
        // AVR guarantees that the instruction immediately following
        // sei() executes before any pending interrupt is serviced.
        sei();

        // Enter the selected sleep mode (Power-down).
        // If INT0 is pending, the MCU enters sleep and immediately
        // wakes up to execute the interrupt handler.
        sleep_cpu();
        sleep_disable(); // Execution resumes here after waking up.

        // Disable interrupts again before checking button_pressed.
        cli();
    }

    // Button was pressed. Re-enable interrupts before returning.
    sei();
}

// ============================================================================
// LED INTERRUPT
// ============================================================================


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

    // TODO: ensure this doesn't take too long
    do
    {
        // led wraps around perfectly since LED count is 256.
        brightness = get_brightness(++led);
    }
    while (brightness == 0);

    current_led = led; // Remember to update

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


// https://ww1.microchip.com/downloads/en/DeviceDoc/doc2512.pdf#G1.1189378
#define TIMER0_PRESCALER_BITS (1 << CS01) // Prescaler: 8
#define TIMER0_CS_MASK ((1 << CS02) | (1 << CS01) | (1 << CS00))

void init_led_timer(void)
{
    TCCR0 = 0;      // Stop timer while configuring it
    TCNT0 = 0;      // Reset counter
    OCR0 = 199;
    TCCR0 = (1 << WGM01);   // CTC mode, timer stopped

    TIMSK &= ~(1 << OCIE0);      // Disable compare-match interrupt
}


static void start_led_timer(void)
{
    TCNT0 = 0;             // Reset counter
    TIFR = (1 << OCF0);    // Clear pending compare flag

    TIMSK |= (1 << OCIE0);        // Enable compare interrupt

    // Clear scaler bits and add new
    TCCR0 = (TCCR0 & ~TIMER0_CS_MASK) | TIMER0_PRESCALER_BITS;
}


static void stop_led_timer(void)
{
    TIMSK &= ~(1 << OCIE0);     // Disable compare-match interrupt
    TCCR0 &= ~TIMER0_CS_MASK;   // Stop timer clock
}


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


// ============================================================================
// MAIN
// ============================================================================

static void ledivilkutin_initialize(void)
{
    // pd2_set_button_mode();

    init_button_interrupt();
    enable_button_interrupt();

    sei(); // Prolly be redundant

    // Wait for button press before starting I2C over pins PB5 & PB6
    wait_for_button_interrupt();

    // Debounce
    _delay_ms(50);

    wait_for_button_release();

    disable_button_interrupt();

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
    start_sim_timer();

    // Set to prevent infinite loop in LED interrupt.
    update_brightness();

    init_led_pins();
    init_led_timer();
    start_led_timer();
    sei();

    running = true; // Remember to start simulation
}

static void ledivilkutin_sleep(void)
{
    stop_led_timer();
    stop_sim_timer();

    // Reset LED pins to inactive states.
    init_led_pins();

    // Failure is acceptable: the sensor may already be disconnected.
    mpu6050_set_sleep(true);

    // Ensure I2C pins are released for possible programming.
    i2c_release_pins();

    enable_button_interrupt();

    // Sleep until interrupt
    wait_for_button_interrupt();

    // Debounce
    _delay_ms(50);
    wait_for_button_release();

    disable_button_interrupt();

    // If it is disconnected, the next connectivity check handles recovery.
    mpu6050_set_sleep(false);

    simulation_init();

    // Set to prevent infinite loop in LED interrupt.
    update_brightness();

    start_led_timer();
    start_sim_timer();
}

static void ledivilkutin_update_acceleration(void)
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
            a.x = 0;
            a.y = 0;
        }



        // // Divide by 4 to map acceleration to [-32, 31]
        // Divide by 8 to map acceleration to [-16, 15]
        // https://ww1.microchip.com/downloads/en/DeviceDoc/AVR-Instruction-Set-Manual-DS40002198A.pdf#_OPENTOPIC_TOC_PROCESSING_d138e17156
        a.x >>= 3;
        a.y >>= 3;
    }
    else
    {
        a.x = 0;
        a.y = 0;
    }
}


static void ledivilkutin_check_button(void)
{
    uint8_t old_sreg = SREG;
    cli();  // Stop interruptions during this check

    // Make sure all LED rows are inactive.
    set_led_rows_low();

    // Switch PD2 to input with pull-up.
    pd2_set_button_mode();

    _delay_us(10);

    // Button pressed = LOW
    running = (PIND & (1 << BUTTON_PIN));

    if (!running)
    {
        _delay_ms(50);
        wait_for_button_release();
    }

    // Restore PD2 as an LED output.
    // PD2 is a column pin so it's inactive state is HIGH.
    pd2_set_led_mode();


    // Restore the previous CPU status and interrupt state.
    SREG = old_sreg;
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
        ledivilkutin_check_button();

        if (!running)
        {
            ledivilkutin_sleep();
            continue;
        }

        // Keep trying to initialize MPU6050 again
        if (!mpu6050_is_connected())
        {
            mpu6050_init();
            _delay_ms(50);
            // Maybe blink a led here
            continue; // Keep trying to connect
        }

        // Wait for simulation tick
        // while (!simulate_step)
        // {
        //     set_sleep_mode(SLEEP_MODE_IDLE);
        //     sleep_mode();

        //     // If button is pressed just do the next simulation tick to not
        //     // waste cycles here performing checks after every interrupt.
        // }

        wait_for_simulation_step();

        ledivilkutin_update_acceleration();
        simulation_step();
        update_brightness();

        // simulate_step = false;
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
