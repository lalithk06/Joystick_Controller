#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Wire.h>
#include <MPU6050.h>

// ================= PIN MAPPING =================
#define JOY1_X  34
#define JOY1_Y  35
#define JOY1_SW 32

#define JOY2_X  33
#define JOY2_Y  36
#define JOY2_SW 25

#define POT1    39
#define POT2    26

#define NRF_CE  4
#define NRF_CSN 5

#define BTN1    13
#define BTN2    14
#define BTN3    16
#define BTN4    17
#define BTN5    27
#define BTN6    2

#define TGL1    12
#define TGL2    15

// ================= nRF24L01 =================
RF24 radio(NRF_CE, NRF_CSN);
const byte address[6] = "00001";

// ================= DATA PACKAGE =================
// Total: 18 bytes (well within 32-byte NRF limit)
struct Data_Package {
    byte joy1_X;      // 0-255
    byte joy1_Y;      // 0-255
    byte joy2_X;      // 0-255
    byte joy2_Y;      // 0-255
    byte pot1;        // 0-255
    byte pot2;        // 0-255
    byte pitch;       // mapped 0-255 (raw -90~+90 → 0~255)
    byte roll;        // mapped 0-255
    byte j1Btn;       // 0 or 1
    byte j2Btn;       // 0 or 1
    byte btn1;        // 0 or 1
    byte btn2;        // 0 or 1
    byte btn3;        // 0 or 1
    byte btn4;        // 0 or 1
    byte btn5;        // 0 or 1
    byte btn6;        // 0 or 1
    byte tgl1;        // 0 or 1
    byte tgl2;        // 0 or 1
};
Data_Package data;

// ================= MPU6050 =================
MPU6050 mpu;

int16_t ax, ay, az;
int16_t gx, gy, gz;

float AccX, AccY, AccZ;
float GyroX, GyroY, GyroZ;
float roll_f  = 0;
float pitch_f = 0;
float alpha   = 0.96;
float dt;
unsigned long previousTime = 0;

float gyroXoffset = 0;
float gyroYoffset = 0;
float gyroZoffset = 0;

// ================= JOYSTICK CALIBRATION =================
int joy1X_center = 127;
int joy1Y_center = 127;
int joy2X_center = 127;
int joy2Y_center = 127;

#define DEADZONE       10
#define CALIB_DURATION 5000   // 5 seconds sampling
#define CALIB_SAMPLES  200    // samples during calibration

// ================= HELPERS =================

// Dynamic deadzone using calibrated center
int applyDeadzone(int value, int center) {
    if (abs(value - center) < DEADZONE)
        return center;
    return value;
}

// Map joystick with calibrated center as midpoint
// Lower half:  center → 0   maps to 0   → 127
// Upper half:  center → 255 maps to 128 → 255
byte mapJoystick(int value, int center) {
    value = constrain(value, 0, 255);
    if (value < center)
        return (byte)map(value, 0, center, 0, 127);
    else
        return (byte)map(value, center, 255, 128, 255);
}

void resetData() {
    data.joy1_X = 127;
    data.joy1_Y = 127;
    data.joy2_X = 127;
    data.joy2_Y = 127;
    data.pot1   = 0;
    data.pot2   = 0;
    data.pitch  = 127;
    data.roll   = 127;
    data.j1Btn  = 1;
    data.j2Btn  = 1;
    data.btn1   = 1;
    data.btn2   = 1;
    data.btn3   = 1;
    data.btn4   = 1;
    data.btn5   = 1;
    data.btn6   = 1;
    data.tgl1   = 0;
    data.tgl2   = 0;
}

// ================= JOYSTICK DYNAMIC CALIBRATION =================
void calibrateJoysticks() {
    Serial.print("Calibrating joysticks for 5s (leave centered)");

    long j1x = 0, j1y = 0, j2x = 0, j2y = 0;
    int interval = CALIB_DURATION / CALIB_SAMPLES;  // ~25ms per sample

    for (int i = 0; i < CALIB_SAMPLES; i++) {
        j1x += map(analogRead(JOY1_X), 0, 4095, 0, 255);
        j1y += map(analogRead(JOY1_Y), 0, 4095, 0, 255);
        j2x += map(analogRead(JOY2_X), 0, 4095, 0, 255);
        j2y += map(analogRead(JOY2_Y), 0, 4095, 0, 255);
        delay(interval);
        if (i % 40 == 0) Serial.print(".");
    }

    joy1X_center = j1x / CALIB_SAMPLES;
    joy1Y_center = j1y / CALIB_SAMPLES;
    joy2X_center = j2x / CALIB_SAMPLES;
    joy2Y_center = j2y / CALIB_SAMPLES;

    Serial.println(" Done!");
    Serial.printf("  J1 center → X:%d Y:%d\r\n", joy1X_center, joy1Y_center);
    Serial.printf("  J2 center → X:%d Y:%d\r\n", joy2X_center, joy2Y_center);
}

// ================= GYRO CALIBRATION =================
void calibrateGyro() {
    Serial.print("Calibrating gyro (keep still)");

    for (int i = 0; i < 500; i++) {
        mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
        gyroXoffset += gx;
        gyroYoffset += gy;
        gyroZoffset += gz;
        delay(3);
        if (i % 100 == 0) Serial.print(".");
    }
    Serial.println(" Done!");

    gyroXoffset /= 500;
    gyroYoffset /= 500;
    gyroZoffset /= 500;
}

