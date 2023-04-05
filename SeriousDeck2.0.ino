#include "Keyboard.h"
#include <HID.h> 
#include <Control_Surface.h>
#include "keydefs.h"

int inChar=0;
int keyPos=0;
int Row=0;
int i;
int HybridKeyboard=1;
int HybridKey=12;  
int RowPinMap[4] = {5,4,3,2};
int ColPinMap[3] = {8,7,6};

uint16_t keyMap[12]={
  KEY_F13, KEY_F14, KEY_F15, KEY_F16,
  KEY_F17, KEY_F18, KEY_F19, KEY_F20,
  KEY_F21, KEY_F22, KEY_F23, KEY_F24,
};

int keyDown[12];

int keyPosition;
int InputKey;

int debounceDelay=50;
long lastDebounceTime[80];

int sendPressedKey(int Row, int Col){
  keyPosition=Col+(Row*3);
  InputKey = digitalRead(ColPinMap[Col]);
  // LOW = Key pressed, HIGH = Key Not Pressed

  if ((millis()-lastDebounceTime[keyPosition])>debounceDelay){
  // Debounce for each key individually

    if (InputKey == LOW && keyDown[keyPosition]==0){
    // If a key is pressed and wasn't already down
      keyDown[keyPosition]=keyMap[keyPosition];
      lastDebounceTime[keyPosition] = millis(); // Reset the debounce delay
      Keyboard.press(keyDown[keyPosition]);
    }

    if (InputKey == HIGH && keyDown[keyPosition]!=0){
    // If key is up and a character is stored in the keydown position
      lastDebounceTime[keyPosition] = millis(); // Reset keybounce delay
      Keyboard.release(keyDown[keyPosition]);
      keyDown[keyPosition]=0;
    }
  }
}

USBMIDI_Interface midi;

CCPotentiometer potentiometers[] = {
  {A0, 0x10},
  {A1, 0x11},
  {A2, 0x12},
  {A3, 0x13},
};

const int NUM_SLIDERS = 6;
const int analogInputs[NUM_SLIDERS] = {A0, A1, A2, A3};

int analogSliderValues[NUM_SLIDERS];

void setup() {
  Control_Surface.begin();
  Serial.begin(9600);
  Keyboard.begin();

  for (int i=0; i<12; i++) {
    keyDown[i]=0;
  }

  for (int Row=0; Row<4; Row++) {
    pinMode(RowPinMap[Row], INPUT_PULLUP);
  }
  
  for (int Col=0; Col<3; Col++) {
    pinMode(ColPinMap[Col],INPUT_PULLUP);
  }

  for (int i = 0; i < NUM_SLIDERS; i++) {
    pinMode(analogInputs[i], INPUT);
  }

}

void loop() // main keyboard scanning loop
{
  int InputValue; // Replaces digitalread, so that global variable can be removed
  int RowPin;     // Replaces rowPinSet, so that global variable can be removed

  for (int Row=0; Row<4; Row++) // scan through all rows
  {
    RowPin = RowPinMap[Row];  // Map logical row to output pin
    pinMode(RowPin,OUTPUT);   // Set output pin to OUTPUT
    digitalWrite(RowPin,LOW); // Set output pin to LOW
    
    for (int Col=0; Col<3; Col++) // scan through columns
    {
      keyPos=Col+(Row*3); // calculate character map position
      
      inChar=keyMap[keyPos];

      InputValue = digitalRead(ColPinMap[Col]); // LOW = Key pressed, HIGH = Key Not Pressed

      if ((millis()-lastDebounceTime[keyPos])>debounceDelay) // debounce for each key individually
      {
        if ((InputValue == LOW) && keyDown[keyPos]==0) // if a key is pressed and wasn't already down
        {
          keyDown[keyPos]=inChar;        // put the right character in the keydown array
          lastDebounceTime[keyPos] = millis(); // reset the debounce delay
          Keyboard.press(keyDown[keyPos]);    // pass the keypress to windows
        }
        if ((InputValue == HIGH) && keyDown[keyPos]!=0) // key is up and a character is stored in the keydown position
        {
            lastDebounceTime[keyPos] = millis();  // reset keybounce delay
            Keyboard.release(keyDown[keyPos]);    // pass key release to windows
  
          keyDown[keyPos]=0; // set keydown array position as up
        }
      }
    }
    digitalWrite(RowPin,HIGH);    // Set output pin to HIGH
    delay(1);                     // Delay to make sure it has time to go HIGH before switching to INPUT
    pinMode(RowPin,INPUT_PULLUP); // Set output pin back to INPUT with pullup to make sure it stays HIGH
  }
  Control_Surface.loop();
  updateSliderValues();
  sendSliderValues();
}

void updateSliderValues() {
  for (int i = 0; i < NUM_SLIDERS; i++) {
     analogSliderValues[i] = analogRead(analogInputs[i]);
  }
}

void sendSliderValues() {
  String builtString = String("");
  for (int i = 0; i < NUM_SLIDERS; i++) {
    builtString += String((int)analogSliderValues[i]);
    if (i < NUM_SLIDERS - 1) {
      builtString += String("|");
    }
  }
  Serial.println(builtString);
}

void printSliderValues() {
  for (int i = 0; i < NUM_SLIDERS; i++) {
    String printedString = String("Slider #") + String(i + 1) + String(": ") + String(analogSliderValues[i]) + String(" mV");
    Serial.write(printedString.c_str());
    if (i < NUM_SLIDERS - 1) {
      Serial.write(" | ");
    } 
    else {
      Serial.write("\n");
    }
  }
}





