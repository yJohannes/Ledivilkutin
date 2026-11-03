// #include <avr/io.h>
// #include <avr/interrupt.h>
// #include <avr/sleep.h>
// #include <util/delay.h>

// #include <stdbool.h>

// #include "sim.h"
// #include "sim_config.h"


// #define DEBOUNCE_DELAY_MS 25

// // https://ww1.microchip.com/downloads/en/DeviceDoc/doc2512.pdf#G1.1189378
// // p. 93 Clock Select bits
// #define LED_TIMER_PRESCALER 64  // 8, 64, 256, 1024
// #define OCR1A_PS1024_20_HZ 389 // // ((8 × 10^(6))/(1024 × 20)) − 1 = 389.625

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


// volatile bool simulate_step = false;
// bool running = true;

// volatile uint32_t frame_count = 0;
// static void wait_for_simulation_step(void)
// {
// 	while (1)
// 	{
// 		cli();

// 		if (simulate_step)
// 		{
// 			simulate_step = false;
// 			sei();
// 			return;
// 		}

// 		set_sleep_mode(SLEEP_MODE_IDLE);
// 		sleep_enable();

// 		sei();
// 		sleep_cpu();

// 		sleep_disable();
// 	}
// }

// ISR(TIMER1_COMPA_vect)
// {
//     simulate_step = true;
// }

// void init_sim_timer(void)
// {

//     // Stop Timer1 while configuring
//     TCCR1B = 0;
//     TCCR1A = 0;

//     // CTC mode (TOP = OCR1A)
//     TCCR1B |= (1 << WGM12);

//     // Compare value
//     OCR1A = OCR1A_PS1024_20_HZ;

//     // Enable Timer1 Compare Match A interrupt
//     TIMSK |= (1 << OCIE1A);

//     // Prescaler = 1024, start timer
//     TCCR1B |= TIMER1_PRESCALER_1024;
// }


// int main()
// {
//     init_sim_timer();

//     // Might be redundant with the wait_for_button
//     sei();

//     // Initialize LED interrupts.

//     simulation_init();

//     while (1)
//     {
//         if (running)
//         {
//             // Choose acceleration
//             // This is moved out of simulation_step for performance.
//             if (--sim_accel_timer == 0)
//             {
//                 sim_accel_timer = SIM_ACCELERATION_INTERVAL;

//                 // Divide by 4 to map acceleration to [-32, 31]
//                 // https://ww1.microchip.com/downloads/en/DeviceDoc/AVR-Instruction-Set-Manual-DS40002198A.pdf#_OPENTOPIC_TOC_PROCESSING_d138e17156
//                 a.x >>= 2;
//                 a.y >>= 2;
//             }
//             else
//             {
//                 a.x = 0u;
//                 a.y = 0u;
//             }

// 			wait_for_simulation_step();
// 			frame_count++;
//             simulation_step();

// 			frame_count++;
//         }
//     }
// }



#include <assert.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>

#include <stdbool.h>
#include <stdint.h>

#include "sim.h"
#include "leds.h"
#include "sim_config.h"


#define DEBOUNCE_DELAY_MS 25

// https://ww1.microchip.com/downloads/en/DeviceDoc/doc2512.pdf#G1.1189378
// p. 93 Clock Select bits
#define LED_TIMER_PRESCALER 64  // 8, 64, 256, 1024
#define OCR1A_PS1024_20_HZ 389 // (8 × 10^(6))/(1024 × 20) − 1 = 389.625
#define OCR1A_PS1024_19_HZ 410 // 8 000 000 / (1 024 × 19) − 1 = 410.184

// Timer 0
#define TIMER0_PRESCALER_1    (1 << CS00)
#define TIMER0_PRESCALER_8    (1 << CS01)
#define TIMER0_PRESCALER_64   ((1 << CS01) | (1 << CS00))
#define TIMER0_PRESCALER_256  (1 << CS02)
#define TIMER0_PRESCALER_1024 ((1 << CS02) | (1 << CS00))

// Timer 1
#define TIMER1_PRESCALER_1    (1 << CS10)
#define TIMER1_PRESCALER_8    (1 << CS11)
#define TIMER1_PRESCALER_64   ((1 << CS11) | (1 << CS10))
#define TIMER1_PRESCALER_256  (1 << CS12)
#define TIMER1_PRESCALER_1024 ((1 << CS12) | (1 << CS10))


volatile bool simulate_step = false;

volatile uint8_t current_led = 0;

bool running = false;



void init_led_timer(void)
{
    TCCR0 = 0;      // Stop timer while configuring it
    TCNT0 = 0;      // Reset counter

    OCR0 = 192;

    TCCR0 = (1 << WGM01)        // CTC mode
          | (1 << CS02)
          | (1 << CS00);        // Prescaler: 1024

    TIMSK |= (1 << OCIE0);      // Enable Timer0 compare-match interrupt
}

// Quadratic progression
static const uint8_t brightness_ocr[] = {
    0, 48, 96, 192
};

/// @brief Interrupt to update the LEDs.
ISR(TIMER0_COMP_vect)
{
    uint8_t row = current_led >> 4;
    uint8_t col = current_led & 0x0F;

    // Signal   Inactive  Active
    // Rows     LOW       HIGH
    // Columns 	HIGH      LOW

    // Set prev row LOW
    *ROW_PINS[row].port &= ~ROW_PINS[row].mask;

    // Set prev col HIGH
    *COL_PINS[col].port |= COL_PINS[col].mask;


    uint8_t brightness;

    do
    {
        brightness = get_brightness(++current_led);
    }
    while (brightness == 0);

    row = current_led >> 4;
    col = current_led & 0x0F;

    // Set new row HIGH
    *ROW_PINS[row].port |= ROW_PINS[row].mask;

    // Set new col LOW
    *COL_PINS[col].port &= ~COL_PINS[col].mask;

    OCR0 = brightness_ocr[brightness];
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
    TCCR1B |= TIMER1_PRESCALER_1024;
}


int main()
{
    // Prolly be redundant with the wait_for_button
    sei();


    simulation_init();
    init_sim_timer();

    cli();
    // Set to prevent infinite loop in LED interrupt.
    update_brightness();
    init_led_pins();
    init_led_timer();
    sei();

    running = true;

    while (1)
    {
    asm volatile("nop"); // BREAKPOINT HERE

        if (running)
        {
            // Choose acceleration
            // This is moved out of simulation_step for performance.
            if (--sim_accel_timer == 0)
            {
                sim_accel_timer = SIM_ACCELERATION_INTERVAL;

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

		    asm volatile("nop"); // BREAKPOINT HERE
			simulate_step = false;
            simulation_step();
		    asm volatile("nop"); // BREAKPOINT HERE
            
			update_brightness();
		    asm volatile("nop"); // BREAKPOINT HERE
			
        }
		    asm volatile("nop"); // BREAKPOINT HERE		
    }
}
