#include <Wire.h>
#include <MPU6050.h>

// Explicit I2C pins for STM32
TwoWire I2CBus(PB7, PB6);

MPU6050 mpu;

void setup() {

  pinMode(PC13, OUTPUT);

  // LED OFF initially
  digitalWrite(PC13, HIGH);

  // Start serial
  Serial.begin(115200);

  // Start I2C
  I2CBus.begin();

  // Use this bus for MPU6050
  mpu.initialize();

  Serial.println("Testing MPU6050...");

  // Check connection
  if (mpu.testConnection()) {

    Serial.println("MPU6050 connected!");

    // LED ON
    digitalWrite(PC13, LOW);

  } else {

    Serial.println("MPU6050 connection failed!");

    // Blink forever if failed
    while (1) {

      digitalWrite(PC13, LOW);
      delay(100);

      digitalWrite(PC13, HIGH);
      delay(100);
    }
  }
}

void loop() {

  int16_t ax, ay, az;
  int16_t gx, gy, gz;

  // Read accelerometer and gyroscope
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  // Print accelerometer
  Serial.print("Accel X: ");
  Serial.print(ax);

  Serial.print("  Y: ");
  Serial.print(ay);

  Serial.print("  Z: ");
  Serial.print(az);

  // Print gyroscope
  Serial.print("   |   Gyro X: ");
  Serial.print(gx);

  Serial.print("  Y: ");
  Serial.print(gy);

  Serial.print("  Z: ");
  Serial.println(gz);

  delay(500);
}