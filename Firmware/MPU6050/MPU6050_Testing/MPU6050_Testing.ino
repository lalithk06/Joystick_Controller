#include <Wire.h>
#include <MPU6050.h>

MPU6050 mpu;

void setup()
{
    pinMode(PC13, OUTPUT);
    digitalWrite(PC13, HIGH);

    Serial.begin(115200);
    delay(1000);

    // Explicitly set Blue Pill I2C pins
    Wire.setSDA(PB7);
    Wire.setSCL(PB6);
    Wire.begin();

    Serial.println("Initializing MPU6050");

    mpu.initialize();

    if(mpu.testConnection())
    {
        Serial.println("MPU6050 Connected");
        digitalWrite(PC13, LOW);
    }
    else
    {
        Serial.println("Failed");

        while(1)
        {
            digitalWrite(PC13,!digitalRead(PC13));
            delay(500);
        }
    }
}

void loop()
{
}