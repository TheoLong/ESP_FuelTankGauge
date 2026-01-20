# Configuration Reference

## Document Information

| Field | Value |
|-------|-------|
| Project | ESP_FuelTankGauge |
| Version | 1.0 |
| Created | 2026-01-20 |
| File | `src/config.h` |

---

## Overview

This document defines all configurable parameters for the ESP_FuelTankGauge project. All configuration is centralized in a single header file (`config.h`) for easy modification.

---

## Configuration File: config.h

```cpp
#ifndef CONFIG_H
#define CONFIG_H

/*******************************************************************************
 * ESP_FuelTankGauge Configuration
 * 
 * All user-configurable parameters are defined here.
 * Modify these values to customize the fuel gauge behavior.
 ******************************************************************************/

//==============================================================================
// OPERATING MODE
//==============================================================================
// Select ONE operating mode by setting it to 1, others to 0

#define MODE_NORMAL           1       // Normal operation with real ADC sensors
#define MODE_DEMO             0       // Demo mode: cycles gauge 0-100% without ADC
#define MODE_DEBUG            0       // Debug mode: shows gauge + ADC diagnostics

// Demo mode settings (only used when MODE_DEMO = 1)
#define DEMO_CYCLE_SPEED_MS   150     // Milliseconds per 1% change (slower for observation)
#define DEMO_TANK2_OFFSET     50      // Tank 2 offset from Tank 1 (0-100)

// Debug mode settings (only used when MODE_DEBUG = 1)
#define DEBUG_UPDATE_RATE_MS  200     // How often to update debug info
#define DEBUG_SHOW_RAW_ADC    1       // Show raw ADC value (0-4095)
#define DEBUG_SHOW_VOLTAGE    1       // Show calculated voltage
#define DEBUG_SHOW_RESISTANCE 1       // Show calculated resistance
#define DEBUG_SHOW_PIN        1       // Show GPIO pin number

//==============================================================================
// HARDWARE PINS - LCD (Fixed by hardware, do not change)
//==============================================================================
#define PIN_LCD_MOSI          4       // SPI MOSI
#define PIN_LCD_CLK           5       // SPI Clock
#define PIN_LCD_DC            6       // Data/Command
#define PIN_LCD_CS            7       // Chip Select
#define PIN_LCD_RST           14      // Reset
#define PIN_LCD_BL            15      // Backlight (PWM capable)

//==============================================================================
// HARDWARE PINS - FUEL SENSORS (ADC)
//==============================================================================
#define PIN_TANK1_ADC         0       // GPIO0 = ADC1_CH0
#define PIN_TANK2_ADC         1       // GPIO1 = ADC1_CH1

// ADC Channel mapping (ESP32-C6 specific)
#define ADC_CHANNEL_TANK1     ADC_CHANNEL_0
#define ADC_CHANNEL_TANK2     ADC_CHANNEL_1

//==============================================================================
// VOLTAGE DIVIDER CIRCUIT
//==============================================================================
// Circuit: Vref -> R_reference -> ADC_PIN -> R_sender -> GND
//
//          Vref (DIVIDER_VREF)
//           |
//          [R_ref] (DIVIDER_R_REFERENCE)
//           |
//          ADC -----> To GPIO pin
//           |
//          [R_sender] (fuel tank sender, 33-240 ohms)
//           |
//          GND

#define DIVIDER_VREF          3.3f    // Voltage applied to top of divider (V)
#define DIVIDER_R_REFERENCE   100.0f  // Reference resistor value (ohms)

//==============================================================================
// FUEL SENDER SPECIFICATIONS
//==============================================================================
// Standard automotive fuel sender: 33 ohms (full) to 240 ohms (empty)
// Some senders are reversed - swap these values if your gauge reads backwards

#define SENDER_R_FULL         33.0f   // Resistance when tank is FULL (ohms)
#define SENDER_R_EMPTY        240.0f  // Resistance when tank is EMPTY (ohms)

//==============================================================================
// TANK CAPACITY
//==============================================================================
// Configure tank size for gallon display
#define TANK_CAPACITY_GALLONS 50      // Tank capacity in gallons (displayed as 0-50G)

//==============================================================================
// ADC CONFIGURATION
//==============================================================================
#define ADC_RESOLUTION        12      // ADC resolution in bits (ESP32-C6 = 12-bit)
#define ADC_MAX_VALUE         4095    // Maximum ADC value (2^12 - 1)
#define ADC_VREF              3.3f    // ADC reference voltage
#define ADC_ATTENUATION       ADC_ATTEN_DB_12  // Full 0-3.3V range
#define ADC_SAMPLES           10      // Number of samples to average per reading

//==============================================================================
// SIGNAL FILTERING / SMOOTHING
//==============================================================================
// Exponential Moving Average (EMA) smoothing
// Lower alpha = smoother but slower response
// Higher alpha = faster response but more noise
// Range: 0.0 to 1.0

#define SMOOTHING_ALPHA       0.15f   // EMA smoothing factor
#define MIN_CHANGE_PERCENT    1       // Minimum % change to trigger display update

//==============================================================================
// DISPLAY CONFIGURATION
//==============================================================================
#define SCREEN_WIDTH          170     // LCD width in pixels
#define SCREEN_HEIGHT         320     // LCD height in pixels
#define SCREEN_ROTATION       0       // 0=Portrait, 1=Landscape, 2=Portrait180, 3=Landscape180

// Backlight
#define BACKLIGHT_DEFAULT     255     // Default brightness (0-255)
#define BACKLIGHT_PIN_ACTIVE  LOW     // Backlight active level (LOW or HIGH)

//==============================================================================
// GAUGE LAYOUT - POSITIONING
//==============================================================================
// Screen is divided into two halves for Tank 1 (left) and Tank 2 (right)
// Layout is optimized to maximize bar size with minimal margins

#define LAYOUT_TOP_MARGIN     2       // Pixels from top to gallons text
#define LAYOUT_TEXT_HEIGHT    18      // Height reserved for text (gallons/percentage)
#define LAYOUT_BAR_Y          20      // Y position where bar starts (top_margin + text_height)
#define LAYOUT_BOTTOM_MARGIN  2       // Pixels from percentage text to bottom

// Text positions (calculated dynamically based on bar position)
// Gallons: bar_y - 18 (ABOVE bar)
// Percentage: bar_y + total_bar_height + 3 (BELOW bar)

//==============================================================================
// GAUGE APPEARANCE
//==============================================================================
#define GAUGE_BAR_WIDTH       60      // Width of each bar in pixels
#define GAUGE_BORDER_WIDTH    2       // Border thickness around bars
#define GAUGE_SEGMENTS        20      // Number of visual segments
#define GAUGE_SEGMENT_HEIGHT  13      // Height of each segment in pixels
#define GAUGE_SEGMENT_GAP     1       // Gap between segments in pixels
// Total bar height = SEGMENTS × (HEIGHT + GAP) - GAP = 20 × 14 - 1 = 279 pixels

//==============================================================================
// COLOR THRESHOLDS (Percentage)
//==============================================================================
// Define at what fuel level colors change
// 0% to THRESHOLD_RED_MAX = RED (danger)
// THRESHOLD_RED_MAX to THRESHOLD_YELLOW_MAX = YELLOW (warning)  
// Above THRESHOLD_YELLOW_MAX = GREEN (good)

#define THRESHOLD_RED_MAX     20      // 0-20% = Red zone
#define THRESHOLD_YELLOW_MAX  40      // 20-40% = Yellow zone
                                      // 40-100% = Green zone

//==============================================================================
// COLORS (RGB565 Format)
//==============================================================================
// RGB565: 5 bits red, 6 bits green, 5 bits blue
// Use RGB565() macro to convert from RGB888

// Macro to convert RGB888 to RGB565
#define RGB565(r, g, b) (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3))

// Background and borders
#define COLOR_BACKGROUND      0x0000  // Black
#define COLOR_BORDER          0xFFFF  // White
#define COLOR_SEGMENT_LINE    0x4208  // Dark gray

// Fuel level colors
#define COLOR_ZONE_RED        0xF800  // Pure red (RGB: 255, 0, 0)
#define COLOR_ZONE_YELLOW     0xFFE0  // Pure yellow (RGB: 255, 255, 0)
#define COLOR_ZONE_GREEN      0x07E0  // Pure green (RGB: 0, 255, 0)

// Text colors (can be same as zone colors or different)
#define COLOR_TEXT_RED        0xF800  // Red text
#define COLOR_TEXT_YELLOW     0xFFE0  // Yellow text
#define COLOR_TEXT_GREEN      0x07E0  // Green text

// Debug mode colors
#define COLOR_DEBUG_TEXT      0xFFFF  // White
#define COLOR_DEBUG_LABEL     0x07FF  // Cyan
#define COLOR_DEBUG_VALUE     0xFFE0  // Yellow

//==============================================================================
// TEXT SIZES
//==============================================================================
// Arduino_GFX text sizes: 1=6x8, 2=12x16, 3=18x24, 4=24x32

#define TEXT_SIZE_PERCENT     3       // Size for percentage display (e.g., "75%")
#define TEXT_SIZE_LABEL       1       // Size for tank labels ("T1", "T2")
#define TEXT_SIZE_DEBUG       1       // Size for debug information

//==============================================================================
// TIMING / REFRESH RATES
//==============================================================================
#define MAIN_LOOP_INTERVAL_MS 100     // Main loop update interval
#define DISPLAY_REFRESH_MS    100     // Display update interval
#define SENSOR_READ_MS        50      // Sensor reading interval

//==============================================================================
// TANK LABELS
//==============================================================================
#define TANK1_LABEL           "T1"    // Label for Tank 1
#define TANK2_LABEL           "T2"    // Label for Tank 2

//==============================================================================
// DEBUG MODE LAYOUT
//==============================================================================
// In debug mode, the screen shows both gauges and diagnostic info
// Diagnostic info appears below each gauge

#define DEBUG_INFO_Y          220     // Y position for debug info (inside bar area)
#define DEBUG_LINE_HEIGHT     12      // Pixels between debug text lines

//==============================================================================
// CALCULATED VALUES (Do not modify)
//==============================================================================
#define GAUGE_BAR_HEIGHT      (LAYOUT_BAR_BOTTOM - LAYOUT_BAR_TOP)
#define ZONE_WIDTH            (SCREEN_WIDTH / 2)
#define BAR1_X                ((ZONE_WIDTH - GAUGE_BAR_WIDTH) / 2)
#define BAR2_X                (ZONE_WIDTH + (ZONE_WIDTH - GAUGE_BAR_WIDTH) / 2)
#define SEGMENT_HEIGHT        (GAUGE_BAR_HEIGHT / GAUGE_SEGMENTS)

// Validate mode selection
#if (MODE_NORMAL + MODE_DEMO + MODE_DEBUG) != 1
  #error "Exactly one operating mode must be enabled!"
#endif

#endif // CONFIG_H
```

