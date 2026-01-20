# Configuration Reference

## Document Information

| Field | Value |
|-------|-------|
| Project | ESP_FuelTankGauge |
| Version | 2.0 |
| Updated | 2026-01-20 |

---

## Overview

All user-configurable parameters are centralized in `src/config.h`. This document provides detailed documentation for each configuration option.

---

## 1. Operating Mode

The operating mode can be switched at runtime using the BOOT button (GPIO9). Press the button to cycle through modes: Normal → Demo → Debug → Normal.

```cpp
// Default startup mode (can be changed at runtime via BOOT button)
// 0 = Normal, 1 = Demo, 2 = Debug
#define DEFAULT_MODE          0       // Start in Normal mode

// Mode constants (used internally)
#define MODE_NORMAL           0       // Normal operation with real ADC sensors
#define MODE_DEMO             1       // Demo mode: cycles gauge without ADC
#define MODE_DEBUG            2       // Debug mode: shows gauge + ADC diagnostics
```

### Mode Descriptions

| Mode | Value | Description |
|------|-------|-------------|
| **Normal** | 0 | Production mode - reads real ADC sensors, displays fuel gauges |
| **Demo** | 1 | Demonstration mode - cycles through fuel levels with 5 brightness steps |
| **Debug** | 2 | Diagnostic mode - shows gauge plus ADC values, resistance, voltage overlay |

### Demo Mode Settings

```cpp
#define DEMO_CYCLE_SPEED_MS   150     // Milliseconds per 1% change
#define DEMO_TANK2_OFFSET     50      // Tank 2 offset from Tank 1 (0-100)
```

| Parameter | Default | Description |
|-----------|---------|-------------|
| `DEMO_CYCLE_SPEED_MS` | 150 | Time between 1% fuel level changes |
| `DEMO_TANK2_OFFSET` | 50 | Phase offset for Tank 2 (creates visual difference) |

### Debug Mode Settings

```cpp
#define DEBUG_UPDATE_RATE_MS  200     // How often to update debug info
```

The debug overlay shows:
- Tank 1 & 2: GPIO pin, raw ADC value, voltage, resistance, fuel percentage
- Brightness: ADC raw value, Vpin (ADC voltage), Vin (calculated input voltage), percentage

---

## 2. Hardware Pins

### 2.1 LCD Pins (Fixed by Hardware)

These pins are fixed by the Waveshare ESP32-C6-LCD-1.9 board design:

```cpp
#define PIN_LCD_MOSI          4       // SPI MOSI
#define PIN_LCD_CLK           5       // SPI Clock
#define PIN_LCD_DC            6       // Data/Command
#define PIN_LCD_CS            7       // Chip Select
#define PIN_LCD_RST           14      // Reset
#define PIN_LCD_BL            15      // Backlight (PWM capable)
```

### 2.2 BOOT Button

```cpp
#define PIN_BOOT_BUTTON       9       // BOOT button for mode switching
#define BUTTON_DEBOUNCE_MS    50      // Debounce time in milliseconds
```

### 2.3 Fuel Sensor ADC Pins

```cpp
#define PIN_TANK1_ADC         0       // GPIO0 = ADC1_CH0
#define PIN_TANK2_ADC         1       // GPIO1 = ADC1_CH1
```

### 2.4 Brightness Sensor ADC Pin

```cpp
#define PIN_BRIGHTNESS_ADC    2       // GPIO2 = ADC1_CH2
```

---

## 3. Brightness Auto-Dimming

Automatic brightness control based on external voltage input (e.g., vehicle dimmer circuit).

```cpp
#define BRIGHTNESS_AUTO_ENABLE      0           // 0=Disabled, 1=Enabled
```

### Voltage Divider Circuit

For 0-14V input to 3.3V ADC range:

```
Input Voltage (0-14V) -> [R1 33k] -> ADC_PIN -> [R2 10k] -> GND
```

```cpp
#define BRIGHTNESS_DIVIDER_R1       33000.0f    // Top resistor (ohms)
#define BRIGHTNESS_DIVIDER_R2       10000.0f    // Bottom resistor (ohms)
```

### Voltage to Brightness Mapping

```cpp
#define BRIGHTNESS_VOLTAGE_MIN      0.0f        // Input voltage for minimum brightness
#define BRIGHTNESS_VOLTAGE_MAX      12.0f       // Input voltage for maximum brightness
#define BRIGHTNESS_OUTPUT_MIN       20          // Never fully off (0-255)
#define BRIGHTNESS_OUTPUT_MAX       255         // Maximum brightness
```

### Update Rate

```cpp
#define BRIGHTNESS_UPDATE_MS        500         // ADC read interval (ms)
#define BRIGHTNESS_SAMPLES          5           // Samples to average
```

---

## 4. Fuel Sensor Configuration

### 4.1 Voltage Divider Circuit

```
Vref (3.3V)
    |
   [R_ref] (100 ohms)
    |
   ADC -----> GPIO pin
    |
   [R_sender] (33-240 ohms, fuel sender)
    |
   GND
```

```cpp
#define DIVIDER_VREF          3.3f    // Voltage applied to divider (V)
#define DIVIDER_R_REFERENCE   100.0f  // Reference resistor value (ohms)
```

### 4.2 Fuel Sender Specifications

Standard automotive fuel sender: 33Ω (full) to 240Ω (empty).

```cpp
#define SENDER_R_FULL         33.0f   // Resistance when FULL (ohms)
#define SENDER_R_EMPTY        240.0f  // Resistance when EMPTY (ohms)
```

> **Note:** If your gauge reads backwards, swap these values.

### 4.3 Tank Capacity

```cpp
#define TANK_CAPACITY_GALLONS 50      // Tank capacity for gallon display
```