// ================= SETUP =================
void setup() {
    Serial.begin(115200);
    delay(2000);

    // Digital inputs
    pinMode(JOY1_SW, INPUT_PULLUP);
    pinMode(JOY2_SW, INPUT_PULLUP);
    pinMode(BTN1,    INPUT_PULLUP);
    pinMode(BTN2,    INPUT_PULLUP);
    pinMode(BTN3,    INPUT_PULLUP);
    pinMode(BTN4,    INPUT_PULLUP);
    pinMode(BTN5,    INPUT_PULLUP);
    pinMode(BTN6,    INPUT_PULLDOWN);  // GPIO2 — boot safe
    pinMode(TGL1,    INPUT_PULLDOWN);  // GPIO12 — must boot LOW
    pinMode(TGL2,    INPUT_PULLUP);    // GPIO15 — boot safe with PULLUP

    SPI.begin(18,19,23,5);

    // nRF24L01
    if(!radio.begin())
    {
        Serial.println("NRF FAIL");
        while(1);
    }

    radio.openWritingPipe(address);
    radio.setAutoAck(false);
    radio.setDataRate(RF24_250KBPS);
    radio.setPALevel(RF24_PA_LOW);
    radio.setChannel(108);
    radio.stopListening();

    // MPU6050
    Wire.begin(21, 22);
    Serial.println("Initializing MPU6050...");
    mpu.initialize();
    delay(100);

    // Calibrate gyro first (sensor still)
    calibrateGyro();

    // Initial pitch & roll seed
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    AccX = ax / 16384.0;
    AccY = ay / 16384.0;
    AccZ = az / 16384.0;
    pitch_f = atan2(-AccX, sqrt(AccY*AccY + AccZ*AccZ)) * 180.0 / M_PI;
    roll_f  = atan2( AccY, sqrt(AccX*AccX + AccZ*AccZ)) * 180.0 / M_PI;

    // Dynamic joystick calibration
    calibrateJoysticks();

    resetData();

    previousTime = millis();
    Serial.println("System Ready! Transmitting...");
}

// ================= LOOP =================
void loop() {

    // ===== MPU6050 =====
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

    unsigned long currentTime = millis();
    dt = (currentTime - previousTime) / 1000.0;
    previousTime = currentTime;
    if (dt > 0.1) dt = 0.1;

    AccX = ax / 16384.0;
    AccY = ay / 16384.0;
    AccZ = az / 16384.0;

    GyroX = (gx - gyroXoffset) / 131.0;
    GyroY = (gy - gyroYoffset) / 131.0;
    GyroZ = (gz - gyroZoffset) / 131.0;

    float accPitch = atan2(-AccX, sqrt(AccY*AccY + AccZ*AccZ)) * 180.0 / M_PI;
    float accRoll  = atan2( AccY, sqrt(AccX*AccX + AccZ*AccZ)) * 180.0 / M_PI;

    pitch_f = alpha * (pitch_f + GyroY * dt) + (1 - alpha) * accPitch;
    roll_f  = alpha * (roll_f  + GyroX * dt) + (1 - alpha) * accRoll;

    // Map pitch/roll (-90~+90) → byte (0~255), center=127
    data.pitch = (byte)constrain(map((int)pitch_f, -90, 90, 0, 255), 0, 255);
    data.roll  = (byte)constrain(map((int)roll_f,  -90, 90, 0, 255), 0, 255);

    // ===== JOYSTICKS =====
    int j1x = applyDeadzone(map(analogRead(JOY1_X), 0, 4095, 0, 255), joy1X_center);
    int j1y = applyDeadzone(map(analogRead(JOY1_Y), 0, 4095, 0, 255), joy1Y_center);
    int j2x = applyDeadzone(map(analogRead(JOY2_X), 0, 4095, 0, 255), joy2X_center);
    int j2y = applyDeadzone(map(analogRead(JOY2_Y), 0, 4095, 0, 255), joy2Y_center);

    data.joy1_X = mapJoystick(j1x, joy1X_center);
    data.joy1_Y = mapJoystick(j1y, joy1Y_center);
    data.joy2_X = mapJoystick(j2x, joy2X_center);
    data.joy2_Y = mapJoystick(j2y, joy2Y_center);

    // ===== POTENTIOMETERS =====
    data.pot1 = (byte)map(analogRead(POT1), 0, 4095, 0, 255);
    data.pot2 = (byte)map(analogRead(POT2), 0, 4095, 0, 255);

    // ===== DIGITAL INPUTS =====
    data.j1Btn = !digitalRead(JOY1_SW);
    data.j2Btn = !digitalRead(JOY2_SW);
    data.btn1  = !digitalRead(BTN1);
    data.btn2  = !digitalRead(BTN2);
    data.btn3  = !digitalRead(BTN3);
    data.btn4  = !digitalRead(BTN4);
    data.btn5  = !digitalRead(BTN5);
    data.btn6  =  digitalRead(BTN6);   // PULLDOWN — active HIGH
    data.tgl1  =  digitalRead(TGL1);   // PULLDOWN — active HIGH
    data.tgl2  = !digitalRead(TGL2);   // PULLUP   — active LOW

    // ===== TRANSMIT =====
    bool status = radio.write(&data, sizeof(Data_Package));

    // ===== DEBUG (comment out when done) =====
    Serial.printf("J1:%d,%d J2:%d,%d P:%d R:%d POT:%d,%d B:%d%d%d%d%d%d T:%d%d\r\n",
        data.joy1_X, data.joy1_Y,
        data.joy2_X, data.joy2_Y,
        data.pitch,  data.roll,
        data.pot1,   data.pot2,
        data.btn1, data.btn2, data.btn3,
        data.btn4, data.btn5, data.btn6,
        data.tgl1, data.tgl2);

    delay(20);
}