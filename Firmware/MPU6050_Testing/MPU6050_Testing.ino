#include <Wire.h>
#include <MPU6050.h>

MPU6050 mpu;

int16_t ax, ay, az;
int16_t gx, gy, gz;

float AccX, AccY, AccZ;
float GyroX, GyroY, GyroZ;

float roll = 0;
float pitch = 0;

unsigned long previousTime = 0;
float dt;

float alpha = 0.96;

// gyro offsets
float gyroXoffset=0;
float gyroYoffset=0;
float gyroZoffset=0;

void setup()
{
    Serial.begin(115200);
    delay(3000);

    Wire.begin(21,22);

    Serial.println("Initializing MPU6050...");

    mpu.initialize();
    delay(100);

    Serial.println("MPU6050 Initialized");

    // Gyro calibration
    Serial.println("Keep sensor still...");

    for(int i=0;i<500;i++)
    {
        mpu.getMotion6(&ax,&ay,&az,&gx,&gy,&gz);

        gyroXoffset += gx;
        gyroYoffset += gy;
        gyroZoffset += gz;

        delay(3);
    }

    gyroXoffset/=500;
    gyroYoffset/=500;
    gyroZoffset/=500;

    Serial.println("Calibration complete");

    // Initial orientation
    mpu.getMotion6(&ax,&ay,&az,&gx,&gy,&gz);

    AccX=ax/16384.0;
    AccY=ay/16384.0;
    AccZ=az/16384.0;

    pitch=atan2(-AccX,
            sqrt(AccY*AccY+AccZ*AccZ))
            *180.0/M_PI;

    roll=atan2(AccY,
           sqrt(AccX*AccX+AccZ*AccZ))
           *180.0/M_PI;

    previousTime=millis();
}

void loop()
{
    mpu.getMotion6(&ax,&ay,&az,&gx,&gy,&gz);

    unsigned long currentTime=millis();

    dt=(currentTime-previousTime)/1000.0;

    previousTime=currentTime;

    if(dt>0.1)
        dt=0.1;

    // Accelerometer
    AccX=ax/16384.0;
    AccY=ay/16384.0;
    AccZ=az/16384.0;

    // Gyroscope with calibration offset removed
    GyroX=(gx-gyroXoffset)/131.0;
    GyroY=(gy-gyroYoffset)/131.0;
    GyroZ=(gz-gyroZoffset)/131.0;

    float accPitch=
        atan2(-AccX,
        sqrt(AccY*AccY+AccZ*AccZ))
        *180.0/M_PI;

    float accRoll=
        atan2(AccY,
        sqrt(AccX*AccX+AccZ*AccZ))
        *180.0/M_PI;

    // Complementary filter
    pitch=
        alpha*(pitch+GyroY*dt)
        +(1-alpha)*accPitch;

    roll=
        alpha*(roll+GyroX*dt)
        +(1-alpha)*accRoll;

    Serial.print("Pitch:");
    Serial.print(pitch,2);

    Serial.print(",Roll:");
    Serial.print(roll,2);

    Serial.print(",GyroZ:");
    Serial.println(GyroZ,2);

    delay(10);
}