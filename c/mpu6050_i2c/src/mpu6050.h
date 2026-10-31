/// @file mpu6050.h
/// @author Johannes Ylinen
/// @brief Real awesome stuff for MPU6050.
/// @see https://cdn.sparkfun.com/datasheets/Sensors/Accelerometers/RM-MPU-6000A.pdf


#ifndef MPU6050_H
#define MPU6050_H

#include <stdint.h>
#include <stdbool.h>


#ifdef __cplusplus
extern "C" {
#endif


/// @brief Wakes MPU6050 up and sets it to Accelerometer
// XY only & Low Power Mode
bool mpu6050_init(void);
bool mpu6050_is_connected(void);
bool mpu6050_set_sleep(bool sleep);
bool mpu6050_set_dlpf(uint8_t level); // Not implemented
bool mpu6050_read_accel(int16_t* ax, int16_t* ay, int16_t* az);
bool mpu6050_read_accel_i8(int8_t *ax, int8_t *ay, int8_t *az);
bool mpu6050_read_accel_xy(int16_t* ax, int16_t* ay);
bool mpu6050_read_accel_xy_i8(int8_t *ax, int8_t *ay);

#ifdef __cplusplus
}
#endif

#endif // MPU6050_H
