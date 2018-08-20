#include <Arduino.h>
#include "InputDebounce.h"

#define BUTTON_DEBOUNCE_DELAY   20   // [ms]
static const int pinLED = LED_BUILTIN; // 13
static const int AUDIO_TRIGGER_PIN = 12;

// Debounced Input Buttons
static InputDebounce buttonTestA;



void buttonTest_pressedCallback(uint8_t pinIn)
{
  // handle pressed state
  digitalWrite(pinLED, HIGH); // turn the LED on
  Serial.print("HIGH (pin: ");
  Serial.print(pinIn);
  Serial.println(")");
}

void buttonTest_releasedCallback(uint8_t pinIn)
{
  // handle released state
  digitalWrite(pinLED, LOW); // turn the LED off
  Serial.print("LOW (pin: ");
  Serial.print(pinIn);
  Serial.println(")");
}

void buttonTest_pressedDurationCallback(uint8_t pinIn, unsigned long duration)
{
  // handle still pressed state
  Serial.print("HIGH (pin: ");
  Serial.print(pinIn);
  Serial.print(") still pressed, duration ");
  Serial.print(duration);
  Serial.println("ms");
}

void buttonTest_releasedDurationCallback(uint8_t pinIn, unsigned long duration)
{
  // handle released state
  Serial.print("LOW (pin: ");
  Serial.print(pinIn);
  Serial.print("), duration ");
  Serial.print(duration);
  Serial.println("ms");
}

void setup() {
    // initialize digital pin as an output
  pinMode(pinLED, OUTPUT);

  // init serial
  Serial.begin(9600);

  Serial.println("Test InputDebounce library, using callback functions");

  // register callback functions (shared, used by all buttons)
  buttonTestA.registerCallbacks(buttonTest_pressedCallback, buttonTest_releasedCallback, buttonTest_pressedDurationCallback, buttonTest_releasedDurationCallback);

  // setup input buttons (debounced)
  buttonTestA.setup(AUDIO_TRIGGER_PIN, BUTTON_DEBOUNCE_DELAY);

}

void loop() {
    unsigned long now = millis();

    // poll button state
    buttonTestA.process(now); // callbacks called in context of this function

    // delay(1); // [ms]

}