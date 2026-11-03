#include "i2c.h"
#include "mpu6050.h"
#include "mpu6050_regs.h"

#define PIN_NX 3
#define PIN_PX 5
#define PIN_NY 6
#define PIN_PY 9
#define PIN_NZ 10
#define PIN_PZ 11


int16_t ax, ay, az;
int8_t ax8, ay8, az8;


static uint8_t accelToPwm(int16_t value, float scale = 1.0)
{
  int32_t pwm = abs((int32_t)value) * 255L / 16384L;
  return (uint8_t)constrain((int32_t)(pwm * scale), 0, 255);
}


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

  while (!mpu6050_init())
  {
    Serial.println("Failed to initialize MPU6050... Retrying...");
    delay(250);
  }

  pinMode(PIN_NX, OUTPUT);
  pinMode(PIN_PX, OUTPUT);
  pinMode(PIN_NY, OUTPUT);
  pinMode(PIN_PY, OUTPUT);
  pinMode(PIN_NZ, OUTPUT);
  pinMode(PIN_PZ, OUTPUT);

  digitalWrite(PIN_NX, HIGH);
  delay(50);
  digitalWrite(PIN_NX, LOW);

  digitalWrite(PIN_PX, HIGH);
  delay(50);
  digitalWrite(PIN_PX, LOW);

  digitalWrite(PIN_NY, HIGH);
  delay(50);
  digitalWrite(PIN_NY, LOW);

  digitalWrite(PIN_PY, HIGH);
  delay(50);
  digitalWrite(PIN_PY, LOW);

  digitalWrite(PIN_NZ, HIGH);
  delay(50);
  digitalWrite(PIN_NZ, LOW);

  digitalWrite(PIN_PZ, HIGH);
  delay(50);
  digitalWrite(PIN_PZ, LOW);

  Serial.println("Initialization done.");
}


void loop()
{
  mpu6050_read_accel(&ax, &ay, &az);

  Serial.print(ax);
  Serial.print(",");
  Serial.println(ay);

  const int16_t THRESHOLD = 2000;

  analogWrite(PIN_NX, ax < -THRESHOLD ? accelToPwm(ax, 1.0f) : 0);
  analogWrite(PIN_PX, ax >  THRESHOLD ? accelToPwm(ax, 1.0f) : 0);

  analogWrite(PIN_NY, ay < -THRESHOLD ? accelToPwm(ay, 0.9f) : 0);
  analogWrite(PIN_PY, ay >  THRESHOLD ? accelToPwm(ay, 0.9f) : 0);

  analogWrite(PIN_NZ, az < -THRESHOLD ? accelToPwm(az, 0.5f) : 0);
  analogWrite(PIN_PZ, az >  THRESHOLD ? accelToPwm(az, 0.5f) : 0);

  delay(50);
}

