#include "i2c_config.h"

#ifdef I2C_PLATFORM_AVR

#include "i2c.h"

#include <avr/io.h>
// #define F_CPU 8000000UL
#include <util/delay.h>
#include <stdint.h>
#include <stdbool.h>

#define I2C_PIN_SDA PB5
#define I2C_PIN_SCL PB6

#define I2C_SDA_MASK (1 << I2C_PIN_SDA)
#define I2C_SCL_MASK (1 << I2C_PIN_SCL)


static inline void i2c_sda_low()
{
    PORTB &= ~I2C_SDA_MASK;
    DDRB |= I2C_SDA_MASK;
}

static inline void i2c_scl_low()
{
    PORTB &= ~I2C_SCL_MASK;
    DDRB |= I2C_SCL_MASK;
}

static inline void i2c_sda_release()
{
    PORTB &= ~I2C_SDA_MASK;
    DDRB &= ~I2C_SDA_MASK;
}

static inline void i2c_scl_release()
{
    PORTB &= ~I2C_SCL_MASK;
    DDRB &= ~I2C_SCL_MASK;
}

static inline bool i2c_is_sda_low()
{
    return (PINB & I2C_SDA_MASK) == 0;
}

static inline bool i2c_is_sda_high()
{
    return (PINB & I2C_SDA_MASK) != 0;
}

static void i2c_delay_us(uint8_t n)
{
    while (n--)
    {
        _delay_us(1); // Expects compile time constant
    }
}

#endif