---

## Operating Modes

### Normal Mode (`MODE_NORMAL = 1`)

Standard operation for actual use:
- Reads ADC values from physical fuel tank senders
- Displays fuel level gauges
- Uses smoothing to reduce noise

### Demo Mode (`MODE_DEMO = 1`)

For testing and demonstration without hardware:
- Does NOT use ADC inputs
- Automatically cycles fuel levels from 0% → 100% → 0%
- Tank 2 can be offset from Tank 1 for visual variety
- Configurable cycle speed

### Debug Mode (`MODE_DEBUG = 1`)

For troubleshooting and calibration:
- Uses real ADC inputs
- Displays fuel level gauges
- **Also shows diagnostic information:**
  - GPIO pin number
  - Raw ADC value (0-4095)
  - Calculated voltage (V)
  - Calculated resistance (Ω)

**Debug Mode Screen Layout:**

```
┌────────────────────────┬────────────────────────┐
│        [ 75% ]         │        [ 42% ]         │
│                        │                        │
│    ┌──────────────┐    │    ┌──────────────┐    │
│    │██████████████│    │    │              │    │
│    │██████████████│    │    │              │    │
│    │██████████████│    │    │              │    │
│    │██████████████│    │    │██████████████│    │
│    │──────────────│    │    │──────────────│    │
│    │ PIN: GPIO0   │    │    │ PIN: GPIO1   │    │
│    │ ADC: 1842    │    │    │ ADC: 2567    │    │
│    │ V: 1.48V     │    │    │ V: 2.07V     │    │
│    │ R: 82Ω       │    │    │ R: 168Ω      │    │
│    │──────────────│    │    │──────────────│    │
│    │▓▓▓▓▓▓▓▓▓▓▓▓▓▓│    │    │▓▓▓▓▓▓▓▓▓▓▓▓▓▓│    │
│    │▒▒▒▒▒▒▒▒▒▒▒▒▒▒│    │    │▒▒▒▒▒▒▒▒▒▒▒▒▒▒│    │
│    └──────────────┘    │    └──────────────┘    │
│          T1            │          T2            │
└────────────────────────┴────────────────────────┘
```

