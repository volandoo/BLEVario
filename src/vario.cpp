#include "include.h"

#ifdef VARIO_ONLY
#include <Arduino.h>

#define BUZZER_PIN 18
void setup()
{
	Serial.begin(9600);
	delay(1000);
	Serial.println("Starting Device...");
}

void loop()
{
	tone(BUZZER_PIN, 1000);
	delay(500);
	noTone(BUZZER_PIN);
	delay(500);
}

#endif