#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// ================= nRF24L01 =================
// Arduino: CE=10, CSN=9 (same as reference)
RF24 radio(4, 5);
const byte address[6] = "00001";

// ================= DATA PACKAGE =================
// Must match transmitter EXACTLY
struct Data_Package {
    byte joy1_X;
    byte joy1_Y;
    byte joy2_X;
    byte joy2_Y;
    byte pot1;
    byte pot2;
    byte pitch;
    byte roll;
    byte j1Btn;
    byte j2Btn;
    byte btn1;
    byte btn2;
    byte btn3;
    byte btn4;
    byte btn5;
    byte btn6;
    byte tgl1;
    byte tgl2;
};
Data_Package data;

// ================= CONNECTION TRACKING =================
unsigned long lastReceiveTime = 0;
bool connected = false;

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
    data.btn6   = 0;
    data.tgl1   = 0;
    data.tgl2   = 0;
}

// ================= SETUP =================
void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("=== NRF24L01 Receiver ===");

    if (!radio.begin()) {
        Serial.println("ERROR: NRF24L01 not found! Check wiring.");
        while (1);  // halt
    }

    radio.openReadingPipe(0, address);
    radio.setAutoAck(false);
    radio.setDataRate(RF24_250KBPS);
    radio.setPALevel(RF24_PA_LOW);
    radio.setChannel(108);
    radio.startListening();

    resetData();

    Serial.println("Listening for transmitter...");
}

// ================= PRINT DATA =================
void printData() {
    // Joysticks
    Serial.print("J1X:"); Serial.print(data.joy1_X);
    Serial.print(" J1Y:"); Serial.print(data.joy1_Y);
    Serial.print(" | J2X:"); Serial.print(data.joy2_X);
    Serial.print(" J2Y:"); Serial.print(data.joy2_Y);

    // Potentiometers
    Serial.print(" | POT1:"); Serial.print(data.pot1);
    Serial.print(" POT2:");   Serial.print(data.pot2);

    // MPU6050
    // pitch/roll stored as 0-255, center=127 → convert back to degrees
    int pitch_deg = map(data.pitch, 0, 255, -90, 90);
    int roll_deg  = map(data.roll,  0, 255, -90, 90);
    Serial.print(" | P:"); Serial.print(pitch_deg);
    Serial.print("° R:");  Serial.print(roll_deg);
    Serial.print("°");

    // Joystick buttons
    Serial.print(" | J1B:"); Serial.print(data.j1Btn);
    Serial.print(" J2B:");   Serial.print(data.j2Btn);

    // Push buttons
    Serial.print(" | B1:"); Serial.print(data.btn1);
    Serial.print(" B2:");   Serial.print(data.btn2);
    Serial.print(" B3:");   Serial.print(data.btn3);
    Serial.print(" B4:");   Serial.print(data.btn4);
    Serial.print(" B5:");   Serial.print(data.btn5);
    Serial.print(" B6:");   Serial.print(data.btn6);

    // Toggles
    Serial.print(" | T1:"); Serial.print(data.tgl1);
    Serial.print(" T2:");   Serial.println(data.tgl2);
}

// ================= LOOP =================
void loop() {
    // Check for incoming data
    if (radio.available()) {
        radio.read(&data, sizeof(Data_Package));
        lastReceiveTime = millis();

        if (!connected) {
            connected = true;
            Serial.println("-- Transmitter connected! --");
        }

        printData();
    }

    // Connection lost check — 1 second timeout
    if (connected && (millis() - lastReceiveTime > 1000)) {
        connected = false;
        resetData();
        Serial.println("-- Connection LOST! Resetting data. --");
    }
}