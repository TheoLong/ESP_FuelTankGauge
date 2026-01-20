# Deployment Guide

## Document Information

| Field | Value |
|-------|-------|
| Project | ESP_FuelTankGauge |
| Board | Waveshare ESP32-C6-LCD-1.9 |
| Created | 2026-01-20 |

---

## Quick Start

```bash
# Build the firmware
pio run

# Upload to connected device
pio run -t upload

# Monitor serial output
pio device monitor
```

---

## Prerequisites

1. **PlatformIO** installed (VS Code extension or CLI)
2. **USB-C cable** connected to the ESP32-C6 board
3. Board in **normal mode** (not in download mode)

---

## Build & Upload Commands

| Command | Description |
|---------|-------------|
| `pio run` | Compile the firmware |
| `pio run -t upload` | Compile and upload to device |
| `pio run -t clean` | Clean build artifacts |
| `pio device monitor` | Open serial monitor |
| `pio run -t upload && pio device monitor` | Upload then monitor |

---

## Serial Port Detection

PlatformIO auto-detects the serial port. On macOS, it typically appears as:
```
/dev/cu.usbmodem11201
```

If auto-detection fails, specify the port in `platformio.ini`:
```ini
upload_port = /dev/cu.usbmodem11201
monitor_port = /dev/cu.usbmodem11201
```

---

## Critical Configuration Notes

### 1. USB-CDC for Serial Output

The ESP32-C6 uses USB-CDC for serial communication. This **must** be enabled in `platformio.ini`:

```ini
build_flags = 
    -DARDUINO_USB_CDC_ON_BOOT=1
    -DARDUINO_USB_MODE=1
```

**Without this flag, `Serial.print()` will produce no output!**

### 2. Backlight Polarity (Active LOW)

The Waveshare board uses **active LOW** backlight control:
- `GPIO15 = LOW` → Backlight ON
- `GPIO15 = HIGH` → Backlight OFF

This is handled in `display.cpp`:
```cpp
cfg.pin_bl = LCD_PIN_BL;      // GPIO15
cfg.invert = true;            // Invert backlight polarity
cfg.pwm_channel = 7;

// Manual backup to ensure backlight is ON
pinMode(LCD_PIN_BL, OUTPUT);
digitalWrite(LCD_PIN_BL, LOW);
```

### 3. Display Orientation

The display uses rotation 0 (portrait mode):
- Width: 170 pixels
- Height: 320 pixels
- Origin (0,0): Top-left corner

---

## platformio.ini Reference

```ini
[env:esp32c6]
platform = https://github.com/tasmota/platform-espressif32/releases/download/2024.08.10/platform-espressif32.zip
board = esp32-c6-devkitc-1
framework = arduino

monitor_speed = 115200

build_flags = 
    -DARDUINO_USB_CDC_ON_BOOT=1
    -DARDUINO_USB_MODE=1

lib_deps =
    lovyan03/LovyanGFX@^1.2.7
```

---

## Troubleshooting

### No Serial Output

**Symptom:** Upload succeeds but `pio device monitor` shows nothing.

**Solution:** Ensure USB-CDC is enabled:
```ini
build_flags = 
    -DARDUINO_USB_CDC_ON_BOOT=1
```

### Blank Display (Backlight Off)

**Symptom:** Upload succeeds, serial works, but LCD is dark.

**Solution:** The backlight is active LOW. Ensure:
```cpp
digitalWrite(LCD_PIN_BL, LOW);  // LOW = ON for this board
```

### Upload Fails - Port Not Found

**Symptom:** `Looking for upload port... Error: No upload port detected`

**Solutions:**
1. Check USB cable is connected
2. Try a different USB-C cable (some are charge-only)
3. Reset the board by pressing the RESET button
4. Check System Preferences → Security & Privacy for USB permissions

### Upload Fails - Permission Denied

**Symptom:** `Permission denied: '/dev/cu.usbmodem...'`

**Solution (macOS):**
```bash
sudo chmod 666 /dev/cu.usbmodem*
```

### Build Fails - Library Not Found

**Symptom:** `LovyanGFX.h: No such file or directory`

**Solution:**
```bash
pio lib install "lovyan03/LovyanGFX@^1.2.7"
```

---

## Hardware Reset Procedure

If the board becomes unresponsive:

1. **Soft Reset:** Press the RESET button once
2. **Hard Reset:** 
   - Unplug USB
   - Wait 5 seconds
   - Reconnect USB
3. **Download Mode (for recovery):**
   - Hold BOOT button
   - Press and release RESET
   - Release BOOT button
   - Upload firmware
   - Press RESET to exit download mode

---

## Switching Operating Modes

Edit `src/config.h` to change modes:

```cpp
// Demo mode (for testing without sensors)
#define MODE_NORMAL           0
#define MODE_DEMO             1
#define MODE_DEBUG            0

// Normal mode (real sensors)
#define MODE_NORMAL           1
#define MODE_DEMO             0
#define MODE_DEBUG            0
```

After changing, rebuild and upload:
```bash
pio run -t upload
```

---

## Verification Checklist

After uploading, verify:

- [ ] Serial monitor shows boot messages
- [ ] Display backlight is ON
- [ ] Two bar gauges are visible
- [ ] Gallon values shown above bars
- [ ] Percentage values shown below bars
- [ ] Colors correct: Red (bottom), Yellow (middle), Green (top)
- [ ] In demo mode: bars animate smoothly 0-100%
