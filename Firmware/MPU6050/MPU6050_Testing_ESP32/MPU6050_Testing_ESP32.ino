#include <Wire.h>
#include <MPU6050.h>

MPU6050 mpu;

void setup()
{
    Serial.begin(115200);
    delay(2000);

    Serial.println();
    Serial.println("ESP32 STARTED...");

    // SDA = 21, SCL = 22
    Wire.begin(21, 22);

    Serial.println("Initializing MPU6050...");

    mpu.initialize();

    Serial.println("Testing connection...");

    if (mpu.testConnection())
    {
        Serial.println("MPU6050 Connected Successfully!");
    }
    else
    {
        Serial.println("MPU6050 Connection Failed!");
    }
}

void loop()
{
    int16_t ax, ay, az;
    int16_t gx, gy, gz;

    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

    Serial.println("---------------");

    Serial.print("AX: ");
    Serial.println(ax);

    Serial.print("AY: ");
    Serial.println(ay);

    Serial.print("AZ: ");
    Serial.println(az);

    Serial.print("GX: ");
    Serial.println(gx);

    Serial.print("GY: ");
    Serial.println(gy);

    Serial.print("GZ: ");
    Serial.println(gz);

    delay(500);
}