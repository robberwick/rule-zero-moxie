#include <Arduino.h>
#include <SoftwareSerial.h>
#include "InputDebounce.h"
#include "DFRobotDFPlayerMini.h"

#define BUTTON_DEBOUNCE_DELAY   20   // [ms]

// pin definitions
static const int pinLED = LED_BUILTIN; // 13
static const int AUDIO_TRIGGER_PIN = 12;

// state variables
bool audioIsOn = false;
int selectedAudioFile = -1;

// Debounced Input Buttons
static InputDebounce audioTriggerButton;
static InputDebounce audioBusyPin;

// MP3 player module
// audio file loopin
DFRobotDFPlayerMini myDFPlayer;
void printDetail(uint8_t type, int value);
SoftwareSerial Serial1(10, 11); // RX, TXg array. using selectedAudioFile as an index, this array
// determines whether or not the file should be looped
const int numAudioFiles = 3;
const bool audioLoop [] = {true, true, false};

long getRandomAudioFile()
{
  return 1;
  // return random(0, numAudioFiles);
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
    if (audioIsOn)
    {
      myDFPlayer.play(selectedAudioFile);
    } else {
      setSelectedAudioFile(-1);
      audioIsOn = false;
      myDFPlayer.stop();
    }
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
    // initialize digital pin as an output
  pinMode(pinLED, OUTPUT);

  // init serial
  Serial1.begin(9600);
  Serial.begin(115200);
  
  Serial.println();
  Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));
  
  //Use softwareSerial to communicate with mp3.
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

  // register callback functions (shared, used by all buttons)
  audioTriggerButton.registerCallbacks(debouncedButtonPressedCallback, debouncedButtonReleasedCallback, debouncedButtonPressedDurationCallback, debouncedButtonReleasedDurationCallback);
  audioBusyPin.registerCallbacks(debouncedButtonPressedCallback, debouncedButtonReleasedCallback, debouncedButtonPressedDurationCallback, debouncedButtonReleasedDurationCallback);

  // setup input buttons (debounced)
  audioTriggerButton.setup(AUDIO_TRIGGER_PIN, BUTTON_DEBOUNCE_DELAY);

}

void loop() {
    unsigned long now = millis();

    // poll button state
    audioTriggerButton.process(now);
    audioBusyPin.process(now);

    if (myDFPlayer.available()) {
    printDetail(myDFPlayer.readType(), myDFPlayer.read()); //Print the detail message from DFPlayer to handle different errors and states.
  }
    delay(1); // [ms]

}