---

## Quick Reference: Common Modifications

### Change Fuel Sender Range

If your sender has different resistance values:

```cpp
#define SENDER_R_FULL         33.0f   // Change to your sender's FULL resistance
#define SENDER_R_EMPTY        240.0f  // Change to your sender's EMPTY resistance
```

### Invert Sender (Reads Backwards)

If your gauge reads 100% when empty, swap the values:

```cpp
#define SENDER_R_FULL         240.0f  // Swapped
#define SENDER_R_EMPTY        33.0f   // Swapped
```

### Change Reference Resistor

If using a different voltage divider resistor:

```cpp
#define DIVIDER_R_REFERENCE   100.0f  // Your resistor value in ohms
```

### Adjust Color Thresholds

To change when colors switch:

```cpp
#define THRESHOLD_RED_MAX     25      // 0-25% = Red (was 20%)
#define THRESHOLD_YELLOW_MAX  50      // 25-50% = Yellow (was 40%)
                                      // 50-100% = Green
```

### Change Colors

To use different colors (RGB565):

```cpp
#define COLOR_ZONE_RED        0xF800  // Standard red
#define COLOR_ZONE_YELLOW     0xFD20  // Orange instead of yellow
#define COLOR_ZONE_GREEN      0x07E0  // Standard green
```

