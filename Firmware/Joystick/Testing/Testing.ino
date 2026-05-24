#define JOY1_X  34
#define JOY1_Y  35
#define JOY1_SW 32

#define JOY2_X  33
#define JOY2_Y  25
#define JOY2_SW 26

#define JOY1_X_CENTER 120
#define JOY1_Y_CENTER 115
#define JOY2_X_CENTER 122
#define JOY2_Y_CENTER 116

#define DEADZONE 10

// Fix: return actual center, not 128
int applyDeadzone(int value, int center) {
    if (abs(value - center) < DEADZONE)
        return center;
    return value;
}

// Fix: now receives deadzone-corrected values
String getDirection(int x, int y, int xCenter, int yCenter) {
    int dx = x - xCenter;
    int dy = y - yCenter;

    if (abs(dx) < DEADZONE && abs(dy) < DEADZONE)
        return "CENTER";

    if (abs(dx) > abs(dy))
        return (dx > 0) ? "FORWARD" : "BACK";
    else
        return (dy > 0) ? "RIGHT" : "LEFT";
}

void setup() {
    Serial.begin(115200);

    pinMode(JOY1_SW, INPUT_PULLUP);
    pinMode(JOY2_SW, INPUT_PULLUP);

    Serial.println("System Ready!");
}

void loop() {
    // Raw reads → map to 0-255
    int joy1X = map(analogRead(JOY1_X), 0, 4095, 0, 255);
    int joy1Y = map(analogRead(JOY1_Y), 0, 4095, 0, 255);
    int joy2X = map(analogRead(JOY2_X), 0, 4095, 0, 255);
    int joy2Y = map(analogRead(JOY2_Y), 0, 4095, 0, 255);

    int joy1Btn = !digitalRead(JOY1_SW);
    int joy2Btn = !digitalRead(JOY2_SW);

    // Apply deadzone with correct center
    int j1x = applyDeadzone(joy1X, JOY1_X_CENTER);
    int j1y = applyDeadzone(joy1Y, JOY1_Y_CENTER);
    int j2x = applyDeadzone(joy2X, JOY2_X_CENTER);
    int j2y = applyDeadzone(joy2Y, JOY2_Y_CENTER);

    // Pass corrected values to getDirection
    String dir1 = getDirection(j1x, j1y, JOY1_X_CENTER, JOY1_Y_CENTER);
    String dir2 = getDirection(j2x, j2y, JOY2_X_CENTER, JOY2_Y_CENTER);

    Serial.print("J1X:"); Serial.print(j1x);
    Serial.print(" J1Y:"); Serial.print(j1y);
    Serial.print(" J1Dir:"); Serial.print(dir1);
    Serial.print(" J1Btn:"); Serial.print(joy1Btn);

    Serial.print(" | J2X:"); Serial.print(j2x);
    Serial.print(" J2Y:"); Serial.print(j2y);
    Serial.print(" J2Dir:"); Serial.print(dir2);
    Serial.print(" J2Btn:"); Serial.println(joy2Btn);

    delay(50);
}