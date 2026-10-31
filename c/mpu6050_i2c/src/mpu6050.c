#include "mpu6050.h"
#include "i2c.h"

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h> // // NULL


static bool mpu6050_check_identity(void)
{
    uint8_t id;

    if (!i2c_read_register(
            MPU6050_ADDR,
            MPU6050_REG_WHO_AM_I,
            &id))
        return false;

    return id == MPU6050_WHO_AM_I;
}

// ------------------------------------------------ //
// MPU6050 to Accelerometer Only Low Power Mode //
// ------------------------------------------------ //
// https://cdn.sparkfun.com/datasheets/Sensors/Accelerometers/RM-MPU-6000A.pdf#%5B%7B%22num%22%3A92%2C%22gen%22%3A0%7D%2C%7B%22name%22%3A%22XYZ%22%7D%2C63%2C692%2C0%5D
static bool mpu6050_set_accel_only_low_power(void)
{
    uint8_t pwr_mgmt_1;
    uint8_t pwr_mgmt_2;

    if (!i2c_read_register(
            MPU6050_ADDR,
            MPU6050_REG_PWR_MGMT_1,
            &pwr_mgmt_1))
        return false;

    // (ii) Set SLEEP bit to 0
    // (i)  Set CYCLE bit to 1
    // (iii) Set TEMP_DIS bit to 1
    pwr_mgmt_1 =
        (pwr_mgmt_1 & ~MPU6050_PWR_MGMT_1_SLEEP)
        | MPU6050_PWR_MGMT_1_CYCLE
        | MPU6050_PWR_MGMT_1_TEMP_DIS;

    if (!i2c_write_register(
            MPU6050_ADDR,
            MPU6050_REG_PWR_MGMT_1,
            pwr_mgmt_1))
        return false;

    if (!i2c_read_register(
            MPU6050_ADDR,
            MPU6050_REG_PWR_MGMT_2,
            &pwr_mgmt_2))
        return false;

    // Set LP_WAKE_CTRL to 40 Hz & set gyro + accel z to standby
    // (iv) Set STBY_XG, STBY_YG, STBY_ZG bits to 1
    pwr_mgmt_2 =
        (pwr_mgmt_2 & ~MPU6050_PWR_MGMT_2_LP_WAKE_CTRL_MASK)
        | MPU6050_PWR_MGMT_2_LP_WAKE_CTRL_3
        | MPU6050_PWR_MGMT_2_STBY_XG
        | MPU6050_PWR_MGMT_2_STBY_YG
        | MPU6050_PWR_MGMT_2_STBY_ZG
        | MPU6050_PWR_MGMT_2_STBY_ZA;

    if (!i2c_write_register(
            MPU6050_ADDR,
            MPU6050_REG_PWR_MGMT_2,
            pwr_mgmt_2))
        return false;

    return true;
}

bool mpu6050_test_connection(void)
{
    return i2c_check_ack(MPU6050_ADDR);
}

bool mpu6050_init(void)
{
    if (!mpu6050_test_connection())
        return false;

    if (!mpu6050_check_identity())
        return false;

    if (!mpu6050_set_accel_only_low_power())
        return false;

    return true;
}

bool mpu6050_set_sleep(bool sleep)
{
    uint8_t pwr_mgmt_1;

    if (!i2c_read_register(
            MPU6050_ADDR,
            MPU6050_REG_PWR_MGMT_1,
            &pwr_mgmt_1))
        return false;

    if (sleep) // Set bit
        pwr_mgmt_1 |= MPU6050_PWR_MGMT_1_SLEEP;
    else // Clear bit
        pwr_mgmt_1 &= ~MPU6050_PWR_MGMT_1_SLEEP;

    if (!i2c_write_register(
            MPU6050_ADDR,
            MPU6050_REG_PWR_MGMT_1,
            pwr_mgmt_1))
        return false;

    return true;
}

bool mpu6050_read_accel(int16_t* ax, int16_t* ay, int16_t* az)
{
    if (ax == NULL || ay == NULL || az == NULL)
        return false;

    uint8_t raw[6];

    if (!i2c_read_registers(
        MPU6050_ADDR,
        MPU6050_REG_ACCEL_XOUT_H,
        raw,
        sizeof(raw)
    ))
        return false;

    *ax = (int16_t)(((uint16_t)raw[0] << 8) | raw[1]);
    *ay = (int16_t)(((uint16_t)raw[2] << 8) | raw[3]);
    *az = (int16_t)(((uint16_t)raw[4] << 8) | raw[5]);

    return true;
}

bool mpu6050_read_accel_i8(int8_t *ax, int8_t *ay, int8_t *az)
{
    if (ax == NULL || ay == NULL || az == NULL)
        return false;

    uint8_t raw[5];

    if (!i2c_read_registers(
            MPU6050_ADDR,
            MPU6050_REG_ACCEL_XOUT_H,
            raw,
            sizeof(raw)))
    {
        return false;
    }

    // Extract high bytes directly
    *ax = (int8_t)raw[0];
    *ay = (int8_t)raw[2];
    *az = (int8_t)raw[4];

    return true;
}


bool mpu6050_read_accel_xy(int16_t* ax, int16_t* ay)
{
    if (ax == NULL || ay == NULL)
        return false;

    uint8_t raw[4];

    if (!i2c_read_registers(
        MPU6050_ADDR,
        MPU6050_REG_ACCEL_XOUT_H,
        raw,
        sizeof(raw)
    ))
        return false;

    *ax = (int16_t)(((uint16_t)raw[0] << 8) | raw[1]);
    *ay = (int16_t)(((uint16_t)raw[2] << 8) | raw[3]);

    return true;
}

bool mpu6050_read_accel_xy_i8(int8_t *ax, int8_t *ay)
{
    if (ax == NULL || ay == NULL)
        return false;


    if (!i2c_read_registers(
            MPU6050_ADDR,
            MPU6050_REG_ACCEL_XOUT_H,
            raw,
            sizeof(raw)))
    {
        return false;
    }

    // Extract high bytes directly
    *ax = (int8_t)raw[0];
    *ay = (int8_t)raw[2];

    return true;
}
