#include <Arduino.h>
#include <SoftwareSerial.h>
#include "InputDebounce.h"
#include "DFRobotDFPlayerMini.h"
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif


#define BUTTON_DEBOUNCE_DELAY   20   // [ms]
#define NUMPIXELS 2
#define DATA_PIN 3
#define LIGHT_FRAME_LENGTH 1000 // [ms]


// pin definitions
static const int AUDIO_TRIGGER_PIN = 11;
static const int AUDIO_BUSY_PIN = 4;
static const int LIGHT_TRIGGER_PIN = 12;
int lastAudioTriggerState = HIGH;
int audioTriggerState;
int lastLightTriggerState = HIGH;
int lightTriggerState;

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, DATA_PIN, NEO_RGB + NEO_KHZ800);


// state variables
int selectedAudioFile = -1;
bool lightsActive = false;
unsigned long lastLightChangeTime = 0;

unsigned long lastAudioDebounceTime = 0;
unsigned long lastLightDebounceTime = 0;
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

// MP3 player module
// audio file loopin
DFRobotDFPlayerMini myDFPlayer;
void printDetail(uint8_t type, int value);
SoftwareSerial Serial1(8, 9); // RX, TX
const int numAudioFiles = 3;
const bool audioLoop [] = {true, false, true};

long getRandomAudioFile()
{
  return random(1, numAudioFiles + 1);
}

void setSelectedAudioFile(int audioFile)
{
  selectedAudioFile = audioFile;
}

bool shouldAudioLoop()
{
  return (selectedAudioFile > 0) ? audioLoop[selectedAudioFile -1] : false;
}

bool audioIsPlaying()
{
  return !digitalRead(AUDIO_BUSY_PIN);
}

void startNewLightFrame() {
  lastLightChangeTime = millis();
}

void debouncedButtonReleasedCallback(uint8_t pinIn)
{
  // handle released state
  Serial.print("LOW (pin: ");
  Serial.print(pinIn);
  Serial.println(")");


  if (pinIn == LIGHT_TRIGGER_PIN) {
    lightsActive = !lightsActive;
    if (lightsActive) {
      startNewLightFrame();

    } else {
      // clear it all
    }
    Serial.println(lightsActive ? F("LIGHTS!") : F("No lights.."));

  }
}

void printDetail(uint8_t type, int value){
  switch (type) {
    case TimeOut:
      Serial.println(F("Time Out!"));
      break;
    case WrongStack:
      Serial.println(F("Stack Wrong!"));
      break;
    case DFPlayerCardInserted:
      Serial.println(F("Card Inserted!"));
      break;
    case DFPlayerCardRemoved:
      Serial.println(F("Card Removed!"));
      break;
    case DFPlayerCardOnline:
      Serial.println(F("Card Online!"));
      break;
    case DFPlayerUSBInserted:
      Serial.println("USB Inserted!");
      break;
    case DFPlayerUSBRemoved:
      Serial.println("USB Removed!");
      break;
    case DFPlayerPlayFinished:
      Serial.print(F("Number:"));
      Serial.print(value);
      Serial.println(F(" Play Finished!"));
      break;
    case DFPlayerError:
      Serial.print(F("DFPlayerError:"));
      switch (value) {
        case Busy:
          Serial.println(F("Card not found"));
          break;
        case Sleeping:
          Serial.println(F("Sleeping"));
          break;
        case SerialWrongStack:
          Serial.println(F("Get Wrong Stack"));
          break;
        case CheckSumNotMatch:
          Serial.println(F("Check Sum Not Match"));
          break;
        case FileIndexOut:
          Serial.println(F("File Index Out of Bound"));
          break;
        case FileMismatch:
          Serial.println(F("Cannot Find File"));
          break;
        case Advertise:
          Serial.println(F("In Advertise"));
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }

}

void setup() {

  // init serial
  Serial1.begin(9600);
  Serial.begin(115200);

  Serial.println();
  Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));

  // Use softwareSerial to communicate with mp3.
  if (!myDFPlayer.begin(Serial1)) {
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while(true){
      delay(0); // Code to compatible with ESP8266 watch dog.
    }
  }
  Serial.println(F("DFPlayer Mini online."));

  myDFPlayer.volume(30);  //Set volume value. From 0 to 30
  // myDFPlayer.play(1);  //Play the first mp3

  pinMode(AUDIO_TRIGGER_PIN, INPUT_PULLUP);
  pinMode(LIGHT_TRIGGER_PIN, INPUT_PULLUP);

  // initialise LEDS
  pixels.begin(); // This initializes the NeoPixel library.
  pixels.clear();
  pixels.show();



}

void loop() {
  unsigned long now = millis();

  // read the state of the audio switch into a local variable:
  int audioPinReading = digitalRead(AUDIO_TRIGGER_PIN);

  // If the audio switch changed, due to noise or pressing:
  if (audioPinReading != lastAudioTriggerState) {
    // reset the debouncing timer
    lastAudioDebounceTime = now;
  }

  if ((now - lastAudioDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (audioPinReading != audioTriggerState) {
      audioTriggerState = audioPinReading;

      // If audioTriggerState is LOW, Play or stop audio as required
      if (audioTriggerState == LOW) {
        if (!audioIsPlaying())
        {
          Serial.println("Playing audio");
          setSelectedAudioFile(getRandomAudioFile());
          Serial.print("Selected audio file: ");
          Serial.println(selectedAudioFile);
          Serial.print("Audio file loops? ");
          Serial.println((shouldAudioLoop()) ? "Yes" : "No");
          myDFPlayer.play(selectedAudioFile);
        } else {
          Serial.println("Stopping audio");
          setSelectedAudioFile(0);
          myDFPlayer.stop();
        }
      }
    }
  }
  lastAudioTriggerState = audioPinReading;


    if (myDFPlayer.available()) {
      printDetail(myDFPlayer.readType(), myDFPlayer.read()); //Print the detail message from DFPlayer to handle different errors and states.
    }


  int lightPinReading = digitalRead(LIGHT_TRIGGER_PIN);

  // If the audio switch changed, due to noise or pressing:
  if (lightPinReading != lastLightTriggerState) {
    // reset the debouncing timer
    lastLightDebounceTime = now;
  }

  if ((now - lastLightDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (lightPinReading != lightTriggerState) {
      lightTriggerState = lightPinReading;

      // only toggle the LED if the new button state is HIGH
      if (lightTriggerState == LOW) {
        lightsActive = !lightsActive;
        Serial.println(lightsActive ? F("LIGHTS") : F("NO LIGHTS"));
        if (lightsActive) {
          pixels.setPixelColor(0, pixels.Color(0,255,0)); // Moderately bright green color.
          pixels.setPixelColor(1, pixels.Color(255,0,0));
        } else {
          pixels.clear();
        }
          pixels.show();
      }
    }
  }
  lastLightTriggerState = lightPinReading;

}