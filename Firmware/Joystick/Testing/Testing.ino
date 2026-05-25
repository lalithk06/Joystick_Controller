#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

#define JOY1_X  34
#define JOY1_Y  35
#define JOY1_SW 32

#define JOY2_X  33
#define JOY2_Y  36
#define JOY2_SW 25

int JOY1_X_CENTER;
int JOY1_Y_CENTER;
int JOY2_X_CENTER;
int JOY2_Y_CENTER;

void setup()
{
    Serial.begin(115200);

    SerialBT.begin("Joystick_Controller");

    pinMode(JOY1_SW, INPUT_PULLUP);
    pinMode(JOY2_SW, INPUT_PULLUP);

    Serial.println("Keep joysticks untouched...");
    SerialBT.println("Keep joysticks untouched...");

    long sumJ1X=0;
    long sumJ1Y=0;
    long sumJ2X=0;
    long sumJ2Y=0;

    // Take 100 samples
    for(int i=0;i<100;i++)
    {
        sumJ1X+=analogRead(JOY1_X);
        sumJ1Y+=analogRead(JOY1_Y);

        sumJ2X+=analogRead(JOY2_X);
        sumJ2Y+=analogRead(JOY2_Y);

        delay(20);
    }

    // Convert averaged values to 0–255 scale
    JOY1_X_CENTER=
        map(sumJ1X/100,0,4095,0,255);

    JOY1_Y_CENTER=
        map(sumJ1Y/100,0,4095,0,255);

    JOY2_X_CENTER=
        map(sumJ2X/100,0,4095,0,255);

    JOY2_Y_CENTER=
        map(sumJ2Y/100,0,4095,0,255);

    Serial.println("Calibration Done");
    SerialBT.println("Calibration Done");

    Serial.print("J1 Center:");
    Serial.print(JOY1_X_CENTER);
    Serial.print(",");
    Serial.println(JOY1_Y_CENTER);

    Serial.print("J2 Center:");
    Serial.print(JOY2_X_CENTER);
    Serial.print(",");
    Serial.println(JOY2_Y_CENTER);
}

void loop()
{
    int joy1X=map(
        analogRead(JOY1_X),
        0,4095,0,255);

    int joy1Y=map(
        analogRead(JOY1_Y),
        0,4095,0,255);

    int joy2X=map(
        analogRead(JOY2_X),
        0,4095,0,255);

    int joy2Y=map(
        analogRead(JOY2_Y),
        0,4095,0,255);

    bool joy1Btn=!digitalRead(JOY1_SW);
    bool joy2Btn=!digitalRead(JOY2_SW);

    String out=
    "J1X:"+String(joy1X)+
    " J1Y:"+String(joy1Y)+
    " J1Btn:"+String(joy1Btn)+
    " | J2X:"+String(joy2X)+
    " J2Y:"+String(joy2Y)+
    " J2Btn:"+String(joy2Btn);

    Serial.println(out);
    SerialBT.println(out);

    delay(100);
}