---

## 5. ADC Configuration

```cpp
#define ADC_RESOLUTION        12      // ESP32-C6 = 12-bit
#define ADC_MAX_VALUE         4095    // Maximum ADC value (2^12 - 1)
#define ADC_VREF              3.3f    // ADC reference voltage
#define ADC_SAMPLES           10      // Samples to average per reading
```

---

## 6. Signal Filtering / Damping

Exponential Moving Average (EMA) smoothing for stable fuel readings:

```cpp
#define FUEL_DAMPING_ENABLE   1         // 1=Enable, 0=Disable
#define FUEL_DAMPING_ALPHA    0.10f     // EMA factor (lower = smoother)
#define MIN_CHANGE_PERCENT    1         // Minimum % change to update display
```

### EMA Alpha Values

| Alpha | Behavior |
|-------|----------|
| 0.05 | Very smooth, slow response (~20 samples to stabilize) |
| 0.10 | Smooth, moderate response (~10 samples to stabilize) |
| 0.20 | Less smooth, faster response (~5 samples to stabilize) |
| 0.50 | Minimal smoothing, fast response |

Formula: `smoothed = alpha * new_value + (1 - alpha) * previous_smoothed`

---

## 7. Display Configuration

```cpp
#define SCREEN_WIDTH          170     // LCD width in pixels
#define SCREEN_HEIGHT         320     // LCD height in pixels
#define SCREEN_ROTATION       0       // 0=Portrait, 1=Landscape, 2=Portrait180

#define BACKLIGHT_ACTIVE_LOW  1       // 1 = LOW turns on backlight
```

---

## 8. Gauge Appearance

```cpp
#define GAUGE_BAR_WIDTH       60      // Width of each bar in pixels
#define GAUGE_BORDER_WIDTH    2       // Border thickness
#define GAUGE_SEGMENTS        20      // Number of visual segments
#define GAUGE_SEGMENT_LINE_W  1       // Segment divider width
```

---

## 9. Color Thresholds

Fuel level zones by percentage:

```cpp
#define THRESHOLD_RED_MAX     20      // 0-20% = Red (danger)
#define THRESHOLD_YELLOW_MAX  40      // 20-40% = Yellow (warning)
                                      // 40-100% = Green (good)
```

---

## 10. Colors (RGB565 Format)

RGB565: 5 bits red, 6 bits green, 5 bits blue.

### Background and Borders

```cpp
#define COLOR_BACKGROUND      0x0000  // Black
#define COLOR_BORDER          0xFFFF  // White
#define COLOR_SEGMENT_LINE    0x39E7  // Medium gray
```

### Fuel Zone Colors

```cpp
#define COLOR_ZONE_RED        0xF800  // Pure red
#define COLOR_ZONE_YELLOW     0xFFE0  // Pure yellow
#define COLOR_ZONE_GREEN      0x07E0  // Pure green
```

### Debug Mode Colors

```cpp
#define COLOR_DEBUG_BG        0x0000  // Black background
#define COLOR_DEBUG_BORDER    0x07FF  // Cyan border
#define COLOR_DEBUG_LABEL     0x07FF  // Cyan labels
#define COLOR_DEBUG_VALUE     0xFFE0  // Yellow values
```

---

## 11. Text Sizes

Arduino_GFX text sizes: 1=6×8, 2=12×16, 3=18×24, 4=24×32 pixels.

```cpp
#define TEXT_SIZE_PERCENT     3       // Percentage display
#define TEXT_SIZE_LABEL       2       // Tank labels
#define TEXT_SIZE_DEBUG       1       // Debug overlay
```

---

## 12. Timing / Refresh Rates

```cpp
#define MAIN_LOOP_INTERVAL_MS 50      // Main loop interval
#define DISPLAY_REFRESH_MS    50      // Display update interval
#define SENSOR_READ_MS        50      // Sensor reading interval
```

---

## 13. Tank Labels

```cpp
#define TANK1_LABEL           "LEFT"  // Label for Tank 1
#define TANK2_LABEL           "RIGHT" // Label for Tank 2
```

---

## 14. Compatibility Aliases

Internal aliases that map user-facing config to module-specific names. These ensure backward compatibility and allow modules to use consistent naming conventions.

```cpp
// LCD Pins
#define LCD_PIN_MOSI          PIN_LCD_MOSI
#define LCD_PIN_CLK           PIN_LCD_CLK
// ... etc.

// ADC Pins
#define ADC_PIN_TANK1         PIN_TANK1_ADC
#define ADC_PIN_TANK2         PIN_TANK2_ADC

// Display dimensions
#define LCD_WIDTH             SCREEN_WIDTH
#define LCD_HEIGHT            SCREEN_HEIGHT
// ... etc.
```

---

## Quick Reference Table

| Setting | Default | Range | Description |
|---------|---------|-------|-------------|
| `DEFAULT_MODE` | 0 | 0-2 | Startup mode (Normal/Demo/Debug) |
| `FUEL_DAMPING_ENABLE` | 1 | 0-1 | Enable EMA smoothing |
| `FUEL_DAMPING_ALPHA` | 0.10 | 0.01-1.0 | Smoothing factor |
| `BRIGHTNESS_AUTO_ENABLE` | 0 | 0-1 | Auto-brightness control |
| `SENDER_R_FULL` | 33Ω | - | Sender resistance at full |
| `SENDER_R_EMPTY` | 240Ω | - | Sender resistance at empty |
| `TANK_CAPACITY_GALLONS` | 50 | - | Tank size for display |
| `THRESHOLD_RED_MAX` | 20% | 0-100 | Red zone upper limit |
| `THRESHOLD_YELLOW_MAX` | 40% | 0-100 | Yellow zone upper limit |
