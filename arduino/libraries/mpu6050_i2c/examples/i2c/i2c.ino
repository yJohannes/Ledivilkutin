#include <stdint.h>

#include "i2c.h"
#include "mpu6050.h"
#include "mpu6050_regs.h"


bool g_initialized = false;

void setup()
{
  Serial.begin(38400);
  i2c_init();

  if (i2c_check_ack(MPU6050_ADDR))
    Serial.println("Acknowledged.");
  else
  {
    Serial.println("Failed to acknowledge.");
    return;
  }

  uint8_t out;
  if (i2c_read_register(MPU6050_ADDR, MPU6050_REG_WHO_AM_I, &out))
  {
    Serial.print("MPU6050_REG_WHO_AM_I=");
    Serial.print(out);
    Serial.println("");
  }
  else
  {
    Serial.println("Failed to read register MPU6050_REG_WHO_AM_I.");
  }


  uint8_t pwr_mgmt_1;
  i2c_read_register(MPU6050_ADDR, MPU6050_REG_PWR_MGMT_1, &pwr_mgmt_1);

  // (ii) Set SLEEP bit to 0
  // (i)  Set CYCLE bit to 1
  // (iii) Set TEMP_DIS bit to 1
  if (i2c_write_register(
    MPU6050_ADDR,
    MPU6050_REG_PWR_MGMT_1,
    pwr_mgmt_1 & ~MPU6050_PWR_MGMT_1_SLEEP
    | MPU6050_PWR_MGMT_1_CYCLE
    | MPU6050_PWR_MGMT_1_TEMP_DIS
  ))
    Serial.println("Set CYCLE=1, SLEEP=0.");
  else
    Serial.println("Failed to set CYCLE=1, SLEEP=0.");

  uint8_t pwr_mgmt_2;
  i2c_read_register(MPU6050_ADDR, MPU6050_REG_PWR_MGMT_2, &pwr_mgmt_2);

  // Set LP_WAKE_CTRL to 40 Hz & set gyro to standby
  // (iv) Set STBY_XG, STBY_YG, STBY_ZG bits to 1
  if (i2c_write_register(
    MPU6050_ADDR,
    MPU6050_REG_PWR_MGMT_2,
    pwr_mgmt_2 & ~MPU6050_PWR_MGMT_2_LP_WAKE_CTRL_MASK
    | MPU6050_PWR_MGMT_2_LP_WAKE_CTRL_3
    | MPU6050_PWR_MGMT_2_STBY_XG
    | MPU6050_PWR_MGMT_2_STBY_YG
    | MPU6050_PWR_MGMT_2_STBY_ZG
  ))
    Serial.println("Set LP_WAKE_CTRL to 40 Hz & set gyro to standby.");
  else
    Serial.println("Failed to set LP_WAKE_CTRL to 40 Hz & set gyro to standby.");

  g_initialized = true;
}


void loop()
{
  delay(500);

  if (!g_initialized)
    return;

  // uint8_t raw[6];

  int16_t ax, ay, az;

  if (!mpu6050_read_accel(&ax, &ay, &az))
  {
    Serial.println("Failed to get accel.");
    return;
  }

  int8_t ax8, ay8, az8;

  if (!mpu6050_read_accel_i8(&ax8, &ay8, &az8))
  {
    Serial.println("Failed to get accel i8.");
    return;
  }

  Serial.print("ax=");
  Serial.print(ax);
  Serial.print(", ay=");
  Serial.print(ay);
  Serial.print(", az=");
  Serial.print(az);

  Serial.print("\nax8=");
  Serial.print(ax8);
  Serial.print(", ay8=");
  Serial.print(ay8);
  Serial.print(", az8=");
  Serial.print(az8);

  Serial.println("");

}
