#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#include <stdbool.h>

#include "i2c.h"
#include "mpu6050.h"
#include "sim.h"


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

    

    MCUCR
}

int main()
{
    // Wait for button press before starting I2C over PB5 & PB6
    while (!button_pressed)
    {

    }

    i2c_init();
    mpu6050_init();


    simulation_init();

    while (1)
    {
        simulation_step();
    }
}
