#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(4,5);

const byte address[6]="00001";

void setup()
{
    Serial.begin(115200);

    radio.begin();

    radio.setPALevel(RF24_PA_MIN);
    radio.setDataRate(RF24_250KBPS);

    radio.openWritingPipe(address);

    radio.stopListening();

    Serial.println("TX Ready");
}

void loop()
{
    char text[]="HELLO";

    bool ok=radio.write(&text,sizeof(text));

    if(ok)
        Serial.println("Sent OK");
    else
        Serial.println("Send Fail");

    delay(1000);
}