/**
 * @file mpu6050.c
 * @author Johannes Ylinen
 * @date 2026-10-22
 */

#include "mpu6050.h"

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h> // // NULL

#include "mpu6050_regs.h"
#include "i2c.h"


/// @brief Configure MPU6050 for accelerometer XY-only operation.
/// @see https://cdn.sparkfun.com/datasheets/Sensors/Accelerometers/RM-MPU-6000A.pdf
static bool mpu6050_set_accel_xy_only(void)
{
    // PWR_MGMT_1:
    // Internal oscillator, SLEEP = 0, CYCLE = 0, TEMP_DIS = 0.
    if (!i2c_write_register(
            MPU6050_ADDR,
            MPU6050_REG_PWR_MGMT_1,
            MPU6050_PWR_MGMT_1_CLKSEL_INTERNAL))
        return false;

    // PWR_MGMT_2:
    // Accelerometer X/Y enabled, Z disabled.
    // All gyroscope axes in standby.
    if (!i2c_write_register(
            MPU6050_ADDR,
            MPU6050_REG_PWR_MGMT_2,
            MPU6050_PWR_MGMT_2_STBY_XG
            | MPU6050_PWR_MGMT_2_STBY_YG
            | MPU6050_PWR_MGMT_2_STBY_ZG
            | MPU6050_PWR_MGMT_2_STBY_ZA))
        return false;

    // ACCEL_CONFIG: +/- 2g.
    if (!i2c_write_register(
            MPU6050_ADDR,
            MPU6050_REG_ACCEL_CONFIG,
            MPU6050_ACCEL_CONFIG_AFS_SEL_2G))
        return false;

    // CONFIG: DLPF_CFG = 3 (44 Hz accelerometer bandwidth).
    if (!i2c_write_register(
            MPU6050_ADDR,
            MPU6050_REG_CONFIG,
            MPU6050_CONFIG_DLPF_CFG_44HZ_ACCEL))
        return false;

    // SMPLRT_DIV: 125 Hz output rate.
    if (!i2c_write_register(
            MPU6050_ADDR,
            MPU6050_REG_SMPLRT_DIV,
            MPU6050_SMPLRT_DIV_125HZ))
        return false;

    return true;
}


/* ====================================================================
 * Public API
 * ==================================================================== */

bool mpu6050_check_identity(void)
{
    uint8_t id;

    if (!i2c_read_register(
            MPU6050_ADDR,
            MPU6050_REG_WHO_AM_I,
            &id))
        return false;

    return id == MPU6050_WHO_AM_I
        || id == MPU6050_WHO_AM_I_CLONE;
}


bool mpu6050_is_connected(void)
{
    return i2c_check_ack(MPU6050_ADDR) && mpu6050_check_identity();
}


bool mpu6050_init(void)
{
    if (!mpu6050_is_connected())
        return false;

    if (!mpu6050_check_identity())
        return false;

    // if (!mpu6050_set_accel_only_low_power())
    //     return false;

    if (!mpu6050_set_accel_xy_only())
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

    uint8_t raw[3];

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
