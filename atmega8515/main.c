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

    }

    init_interrupt();
    i2c_init();
    mpu6050_init();


    simulation_init();

    while (1)
    {
        simulation_step();
    }
}
