/**
 * @file i2c_arduino.c
 * @author Johannes Ylinen
 * @brief SDA and SCL pin configuration and manipulation for Arduino.
 * @date 2026-11-15
 */


#ifdef I2C_PLATFORM_ARDUINO


#include <stdint.h>
#include <stdbool.h>
#include <stddef.h> // NULL

#include <Arduino.h>

#define I2C_PIN_SDA A4
#define I2C_PIN_SCL A5


static inline void i2c_sda_low()
{
  digitalWrite(I2C_PIN_SDA, LOW);
  pinMode(I2C_PIN_SDA, OUTPUT);
}

static inline void i2c_scl_low()
{
  digitalWrite(I2C_PIN_SCL, LOW);
  pinMode(I2C_PIN_SCL, OUTPUT);
}

static inline void i2c_sda_release()
{
  pinMode(I2C_PIN_SDA, INPUT);
}

static inline void i2c_scl_release()
{
  pinMode(I2C_PIN_SCL, INPUT);
}

static inline bool i2c_is_sda_low()
{
    return digitalRead(I2C_PIN_SDA) == LOW;
}

static inline bool i2c_is_sda_high()
{
    return digitalRead(I2C_PIN_SDA) == HIGH;
}

static void i2c_delay_us(uint8_t n)
{
  delayMicroseconds(n);
}

#endif
