#include "i2c.h"
#include "i2c_config.h"

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h> // NULL

#include <avr/io.h>
#include <util/delay.h>

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


/// @brief Reads the ACK bit sent by slave.
static bool i2c_read_ack(void)
{
    i2c_sda_release();
    i2c_delay_us(I2C_HALF_CLOCK_US);

    // 9th clock pulse for ACK
    i2c_scl_release();
    i2c_delay_us(I2C_HALF_CLOCK_US);

    bool ack = i2c_is_sda_low();

    i2c_scl_low(); // pull clock back down
    return ack;
}


static void i2c_write_ack(bool ack)
{
    // Pull clock & data down for ACK
    i2c_scl_low();

    if (ack)
        i2c_sda_low(); // ACK
    else
        i2c_sda_release();
    i2c_delay_us(I2C_HALF_CLOCK_US);

    // Send ACK clock pulse
    i2c_scl_release();
    i2c_delay_us(I2C_HALF_CLOCK_US);

    // Pull clock back down
    i2c_scl_low();
    i2c_sda_release();
}


void i2c_release_pins(void)
{
    i2c_scl_release();
    i2c_sda_release();
}


void i2c_init(void)
{
    i2c_release_pins();
}


/// @brief Start I2C transaction.
static void i2c_start(void)
{
    // Ensure SCL is low before preparing the START
    i2c_scl_low();
    i2c_sda_release();
    i2c_delay_us(I2C_HALF_CLOCK_US);

    // Release SCL; wait for it to go high
    i2c_scl_release();
    i2c_delay_us(I2C_HALF_CLOCK_US);

    // SDA falling while SCL is high = START
    i2c_sda_low();
    i2c_delay_us(I2C_HALF_CLOCK_US);

    // Begin clocking
    i2c_scl_low();
}


/// @brief End I2C transaction.
static void i2c_stop(void)
{
    i2c_sda_low();
    i2c_delay_us(I2C_HALF_CLOCK_US);

    i2c_scl_release();
    i2c_delay_us(I2C_HALF_CLOCK_US);

    // STOP - SDA rises while SCL is high
    i2c_sda_release();
    i2c_delay_us(I2C_HALF_CLOCK_US);
}


/// @brief Write a byte over I2C.
/// Must have i2c_write_address_rw set prior.
static bool i2c_write_byte(uint8_t data)
{
    for (uint8_t i = 0; i < 8; ++i)
    {
        i2c_scl_low(); // pull clock low after each bit

        // Send individual bit MSB first
        // 0x80 = 0b1000 0000
        if (data & 0x80)
            i2c_sda_release();
        else
            i2c_sda_low();

        i2c_delay_us(I2C_HALF_CLOCK_US);
        i2c_scl_release();

        i2c_delay_us(I2C_HALF_CLOCK_US);
        data <<= 1;
    }

    i2c_scl_low(); // pull clock back down
    return i2c_read_ack();
}


/// @brief Write the 7-bit address of target device
/// and mode to the target device
///
/// @param addr target device address
/// @param rw mode I2C_READ=1 or I2C_WRITE=0
static bool i2c_write_address_rw(uint8_t addr, uint8_t rw)
{
    return i2c_write_byte((addr << 1) | rw);
}


/// @param ack true if more bytes will be read
static void i2c_read_byte(uint8_t* out_data, bool ack)
{
    uint8_t value = 0;

    for (uint8_t i = 0; i < 8; ++i)
    {
        // Pull clock down
        i2c_scl_low();
        i2c_delay_us(I2C_HALF_CLOCK_US);

        // Send clock pulse
        i2c_scl_release();
        i2c_delay_us(I2C_HALF_CLOCK_US);

        // Read bit and append it
        value <<= 1;

        if (i2c_is_sda_high())
        {
            value |= 1;
        }
    }

    *out_data = value;
    i2c_write_ack(ack);
}


bool i2c_write_register(
    uint8_t addr,
    uint8_t reg,
    uint8_t data)
{
    i2c_start();

    // Select device for writing
    if (!i2c_write_address_rw(addr, I2C_WRITE))
        goto fail;

    // Select the register inside the device
    if (!i2c_write_byte(reg))
        goto fail;

    if (!i2c_write_byte(data))
        goto fail;

    i2c_stop();
    return true;

fail:
    i2c_stop();
    return false;
}


bool i2c_read_register(
    uint8_t addr,
    uint8_t reg,
    uint8_t* out_data)
{
    if (out_data == NULL)
        return false;

    i2c_start();

    // Select device for writing
    if (!i2c_write_address_rw(addr, I2C_WRITE))
        goto fail;

    // Select the register inside the device
    if (!i2c_write_byte(reg))
        goto fail;

    // Switch to reading without releasing the bus
    i2c_start();

    if (!i2c_write_address_rw(addr, I2C_READ))
        goto fail;

    // Read one byte; NACK because we're finished
    i2c_read_byte(out_data, I2C_NACK);

    i2c_stop();
    return true;

fail:
    i2c_stop();
    return false;
}


bool i2c_read_registers(
    uint8_t addr,
    uint8_t reg,
    uint8_t* data,
    uint8_t len)
{
    if (data == NULL || len == 0)
        return false;

    i2c_start();

    if (!i2c_write_address_rw(addr, I2C_WRITE))
        goto fail;

    if (!i2c_write_byte(reg))
        goto fail;

    i2c_start(); // Repeated START

    if (!i2c_write_address_rw(addr, I2C_READ))
        goto fail;

    for (uint8_t i = 0; i < len; ++i)
    {
        // ACK every byte except the last one
        bool ack = (i < len - 1);
        i2c_read_byte(&data[i], ack);
    }

    i2c_stop();
    return true;

fail:
    i2c_stop();
    return false;
}


bool i2c_check_ack(uint8_t addr)
{
    i2c_start();

    bool ack = i2c_write_address_rw(addr, I2C_WRITE);

    i2c_stop();

    return ack;
}
