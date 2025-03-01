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
