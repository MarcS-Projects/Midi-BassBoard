# BassBoard - DIY MIDI Pedalboard  

### A custom-built MIDI pedalboard for foot-controlled note triggering, octave shifting, and expression control.

---

## 🎹 Features  
- **Multiplexed Foot Pedals** – Trigger MIDI notes seamlessly.  
- **Octave Control** – Shift octaves **up**, **down**, or **reset** instantly.  
- **Sustain Pedal Input** – Hold notes just like a piano pedal.  
- **Expression Pedal** – Control **volume (CC 7)** or **expression (CC 11)** dynamically.  
- **OLED Display (SH1106)** – See the current octave in real time.  
- **MIDI OUT (DIN-5)** – Connect to external synths, organs, and drum machines.  
- **USB MIDI & Serial MIDI OUT** – Use it with both **USB MIDI** devices and traditional **5-pin MIDI**.  
- **Panic Mode** – Hold **Octave Reset** for **3 seconds** to send an **ALL NOTES OFF** signal.  

---

## 🎛️ MIDI Routing  
- 🎵 **MIDI Notes** → **Channel 2**  
- 🎚️ **Expression, Volume, Sustain** → **Channel 1**  

---

## 🛠️ Hardware Requirements  
- **Microcontroller**: Arduino-compatible board with USB MIDI support (e.g., **Arduino Leonardo**, **Pro Micro**, or **Teensy**).  
- **Multiplexer**: For reading multiple foot pedals using fewer pins.  
- **OLED Display**: SH1106 (I2C, Address: **0x3C**).  
- **DIN-5 MIDI Out**: For hardware synths and drum machines.  
- **Sustain Pedal**: Standard sustain pedal input.  
- **Expression Pedal**: TRS-based expression pedal input.  

---

## 🔌 Wiring Overview  
| **Component**        | **Pin** |
|----------------------|--------|
| **Multiplexer** (S0) | **7**  |
| **Multiplexer** (S1) | **8**  |
| **Multiplexer** (S2) | **9**  |
| **Multiplexer** (S3) | **10** |
| **Signal Input**     | **6**  |
| **Sustain Pedal**    | **13** |
| **Expression Pedal** | **A1** |
| **Exp/Vol Toggle**   | **11** |
| **Exp/Vol LED**      | **12** |
| **MIDI OUT TX**      | **Serial1 (TX)** |

---

## 📜 Code Overview  
### 🏗️ Setup  
```cpp
void setup() {
  Serial.begin(115200);  // USB MIDI
  Serial1.begin(31250);  // DIN-5 MIDI Out
  u8g2.begin();          // Initialize OLED
  updateOLED();
}
```
### 🔄 Main Loop  
```cpp
void loop() {  
  unsigned long currentTime = millis();

  // Scan each note input
  for (int i = 0; i < numNotes; i++) {
    selectChannel(i);
    delayMicroseconds(50);
    bool buttonState = digitalRead(signalPin) == LOW;

    if (buttonState != lastButtonStates[i] && (currentTime - lastDebounceTimes[i] > debounceTime)) {
      lastDebounceTimes[i] = currentTime;
      lastButtonStates[i] = buttonState;

      int note = baseNote + (currentOctave * 12) + i;
      buttonState ? sendNoteOn(note) : sendNoteOff(note);
    }
  }

  checkOctaveButton(octaveUpChannel, increaseOctave, currentTime);
  checkOctaveButton(octaveDownChannel, decreaseOctave, currentTime);
  checkOctaveButton(octaveResetChannel, resetOctave, currentTime);

  delay(5);
}
```

### 🎵 MIDI Note Control  
```cpp
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
```

### 🚨 Panic Mode (All Notes Off)  
```cpp
void panicReset() {
  for (int i = 0; i < 5; i++) {
    digitalWrite(expVolLED, HIGH);
    delay(200);
    digitalWrite(expVolLED, LOW);
    delay(200);
  }

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB18_tr);
  u8g2.setCursor(40, 32);
  u8g2.print("PANIC!");
  u8g2.sendBuffer();

  resetOctave();

  for (int channel = 0; channel < 16; channel++) {
    sendMIDIControlChange(123, 0);  // Send "ALL NOTES OFF"
  }

  delay(1000);
  updateOLED();
}
```

---

## 🛠️ Dependencies  
This project uses the following libraries:  
📌 **MIDIUSB** – For USB MIDI communication.  
📌 **U8g2** – To control the SH1106 OLED display.  
📌 **usb_rename** - To customize the name displayed when device is connect via USB.  
📌 **Wire** - For MIDI **DIN-5** output.

---

## 🎛️ How to Use  
1. **Power on the pedalboard** – Connect via USB or external power.  
2. **Press pedals** – Send MIDI note signals via USB and MIDI OUT.  
3. **Use Octave Buttons** – Shift octaves up/down/reset.  
4. **Use Expression Pedal** – Controls either **Volume (CC 7)** or **Expression (CC 11)**.  
5. **Activate Panic Mode** – Hold Octave Reset for **3+ seconds** to stop all notes.  

---

## 🏆 Credits  
🛠️ **Code by:** [Marc Sexton](https://github.com/MarcS-Projects)  
📅 **Date:** 2025-02-18  

---

## 📜 License  
🔓 This project is **open-source** under the MIT License. Use, modify, and improve it freely! 🎶  

---

## 📬 Feedback & Contributions  
🎛️ Found a bug or have an idea? Submit an **issue** or a **pull request** on GitHub! 🚀
