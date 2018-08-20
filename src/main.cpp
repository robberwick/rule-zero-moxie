#include <Arduino.h>
#include "InputDebounce.h"
#include "WTV020SD16P.h"

#define BUTTON_DEBOUNCE_DELAY   20   // [ms]

// pin definitions
static const int pinLED = LED_BUILTIN; // 13
static const int AUDIO_TRIGGER_PIN = 12;
static const int wtvResetPin = 2;  // The pin number of the reset pin.
static const int wtvClockPin = 3;  // The pin number of the clock pin.
static const int wtvDataPin = 4;  // The pin number of the data pin.
static const int wtvBusyPin = 5;  // The pin number of the busy pin.

// state variables
bool audioIsOn = false;
int selectedAudioFile = -1;

// Debounced Input Buttons
static InputDebounce audioTriggerButton;

// MP3 player module
static WTV020SD16P wtv020sd16p(wtvClockPin, wtvDataPin, wtvBusyPin);
// audio file looping array. using selectedAudioFile as an index, this array
// determines whether or not the file should be looped
const int numAudioFiles = 3;
const bool audioLoop [] = {true, true, false};

long getRandomAudioFile()
{

  return random(0, numAudioFiles);
}

void setSelectedAudioFile(int audioFile)
{
  selectedAudioFile = audioFile;
}

bool shouldAudioLoop()
{
  return (selectedAudioFile > -1) ? audioLoop[selectedAudioFile] : false;
}

void debouncedButtonPressedCallback(uint8_t pinIn)
{
  // handle pressed state
  digitalWrite(pinLED, HIGH); // turn the LED on
  Serial.print("HIGH (pin: ");
  Serial.print(pinIn);
  Serial.println(")");
}

void debouncedButtonReleasedCallback(uint8_t pinIn)
{
  // handle released state
  digitalWrite(pinLED, LOW); // turn the LED off
  Serial.print("LOW (pin: ");
  Serial.print(pinIn);
  Serial.println(")");

  if (pinIn == AUDIO_TRIGGER_PIN) {
    audioIsOn = !audioIsOn;
    Serial.print("Aduio is ON:");
    Serial.println(audioIsOn);
    // If audio should play, choose a random file and set it as selected
    // else reset to -1
    setSelectedAudioFile((audioIsOn) ? getRandomAudioFile() : -1);
    Serial.print("Selected audio file: ");
    Serial.println(selectedAudioFile);
    Serial.print("Audio file loops? ");
    Serial.println((shouldAudioLoop()) ? "Yes" : "No");
  }
}

void debouncedButtonPressedDurationCallback(uint8_t pinIn, unsigned long duration)
{
  // handle still pressed state
  Serial.print("HIGH (pin: ");
  Serial.print(pinIn);
  Serial.print(") still pressed, duration ");
  Serial.print(duration);
  Serial.println("ms");
}

void debouncedButtonReleasedDurationCallback(uint8_t pinIn, unsigned long duration)
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
  audioTriggerButton.registerCallbacks(debouncedButtonPressedCallback, debouncedButtonReleasedCallback, debouncedButtonPressedDurationCallback, debouncedButtonReleasedDurationCallback);

  // setup input buttons (debounced)
  audioTriggerButton.setup(AUDIO_TRIGGER_PIN, BUTTON_DEBOUNCE_DELAY);

}

void loop() {
    unsigned long now = millis();

    // poll button state
    audioTriggerButton.process(now);

    delay(1); // [ms]

}