### Adjust Smoothing

For more responsive readings (more noise):
```cpp
#define SMOOTHING_ALPHA       0.3f    // Higher = faster response
```

For smoother readings (slower response):
```cpp
#define SMOOTHING_ALPHA       0.05f   // Lower = smoother
```

### Use Different ADC Pins

If you need to use different GPIO pins:

```cpp
#define PIN_TANK1_ADC         2       // Use GPIO2 instead
#define PIN_TANK2_ADC         3       // Use GPIO3 instead

// Also update channel mapping
#define ADC_CHANNEL_TANK1     ADC_CHANNEL_2
#define ADC_CHANNEL_TANK2     ADC_CHANNEL_3
```

---

## RGB565 Color Reference

Common colors in RGB565 format:

| Color | RGB565 Hex | RGB888 |
|-------|------------|--------|
| Black | `0x0000` | (0, 0, 0) |
| White | `0xFFFF` | (255, 255, 255) |
| Red | `0xF800` | (255, 0, 0) |
| Green | `0x07E0` | (0, 255, 0) |
| Blue | `0x001F` | (0, 0, 255) |
| Yellow | `0xFFE0` | (255, 255, 0) |
| Cyan | `0x07FF` | (0, 255, 255) |
| Magenta | `0xF81F` | (255, 0, 255) |
| Orange | `0xFD20` | (255, 165, 0) |
| Dark Gray | `0x4208` | (66, 66, 66) |
| Light Gray | `0xC618` | (192, 192, 192) |

To calculate custom colors:
```cpp
// Use the RGB565 macro
#define MY_COLOR RGB565(128, 64, 255)  // R=128, G=64, B=255
```

---

## Validation

The configuration includes a compile-time check to ensure exactly one mode is selected:

```cpp
#if (MODE_NORMAL + MODE_DEMO + MODE_DEBUG) != 1
  #error "Exactly one operating mode must be enabled!"
#endif
```

This prevents accidentally enabling multiple modes or no modes.
