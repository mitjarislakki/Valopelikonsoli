/*
  Himmenevä nappi yksinkertaisella debouncealgoritmilla.
*/
#include <Arduino.h>

int redPin = 6;     // pin for the LED
int buttonPin = 13; // pin for button

const int maxBrightness = 255;
int brightness = maxBrightness; // how bright the LED is
const int fadeAmount = 100;     // how many points to fade the LED by

int buttonState = 0;

// Variables will change:
int lastButtonState = LOW; // the previous reading from the input pin

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime = 0; // the last time the output pin was toggled
unsigned long debounceDelay = 50;   // the debounce time; increase if the output flickers

// the setup routine runs once when you press reset:
void setup()
{
    Serial.begin(9600);

    pinMode(redPin, OUTPUT);
    pinMode(buttonPin, INPUT);
}

// the loop routine runs over and over again forever:
void loop()
{

    // Reset on 0

    // if (brightness <= 0) {
    //   brightness = maxBrightness;
    // }

    int reading = digitalRead(buttonPin);

    if (reading != lastButtonState)
    {
        // reset the debouncing timer
        lastDebounceTime = millis();
    }

    // change the brightness for next time through the loop:

    if ((millis() - lastDebounceTime) > debounceDelay)
    {
        // whatever the reading is at, it's been there for longer than the debounce
        // delay, so take it as the actual current state:

        // if the button state has changed:
        if (reading != buttonState)
        {
            buttonState = reading;

            // only toggle the LED if the new button state is HIGH
            if (buttonState == HIGH)
            {
                brightness -= fadeAmount;
                if (brightness < 0)
                    brightness = 0;
            }
        }
    }

    analogWrite(redPin, brightness);

    // Save current button state.
    lastButtonState = reading;
}
