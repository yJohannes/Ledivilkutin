/// @file i2c.h
/// @author Johannes Ylinen
/// @brief A very elementary I2C implementation for a MCU + MPU6050.
///
/// @see https://www.nxp.com/docs/en/user-guide/UM10204.pdf

#ifndef I2C_H
#define I2C_H

#include <stdint.h>
#include <stdbool.h>


#define I2C_WRITE 0
#define I2C_READ  1

#define I2C_ACK   true
#define I2C_NACK  false


#ifdef __cplusplus
extern "C" {
#endif

/// @brief Initialize PIN_SDA and PIN_SCL for I2C.
void i2c_init(void);

/// @brief Release PIN_SDA and PIN_SCL.
void i2c_release_pins(void);

/// @brief Check if target device acknowledges.
/// @returns true on success
bool i2c_check_ack(uint8_t addr);

/// @brief Read register of the target device.
/// @returns true on success
bool i2c_write_register(
    uint8_t addr,
    uint8_t reg,
    uint8_t data);

/// @brief Read register of the target device.
/// @returns true on success
bool i2c_read_register(
    uint8_t addr,
    uint8_t reg,
    uint8_t* out_data);

/// @brief Read multiple consecutive registers starting at the specified register.
/// @note Requires device support for sequential register reads.
/// @returns true on success
bool i2c_read_registers(
    uint8_t addr,
    uint8_t reg,
    uint8_t* data,
    uint8_t len);


#ifdef __cplusplus
}
#endif

#endif // I2C_H
