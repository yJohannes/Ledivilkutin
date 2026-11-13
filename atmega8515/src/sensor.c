/**
 * @file sensor.h
 * @author Johannes Ylinen
 * @date 2026-11-14
 */

#include "sensor.h"

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/common.h>

#include <stdint.h>
#include <stdbool.h>

#include "ledivilkutin_config.h"
#include "sim.h"
#include "sim_config.h"

#ifndef DISABLE_I2C

#include "i2c.h"
#include "mpu6050.h"

/// @brief Maps acceleration to range, e.g. 3 -> [-16, 15]
#define ACCELERATION_SHIFT 3


bool sensor_connect(void)
{
    i2c_init();

    uint8_t old_sreg = SREG;
    cli();
    bool ok = mpu6050_init();
    SREG = old_sreg;
    return ok;
}

bool sensor_is_alive(void)
{
    uint8_t old_sreg = SREG;
    cli();
    bool ok = mpu6050_is_connected();
    SREG = old_sreg;
    return ok;
}

void sensor_set_sleep(bool sleep)
{
    if (sleep)
    {
        // Failure is acceptable: the sensor may already be disconnected.
        mpu6050_set_sleep(true);

        // Ensure I2C pins are released for possible programming.
        i2c_release_pins();

        return;
    }

    // If disconnected, next connectivity check handles recovery.
    mpu6050_set_sleep(false);
}


void sensor_update_acceleration(void)
{
    // Moved out of simulation_step for performance.
    if (--sim_accel_timer == 0)
    {
        sim_accel_timer = SIM_ACCELERATION_INTERVAL;

        {
            // Don't allow timers to disturb I2C
            uint8_t old_sreg = SREG;
            cli();

            // SWAP X- AND Y-AXIS
            if (!mpu6050_read_accel_xy_i8(&a.y, &a.x))
            {
                // On failure set acceleration to zero and if this was
                // caused by MPU6050 disconnecting the code above will
                // handle it.
                a.x = 0;
                a.y = 0;
            }

            SREG = old_sreg;
        }

        a.x >>= ACCELERATION_SHIFT;
        a.y >>= ACCELERATION_SHIFT;

        a.x = -a.x;
        // a.y = -a.y;
    }
    else
    {
        a.x = 0;
        a.y = 0;
    }
}

#else /* DISABLE_I2C: stubs */

bool sensor_connect(void) { return true; }
bool sensor_is_alive(void) { return true; }
void sensor_set_sleep(bool sleep) { (void)sleep; }
void sensor_update_acceleration(void) { a.x = 0; a.y = 0; }

#endif // DISABLE_I2C
