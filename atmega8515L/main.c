#include <avr/io.h>

#include "i2c.h"
#include "mpu6050.h"
#include "simulation.h"


int main()
{
    i2c_init();
    mpu6050_init();

    // Start interrupts
    

    simulation_init();

    while (1)
    {
        simulation_step();
    }
}
