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

    radio.openReadingPipe(0,address);

    radio.startListening();

    Serial.println("RX Ready");
}

void loop()
{
    if(radio.available())
    {
        char text[32]="";

        radio.read(&text,sizeof(text));

        Serial.print("Received: ");
        Serial.println(text);
    }
}