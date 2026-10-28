/*
  Full MPU6050 library documentation here:
  https://github.com/ElectronicCats/mpu6050/wiki
*/
#include "I2Cdev.h"
#include "MPU6050.h"
#include <Wire.h>


/* MPU6050 default I2C address is 0x68*/
MPU6050 mpu(0x68, &Wire); //Use for AD0 low, but 2nd Wire (TWI/I2C) object.

#define OUTPUT_PYTHON

#define PIN_NX 3
#define PIN_PX 5

#define PIN_NY 6
#define PIN_PY 9

#define PIN_NZ 10
#define PIN_PZ 11


int16_t ax, ay, az;


void setup()
{
  // Initializate Serial wo work well at 8MHz/16MHz
  Serial.begin(38400);
  Wire.begin();

  Serial.println("Scanning...");

  for (byte address = 1; address < 127; address++) {
      Wire.beginTransmission(address);

      byte error = Wire.endTransmission();

      if (error == 0) {
          Serial.print("Found 0x");
          Serial.println(address, HEX);
      }
  }

  Serial.println("Done");

  pinMode(PIN_NX, OUTPUT);
  pinMode(PIN_PX, OUTPUT);
  pinMode(PIN_NY, OUTPUT);
  pinMode(PIN_PY, OUTPUT);
  pinMode(PIN_NZ, OUTPUT);
  pinMode(PIN_PZ, OUTPUT);


  /*--Start I2C interface--*/
  #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    Wire.begin();
  #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
    Fastwire::setup(400, true);
  #endif

  /*Initialize device and check connection*/
  Serial.println("Initializing MPU...");
  mpu.initialize();
  Serial.println("Testing MPU6050 connection...");
  if(mpu.testConnection() ==  false){
    Serial.println("MPU6050 connection failed");
    while(true);
  }
  else{
    Serial.println("MPU6050 connection successful");
  }

  mpu.setDLPFMode(MPU6050_DLPF_BW_10);

  /* Use the code below to change accel/gyro offset values. Use MPU6050_Zero to obtain the recommended offsets */

  Serial.println("Updating internal sensor offsets...\n");
  Serial.println("Keep the MPU6050 stationary and level...");
  delay(1000);

  mpu.CalibrateAccel(6);
  // mpu.CalibrateGyro(6);

  // mpu.setXAccelOffset(0); //Set your accelerometer offset for axis X
  // mpu.setYAccelOffset(0); //Set your accelerometer offset for axis Y
  // mpu.setZAccelOffset(0); //Set your accelerometer offset for axis Z

  /*Print the defined offsets*/
  Serial.print("\t");
  Serial.print(mpu.getXAccelOffset());
  Serial.print("\t");
  Serial.print(mpu.getYAccelOffset());
  Serial.print("\t");
  Serial.print(mpu.getZAccelOffset());
  Serial.print("\t");

  /*Configure board LED pin for output*/
  pinMode(LED_BUILTIN, OUTPUT);

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
}

uint8_t accelToPwm(int16_t value, float scale = 1.0)
{
  int32_t pwm = abs((int32_t)value) * 255L / 16384L;
  return (uint8_t)constrain((int32_t)(pwm * scale), 0, 255);
}

void loop()
{
  mpu.getAcceleration(&ax, &ay, &az);

  /*Print the obtained data on the defined format*/
#ifdef OUTPUT_READABLE_ACCELGYRO
  Serial.print("a/g:\t");
  Serial.print(ax); Serial.print("\t");
  Serial.print(ay); Serial.print("\t");
  Serial.print(az); Serial.print("\t");
  Serial.print("\n");
#endif

#ifdef OUTPUT_PYTHON
  Serial.print(ax);
  Serial.print(",");
  Serial.println(ay);
  delay(50);

#endif

  const int16_t THRESHOLD = 2000;

  analogWrite(PIN_NX, ax < -THRESHOLD ? accelToPwm(ax, 1.0f) : 0);
  analogWrite(PIN_PX, ax >  THRESHOLD ? accelToPwm(ax, 1.0f) : 0);

  analogWrite(PIN_NY, ay < -THRESHOLD ? accelToPwm(ay, 0.9f) : 0);
  analogWrite(PIN_PY, ay >  THRESHOLD ? accelToPwm(ay, 0.9f) : 0);

  analogWrite(PIN_NZ, az < -THRESHOLD ? accelToPwm(az, 0.5f) : 0);
  analogWrite(PIN_PZ, az >  THRESHOLD ? accelToPwm(az, 0.5f) : 0);

  delay(10);
}

