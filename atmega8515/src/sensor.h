#ifndef SENSOR_H
#define SENSOR_H

#include <stdbool.h>


/// @brief One full MPU6050 init attempt.
/// Interrupts are masked for its duration.
bool sensor_connect(void);

/// @brief Connectivity check.
/// Interrupts are masked for its duration.
bool sensor_is_alive(void);

/// @brief Put the sensor to sleep (if it was ever started) and
/// release I2C pins so the device can be reprogrammed.
void sensor_set_sleep(bool sleep);

/// @brief Read acceleration into the simulation's acceleration vector
/// (every SIM_ACCELERATION_INTERVAL calls, zero otherwise).
void sensor_update_acceleration(void);

#endif
