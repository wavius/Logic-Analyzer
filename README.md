# Logic Analyzer (DE1-SoC, Nios V)

Logic Analyzer that samples digital signals on 16 channels from the GPIO expansion header. It polls the input pins to capture signal data and stores it in a circular buffer. The signals are plotted on the VGA and the display is controlled with a PS2 keyboard. Features include adjustable time division and vertical scale, time measurement cursor, and selectable trigger channel. The channel capture logic was implemented on the FPGA hardware and the display logic runs on the Nios V processor.


## Overview

### UI Display (Top Status Bar)
- Time per division (ns/div)
- Sampling frequency (MHz)
- Number of visible samples
- Time offset from trigger (ns)

### Signal Display
- 16 total channels
- 8 channels displayed per page
- Page toggled using `Tab`

### Interaction
- Fully keyboard-controlled (PS/2 interface)


## Controls

### Channel Selection
- Active channel shown on HEX display (0–7 per page)
- No-selection state:
  - Triggered by `Esc` or navigating off-screen
  - HEX display cleared

### Navigation
- `↑ / ↓` → move between channels  
- `0–7` → direct channel selection  

### Channel Control
- `Space` or `E` → enable/disable selected channel  


## Control Functions
- `S` → start acquisition + waveform display  
- `T` → set trigger channel (current selection)  
- `C` → clear all signals + reset analyzer  
- `Esc` → deselect channel  
- `Tab` → toggle between channel pages (0–7, 8–15)  


## Zoom & Time Scaling

- Controlled using `+` and `-`
- Discrete zoom levels (samples): {64, 96, 256, 512, 1024}
- Time/div is dynamically computed from:
  - Sampling frequency
  - Visible sample window


## Time Axis

- Horizontal time scale rendered across screen  
  - Range: [0, 39,000] ns
  - Derived from buffer size and sampling rate  


---

## Hardware Interface (Parallel Port)

The logic analyzer communicates via memory-mapped registers on the DE1-SoC.

### Register Map

| Address     | Register         | Description |
|------------|------------------|-------------|
| 0xFF205000 | Control          | Start/stop acquisition |
| 0xFF205004 | Status           | RUN, FULL, TRIG flags |
| 0xFF205008 | Trigger Config   | Trigger channel |
| 0xFF20500C | Buffer Window    | Data read interface |
| 0xFF205010 | Trigger Pointer  | Trigger index |
| 0xFF205014 | Samples          | Pre/Post trigger counts |

### Register Behavior

**Control**
- Write `1` → start sampling (RUN)
- Write `0` after FULL → reset to idle

**Status**
- `RUN` → high during sampling  
- `FULL` → buffer filled  
- `TRIG` → trigger condition detected  

**Trigger Config**
- 16-bit value specifying trigger channel  

**Buffer Window**
- Read: returns current sample  
- Read advances internal pointer  
- Write: resets read pointer  

**Trigger Pointer**
- Read-only index where trigger occurred  

**Samples**
- Upper 16 bits → post-trigger samples  
- Lower 16 bits → pre-trigger samples  


## Other Notes
- Uses VGA frame buffer + character buffer for rendering  
- Modular design (UI, logic, hardware separated)
