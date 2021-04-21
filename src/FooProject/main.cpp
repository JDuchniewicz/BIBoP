#include <Arduino.h>
#include <Wire.h>
#include "FooLib.h"

FooClass FooObject;

// constants won't change. Used here to set a pin number:
const int ledPin =  LED_BUILTIN;// the number of the LED pin

// Variables will change:
int ledState = LOW;             // ledState used to set the LED

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0;        // will store last time LED was updated

// constants won't change:
const long interval = 3000;           // interval at which to blink (milliseconds)

void setup() {

	pinMode(ledPin, OUTPUT);
    Serial.begin(115200);
	delay(1000);

}

void loop() {

	Serial.println("Hello world");
    unsigned long currentMillis = millis();

      if (currentMillis - previousMillis >= interval) {
        // save the last time you blinked the LED
        previousMillis = currentMillis;

        // if the LED is off turn it on and vice-versa:
        if (ledState == LOW) {
          ledState = HIGH;
        } else {
          ledState = LOW;
        }

        // set the LED with the ledState of the variable:
        digitalWrite(ledPin, ledState);
      }
    /*
	FooObject.firstFooMethod();
	delay(1000);
	FooObject.secondFooMethod();
	delay(1000);
    */

}

