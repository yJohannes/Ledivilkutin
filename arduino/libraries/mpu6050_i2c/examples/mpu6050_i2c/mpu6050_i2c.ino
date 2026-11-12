#include "i2c.h"
#include "mpu6050.h"
#include "mpu6050_regs.h"

int16_t ax, ay, az;
int8_t ax8, ay8, az8;


void setup()
{
  Serial.begin(38400);

  Serial.println("Initializing...");
  i2c_init();

  while (!i2c_check_ack(MPU6050_ADDR))
  {
    Serial.println("Failed to connect to MPU6050... Retrying...");
    delay(250);
  }

  uint8_t who_am_i;
  while (!i2c_read_register(MPU6050_ADDR, MPU6050_REG_WHO_AM_I, &who_am_i))
  {
    Serial.println("Failed to read MPU6050 register WHO_AM_I... Retrying...");
  }

  Serial.print("MPU6050 WHO_AM_I=");
  Serial.print(who_am_i);
  Serial.println("");

  while (!mpu6050_init())
  {
    Serial.println("Failed to initialize MPU6050... Retrying...");
    delay(250);
  }

  Serial.println("Initialization done.");
}


void loop()
{
  mpu6050_read_accel(&ax, &ay, &az);
  mpu6050_read_accel_i8(&ax8, &ay8, &az8);

  Serial.print("Accel raw:  X="); Serial.print(ax);
  Serial.print(" Y="); Serial.print(ay);
  Serial.print(" Z="); Serial.println(az);

  Serial.print("Accel int8: X="); Serial.print(ax8);
  Serial.print(" Y="); Serial.print(ay8);
  Serial.print(" Z="); Serial.println(az8);

  delay(250);
}

