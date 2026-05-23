#include <Wire.h>
#include <MPU6050.h>

MPU6050 mpu;

void setup() {

  Serial.begin(115200);

  Wire.begin();

  Serial.println("Initializing MPU6050...");

  mpu.initialize();

  if (mpu.testConnection()) {

    Serial.println("MPU6050 connected successfully!");

  } else {

    Serial.println("MPU6050 connection failed!");

    while (1);
  }
}

void loop() {

  int16_t ax, ay, az;
  int16_t gx, gy, gz;

  // Read accel and gyro
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
