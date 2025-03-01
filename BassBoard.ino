/*
  BassBoard - DIY MIDI Pedalboard with OLED & MIDI OUT 
  -----------------------------------------------------
  This is a custom-built MIDI pedalboard designed for use with 
  organs, synths, pads, and more. It allows for foot-controlled 
  MIDI note triggering, octave shifting, and expression control.

  Features:
  - Multiplexed foot pedals for triggering MIDI notes.
  - Octave control (Up, Down, Reset).
  - Sustain pedal input.
  - Expression pedal for volume (CC 7) or expression (CC 11).
  - OLED display (SH1106) showing the current octave.
  - MIDI OUT (DIN-5) for external synths, organs, and drum machines.
  - Supports USB MIDI & Serial MIDI OUT simultaneously.
  - Panic Mode sends an ALL NOTES OFF signal when Octave Reset is held for more than 3 seconds.

  MIDI Routing:
  - Notes → MIDI Channel 2
  - Expression, Volume, Sustain → MIDI Channel 1

  Code By: Marc Sexton
  Date: 2025-02-18
*/

#include <MIDIUSB.h>
#include "usb_rename.h"
USBRename dummy = USBRename("BassBoard", "BassBoard", "0001");
#include <Wire.h>
#include <U8g2lib.h>

// OLED Setup (I2C Address: 0x3C)
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// Multiplexer control pins
const int s0 = 7;
const int s1 = 8;
const int s2 = 9;
const int s3 = 10;

// Signal & Control Pins
const int signalPin = 6;
const int sustainPedalPin = 13;
const int expressionPedalPin = A1;
const int expVolTogglePin = 11;
const int expVolLED = 12;

// MIDI settings
const int baseNote = 36;
const int numNotes = 13;
const int octaveUpChannel = 13;
const int octaveDownChannel = 14;
const int octaveResetChannel = 15;
const int minOctave = -2;
const int maxOctave = 2;
const int debounceTime = 50;

int currentOctave = 0;
bool lastButtonStates[16] = { 0 };
bool sustainState = false;
int lastExpressionValue = 0;
bool expVolMode = false;
unsigned long lastDebounceTimes[16] = { 0 };
unsigned long lastExpVolPress = 0;

// Function Prototypes
void sendNoteOn(byte note);
void sendNoteOff(byte note);
void sendMIDIControlChange(byte control, byte value);
void selectChannel(int channel);
void increaseOctave();
void decreaseOctave();
void resetOctave();
void panicReset();
void checkOctaveButton(int channel, void (*action)(), unsigned long currentTime);
void updateOLED();

void setup() {
  Serial.begin(115200);
  Serial1.begin(31250); 

  u8g2.begin();
  updateOLED();

  pinMode(s0, OUTPUT);
  pinMode(s1, OUTPUT);
  pinMode(s2, OUTPUT);
  pinMode(s3, OUTPUT);

  pinMode(signalPin, INPUT_PULLUP);
  pinMode(sustainPedalPin, INPUT_PULLUP);
  pinMode(expVolTogglePin, INPUT_PULLUP);
  pinMode(expVolLED, OUTPUT);
  digitalWrite(expVolLED, LOW);
}

// Check Octave Buttons Function (With Panic Mode)
void checkOctaveButton(int channel, void (*action)(), unsigned long currentTime) {
  static unsigned long holdStartTime = 0;
  static bool isHolding = false;

  selectChannel(channel);
  delayMicroseconds(50);
  bool buttonState = digitalRead(signalPin) == LOW;

  if (buttonState) {
    if (!isHolding) {
      holdStartTime = currentTime;
      isHolding = true;
    } else if (currentTime - holdStartTime >= 3000) {
      panicReset();
      isHolding = false;
    }
  } else {
    if (isHolding && (currentTime - holdStartTime < 3000)) {
      action();
    }
    isHolding = false;
  }
}

void loop() {  
  unsigned long currentTime = millis();

  for (int i = 0; i < numNotes; i++) {
    selectChannel(i);
    delayMicroseconds(50);

    bool buttonState = digitalRead(signalPin) == LOW;

    if (buttonState != lastButtonStates[i] && (currentTime - lastDebounceTimes[i] > debounceTime)) {
      lastDebounceTimes[i] = currentTime;
      lastButtonStates[i] = buttonState;

      int note = baseNote + (currentOctave * 12) + i;

      if (buttonState) {
        sendNoteOn(note);
      } else {
        sendNoteOff(note);
      }
    }
  }

  checkOctaveButton(octaveUpChannel, increaseOctave, currentTime);
  checkOctaveButton(octaveDownChannel, decreaseOctave, currentTime);
  checkOctaveButton(octaveResetChannel, resetOctave, currentTime);

  delay(5);
}

// Octave Functions
void increaseOctave() {
  if (currentOctave < maxOctave) {
    currentOctave++;
    updateOLED();  
  }
}

void decreaseOctave() {
  if (currentOctave > minOctave) {
    currentOctave--;
    updateOLED();  
  }
}

void resetOctave() {
  currentOctave = 0;
  updateOLED();  
}

// MIDI Functions - Notes on Ch 2, CCs on Ch 1
void sendMIDIControlChange(byte control, byte value) {
  midiEventPacket_t cc = { 0x0B, 0xB0, control, value };  // Channel 1
  MidiUSB.sendMIDI(cc);
  MidiUSB.flush();
}

void sendNoteOn(byte note) {
  midiEventPacket_t noteOn = { 0x09, 0x91, note, 127 }; // Channel 2
  MidiUSB.sendMIDI(noteOn);
  MidiUSB.flush();
}

void sendNoteOff(byte note) {
  midiEventPacket_t noteOff = { 0x08, 0x81, note, 0 }; // Channel 2
  MidiUSB.sendMIDI(noteOff);
  MidiUSB.flush();
}

// Panic Reset Function
void panicReset() {
  for (int i = 0; i < 5; i++) {
    digitalWrite(expVolLED, HIGH);
    delay(200);
    digitalWrite(expVolLED, LOW);
    delay(200);
  }

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB18_tr);
  int textWidth = u8g2.getStrWidth("PANIC!!!");
  u8g2.setCursor((128 - textWidth) / 2, 32);
  u8g2.print("PANIC!");
  u8g2.sendBuffer();

  resetOctave();

  for (int channel = 0; channel < 16; channel++) {
    sendMIDIControlChange(123, 0);  
  }

  delay(1000);
  updateOLED();
}

// OLED Update
void updateOLED() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.setCursor(20, 20);
  u8g2.print("Current Octave:");

  u8g2.setFont(u8g2_font_ncenB24_tr);
  u8g2.setCursor(50, 50);
  u8g2.print(currentOctave);

  u8g2.sendBuffer();
}

void selectChannel(int channel) {
  digitalWrite(s0, channel & 1);
  digitalWrite(s1, (channel >> 1) & 1);
  digitalWrite(s2, (channel >> 2) & 1);
  digitalWrite(s3, (channel >> 3) & 1);
}
