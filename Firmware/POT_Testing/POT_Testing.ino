#define POT1 39

void setup()
{
    Serial.begin(115200);
    delay(2000);

    Serial.println("Pot test started");
}

void loop()
{
    // Read raw ADC (0–4095)
    int raw1 = analogRead(POT1);

    // Convert to 0–255 like joystick values
    int pot1 = map(raw1,0,4095,0,255);

    Serial.print("POT1:");
    Serial.print(pot1);

    delay(1000);
}