# ESP_FuelTankGauge - Project Specification

## Document Information

| Field | Value |
|-------|-------|
| Project Name | ESP_FuelTankGauge |
| Version | 1.0 |
| Created | 2026-01-20 |
| Author | TheoLong |
| Status | Draft |

---

## 1. Project Overview

### 1.1 Purpose
Build a dual fuel tank level gauge using an ESP32-C6 microcontroller with an integrated LCD display. The system reads fuel levels from two resistive float sender units (33-240Ω) and displays them as visual bar gauges with percentage readouts.

### 1.2 Target Hardware
- **Development Board**: [Waveshare ESP32-C6-LCD-1.9](https://www.waveshare.com/wiki/ESP32-C6-LCD-1.9)
- **Display**: 1.9" ST7789 LCD (170×320 pixels, 262K colors)
- **Fuel Sensors**: 2× Resistive Float Sender Units (33-240Ω range)

### 1.3 Key Features
- Dual tank fuel level monitoring (Tank 1 & Tank 2)
- Visual bar gauge display with color-coded levels (Red/Yellow/Green)
- Percentage readout for each tank
- Configurable voltage divider parameters
- Test mode with simulated cycling fuel levels (0-100%)
- Portrait orientation display

---

## 2. Hardware Requirements

### 2.1 Microcontroller - ESP32-C6-LCD-1.9

| Specification | Value |
|---------------|-------|
| MCU | ESP32-C6FH8 (RISC-V, 160MHz) |
| Flash | 8MB (stacked) |
| SRAM | 512KB HP + 16KB LP |
| ROM | 320KB |
| Wireless | WiFi 6 (2.4GHz), BLE 5, Zigbee 3.0/Thread |
| Display | 1.9" ST7789 LCD, 170×320 pixels |
| Display Interface | SPI |
| ADC | 12-bit SAR ADC, multiple channels |
| Power Input | USB Type-C or 3.7V LiPo battery |

### 2.2 LCD Display Specifications

| Parameter | Value |
|-----------|-------|
| Controller | ST7789 |
| Resolution | 170 × 320 pixels |
| Color Depth | 262K colors (18-bit) |
| Interface | SPI (4-wire) |
| Orientation | Portrait (170W × 320H) |

### 2.3 Fuel Tank Sending Units

| Parameter | Value |
|-----------|-------|
| Type | Resistive Float Sender |
| Resistance Range | 33Ω (Full) to 240Ω (Empty) |
| Quantity | 2 units |
| Measurement Method | Voltage divider with ADC |

### 2.4 Pin Assignments

#### LCD Pins (Fixed - Used by Display)
| Function | GPIO |
|----------|------|
| MOSI | GPIO4 |
| CLK/SCK | GPIO5 |
| DC | GPIO6 |
| CS | GPIO7 |
| RST | GPIO14 |
| Backlight | GPIO15 |

#### ADC Pins for Fuel Sensors (Available)
| Function | GPIO | ADC Channel | Notes |
|----------|------|-------------|-------|
| Tank 1 Sensor | GPIO0 | ADC1_CH0 | Available on expansion header |
| Tank 2 Sensor | GPIO1 | ADC1_CH1 | Available on expansion header |

#### Reserved/Occupied Pins
| Function | GPIO |
|----------|------|
| WS2812 RGB LED | GPIO3 |
| I2C SDA (QMI8658) | GPIO18 |
| I2C SCL (QMI8658) | GPIO8 |
| SD Card MISO | GPIO19 |
| SD Card CS | GPIO20 |

---

## 3. Electrical Design

### 3.1 Voltage Divider Circuit

To measure the resistive fuel sender (33-240Ω), a voltage divider circuit is used with a known reference resistor.

```
                 Vref (Configurable)
                   │
                   ├──────────────┐
                   │              │
                  [R_ref]         │
                   │              │
ADC Input ────────●              │
                   │              │
              [R_sender]          │
              (33-240Ω)           │
                   │              │
                  GND ────────────┘
```

**Formula:**
```
V_adc = Vref × (R_sender / (R_sender + R_ref))
```

**Inverse (Calculate R_sender from ADC reading):**
```
R_sender = R_ref × (V_adc / (Vref - V_adc))
```

### 3.2 Configurable Parameters

| Parameter | Symbol | Default Value | Range | Notes |
|-----------|--------|---------------|-------|-------|
| Reference Voltage | `Vref` | 3.3V | 0-5V | Power supply to divider |
| Reference Resistor | `R_ref` | 100Ω | 50-500Ω | Fixed resistor in divider |
| Sender Empty Resistance | `R_empty` | 240Ω | - | Resistance when tank empty |
| Sender Full Resistance | `R_full` | 33Ω | - | Resistance when tank full |
| ADC Reference Voltage | `V_adc_ref` | 3.3V | - | ESP32-C6 ADC reference |

### 3.3 Recommended R_ref Selection

For optimal ADC resolution across the 33-240Ω range:
- **Recommended R_ref**: 100Ω
- At 33Ω (Full): V_adc ≈ 0.82V (ADC: ~1017)
- At 240Ω (Empty): V_adc ≈ 2.33V (ADC: ~2890)
- This provides ~1873 ADC counts of range (good resolution)

---

## 4. Software Requirements

### 4.1 Development Environment

| Component | Choice | Rationale |
|-----------|--------|-----------|
| Framework | Arduino with ESP32 | Simpler development, good library support |
| IDE | Arduino IDE 2.x or PlatformIO | User preference |
| Display Library | Arduino_GFX_Library | Native support from Waveshare demos, lightweight, direct pixel access |
| Board Package | esp32 by Espressif (≥3.0.7) | Required for ESP32-C6 support |

**Alternative Considered:**
- **LVGL**: More powerful UI framework, but overkill for this simple dual-bar display. Arduino_GFX provides direct pixel drawing which is ideal for custom bar graphs.

### 4.2 Required Libraries

| Library | Version | Purpose | Installation |
|---------|---------|---------|--------------|
| Arduino_GFX_Library | ≥1.5.6 | LCD display driver | Online/Offline |
| esp_adc (ESP-IDF component) | Built-in | ADC calibration & reading | Included with ESP32 core |

### 4.3 Functional Requirements

#### FR-01: Dual Tank Level Display
- Display two fuel tank levels simultaneously on screen
- Each tank occupies half of the screen (left/right split)
- Portrait orientation (170W × 320H)

#### FR-02: Bar Gauge Visualization
- Each tank displays a vertical bar gauge
- Bar is segmented into 20 visual sections for easier readout
- Segments are visual guides only; fill level is continuous (not discrete)
- Bar height represents fuel level (0-100%)

#### FR-03: Color-Coded Levels (Configurable)
| Level Range | Default Color | RGB Value |
|-------------|---------------|-----------|
| 0-20% | Red | 0xFF0000 |
| 20-40% | Yellow | 0xFFFF00 |
| 40-100% | Green | 0x00FF00 |

- Color thresholds are configurable in software
- Percentage text color matches current fuel level color

#### FR-04: Percentage Readout
- Display numeric percentage (0-100%) above each bar
- Text color matches the current level's color band
- Clear, readable font size

#### FR-05: ADC Fuel Sensor Reading
- Read two ADC channels for Tank 1 and Tank 2
- Convert ADC values to resistance, then to percentage
- Apply smoothing/averaging to reduce noise
- Configurable voltage divider parameters

#### FR-06: Operating Modes
Three operating modes are supported, selectable via compile-time configuration:

**Normal Mode (`MODE_NORMAL`)**
- Standard operation with real ADC sensors
- Reads physical fuel tank senders
- Displays fuel level gauges with color-coded levels

**Demo Mode (`MODE_DEMO`)**
- For testing/demonstration without hardware
- Does NOT use ADC inputs
- Automatically cycles fuel levels 0% → 100% → 0%
- Configurable cycle speed and tank offset

**Debug Mode (`MODE_DEBUG`)**
- For troubleshooting and calibration
- Uses real ADC inputs
- Displays fuel gauges PLUS diagnostic overlay:
  - GPIO pin number
  - Raw ADC value (0-4095)
  - Calculated voltage (V)
  - Calculated resistance (Ω)

#### FR-07: Configuration Options
All configurable parameters should be defined in a central config header:
- Voltage divider parameters (Vref, R_ref)
- Sender resistance range (R_full, R_empty)
- Color thresholds for bar segments
- Test mode enable/disable
- Update/refresh rate

---

## 5. User Interface Specification

### 5.1 Screen Layout (Portrait: 170W × 320H)

```
┌─────────────────────────────────────┐ ← 0
│          ┌───────────────────┐      │
│          │   TANK 1  TANK 2  │      │ ← Title area (optional)
│          └───────────────────┘      │
├─────────────────┬───────────────────┤ ← ~30px
│                 │                   │
│    [  75%  ]    │    [  42%  ]      │ ← Percentage display
│                 │                   │
├─────────────────┼───────────────────┤ ← ~60px
│   ┌───────┐     │     ┌───────┐     │
│   │███████│     │     │       │     │  ← Green zone (40-100%)
│   │███████│     │     │       │     │
│   │███████│     │     │       │     │
│   │███████│     │     │███████│     │  ← Yellow zone (20-40%)
│   │       │     │     │███████│     │
│   │       │     │     │███████│     │  ← Red zone (0-20%)
│   └───────┘     │     └───────┘     │
│                 │                   │
└─────────────────┴───────────────────┘ ← 320
0                 85                170
```

### 5.2 Bar Gauge Specifications

| Property | Value | Notes |
|----------|-------|-------|
| Bar Width | ~50px | Centered in each half |
| Bar Height | ~230px | Full height of gauge area |
| Segments | 20 | Visual dividers (11.5px each) |
| Segment Gap | 2px | Thin line between segments |
| Border | 2px white | Outline around each bar |
| Background | Black | Unfilled portion |

### 5.3 Color Scheme

| Element | Color | Hex |
|---------|-------|-----|
| Background | Black | 0x0000 |
| Bar Border | White | 0xFFFF |
| Segment Divider | Dark Gray | 0x4208 |
| Red Zone (0-20%) | Red | 0xF800 |
| Yellow Zone (20-40%) | Yellow | 0xFFE0 |
| Green Zone (40-100%) | Green | 0x07E0 |
| Text | Matches bar level | Variable |

### 5.4 Rendering Approach

Since the UI is simple (two bars + percentages), implement using direct pixel drawing:
- Use `gfx->fillRect()` for bar fills
- Use `gfx->drawRect()` for borders
- Use `gfx->drawFastHLine()` for segment dividers
- Use `gfx->setCursor()` and `gfx->print()` for text
- Minimize full-screen redraws; update only changed regions

---

## 6. Software Architecture

### 6.1 Module Structure

```
src/
├── main.cpp              # Entry point, setup and loop
├── config.h              # All configurable parameters
├── display/
│   ├── display.h         # Display interface
│   ├── display.cpp       # Display initialization
│   ├── gauge.h           # Gauge drawing interface
│   └── gauge.cpp         # Bar gauge rendering logic
├── sensor/
│   ├── fuel_sensor.h     # Fuel sensor interface
│   └── fuel_sensor.cpp   # ADC reading & conversion
└── test/
    ├── test_mode.h       # Test mode interface
    └── test_mode.cpp     # Simulated fuel level cycling
```

### 6.2 Data Flow

```
┌─────────────┐     ┌──────────────┐     ┌─────────────┐
│ ADC Reading │ ──► │  Resistance  │ ──► │ Percentage  │
│ (0-4095)    │     │  Calculation │     │ (0-100%)    │
└─────────────┘     └──────────────┘     └──────────────┘
                                                │
                                                ▼
┌─────────────┐     ┌──────────────┐     ┌─────────────┐
│   Display   │ ◄── │    Gauge     │ ◄── │   Smooth/   │
│   Update    │     │   Render     │     │   Filter    │
└─────────────┘     └──────────────┘     └─────────────┘
```

### 6.3 Main Loop Timing

| Task | Interval | Notes |
|------|----------|-------|
| ADC Read | 100ms | Read both sensors |
| Smoothing | Per read | Exponential moving average |
| Display Update | 100ms | Only if level changed by ≥1% |
| Test Mode Cycle | 50ms | When in test mode |

---

## 7. Configuration Header (config.h)

See [CONFIG_REFERENCE.md](CONFIG_REFERENCE.md) for complete configuration documentation.

```cpp
#ifndef CONFIG_H
#define CONFIG_H

// ============== OPERATING MODE ==============
// Select ONE operating mode by setting it to 1, others to 0
#define MODE_NORMAL           1       // Normal operation with real ADC sensors
#define MODE_DEMO             0       // Demo mode: cycles gauge without ADC
#define MODE_DEBUG            0       // Debug mode: shows gauge + ADC diagnostics

// Demo mode settings
#define DEMO_CYCLE_SPEED_MS   50      // Milliseconds per 1% change
#define DEMO_TANK2_OFFSET     50      // Tank 2 offset from Tank 1 (0-100)

// ============== ADC & VOLTAGE DIVIDER ==============
#define VREF_VOLTAGE          3.3f    // Reference voltage to divider
#define R_REFERENCE           100.0f  // Reference resistor value (ohms)
#define R_SENDER_FULL         33.0f   // Sender resistance at FULL (ohms)
#define R_SENDER_EMPTY        240.0f  // Sender resistance at EMPTY (ohms)

// ============== ADC PINS ==============
#define ADC_PIN_TANK1         0       // GPIO0 - ADC1_CH0
#define ADC_PIN_TANK2         1       // GPIO1 - ADC1_CH1

// ============== DISPLAY ==============
#define SCREEN_WIDTH          170
#define SCREEN_HEIGHT         320
#define SCREEN_ROTATION       0       // Portrait

// ============== GAUGE APPEARANCE ==============
#define GAUGE_SEGMENTS        20      // Number of visual segments
#define GAUGE_BAR_WIDTH       50      // Width of each bar in pixels
#define GAUGE_BAR_MARGIN      10      // Margin from center divider

// ============== COLOR THRESHOLDS (percentage) ==============
#define THRESHOLD_RED         20      // 0% to this = RED
#define THRESHOLD_YELLOW      40      // RED to this = YELLOW
                                      // Above this = GREEN

// ============== COLORS (RGB565) ==============
#define COLOR_BACKGROUND      0x0000  // Black
#define COLOR_BORDER          0xFFFF  // White
#define COLOR_SEGMENT_LINE    0x4208  // Dark gray
#define COLOR_RED             0xF800  // Red
#define COLOR_YELLOW          0xFFE0  // Yellow
#define COLOR_GREEN           0x07E0  // Green

// ============== SMOOTHING ==============
#define SMOOTHING_FACTOR      0.2f    // EMA alpha (0.0-1.0, lower = smoother)
#define UPDATE_INTERVAL_MS    100     // Main loop update interval
#define MIN_CHANGE_PERCENT    1       // Minimum change to trigger redraw

#endif // CONFIG_H
```

---

## 8. Test Plan

### 8.1 Unit Tests

| Test ID | Description | Expected Result |
|---------|-------------|-----------------|
| T-01 | ADC reads 0V (shorted) | Returns ~0 |
| T-02 | ADC reads 3.3V (open) | Returns ~4095 |
| T-03 | Resistance calculation at R=33Ω | Returns ~100% |
| T-04 | Resistance calculation at R=240Ω | Returns ~0% |
| T-05 | Color selection at 10% | Returns RED |
| T-06 | Color selection at 30% | Returns YELLOW |
| T-07 | Color selection at 70% | Returns GREEN |

### 8.2 Integration Tests

| Test ID | Description | Expected Result |
|---------|-------------|-----------------|
| T-10 | Display initialization | Screen shows black background |
| T-11 | Single bar at 50% | Bar fills to middle, green color |
| T-12 | Dual bars different levels | Both bars display correctly |
| T-13 | Test mode cycling | Bars animate 0→100→0 smoothly |

### 8.3 Hardware Tests

| Test ID | Description | Expected Result |
|---------|-------------|-----------------|
| T-20 | Connect 33Ω resistor to Tank 1 | Shows ~100% |
| T-21 | Connect 240Ω resistor to Tank 1 | Shows ~0% |
| T-22 | Connect potentiometer | Level changes smoothly |
| T-23 | Real sender in fuel tank | Accurate level reading |

---

## 9. Milestones & Deliverables

| Milestone | Description | Deliverables |
|-----------|-------------|--------------|
| M1 | Project Setup | PlatformIO project structure, dependencies configured |
| M2 | Display Driver | LCD initialization, basic drawing working |
| M3 | Gauge Rendering | Single bar gauge with colors, segments |
| M4 | Dual Gauge UI | Complete UI with both tanks, percentages |
| M5 | ADC Integration | Read real sensors, convert to percentage |
| M6 | Test Mode | Simulated cycling mode working |
| M7 | Final Integration | Complete working system |

---

## 10. References & Resources

### 10.1 Local Documentation
- [doc/CONFIG_REFERENCE.md](CONFIG_REFERENCE.md) - **Complete configuration reference**
- [doc/HARDWARE.md](HARDWARE.md) - Detailed hardware documentation
- [doc/SOFTWARE_ARCHITECTURE.md](SOFTWARE_ARCHITECTURE.md) - Code architecture details
- [doc/UI_SPECIFICATION.md](UI_SPECIFICATION.md) - Visual design details

### 10.2 Datasheets (Local)
- [ESP32-C6-LCD-1.9-Schematic.pdf](datasheets/ESP32-C6-LCD-1.9-Schematic.pdf)
- [ESP32-C6_Technical_Reference_Manual.pdf](datasheets/ESP32-C6_Technical_Reference_Manual.pdf)
- [ESP32-C6_Series_Datasheet.pdf](datasheets/ESP32-C6_Series_Datasheet.pdf)
- [ST7789T3_Datasheet.pdf](datasheets/ST7789T3_Datasheet.pdf)

### 10.3 Demo Code
- [resources/ESP32-C6-LCD-1.9-Demo-V2/](resources/ESP32-C6-LCD-1.9-Demo-V2/) - Waveshare official examples

### 10.4 External Links
- [Waveshare Wiki](https://www.waveshare.com/wiki/ESP32-C6-LCD-1.9)
- [Arduino_GFX Library](https://github.com/moononournation/Arduino_GFX)
- [ESP32-C6 Arduino Core](https://github.com/espressif/arduino-esp32)

---

## 11. Revision History

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 1.0 | 2026-01-20 | TheoLong | Initial specification |
