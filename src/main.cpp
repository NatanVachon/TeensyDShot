#include <Arduino.h>
#include "Dshot600.h"

#define PIN_POS1  7
#define PIN_POS2  4
#define PIN_POS3  2
#define PIN_POS4  3
// Real pins: 5, 6, 7, 8

Dshot600 dshot;

uint32_t lastMillis = 0U;
volatile uint32_t isrCounter = 0;

float commands[4] = {0.0f, 0.0f, 50.0f, 0.0f};

void setup()
{
    Serial.begin(9600);

    pinMode(7, OUTPUT);
    pinMode(4, OUTPUT);
    pinMode(2, OUTPUT);
    pinMode(3, OUTPUT);
    dshot.begin(PIN_POS1, PIN_POS2, PIN_POS3, PIN_POS4);

    dshot.setCommand(commands, -1);
}

void loop()
{

}
