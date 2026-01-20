# Software Architecture

## Document Information

| Field | Value |
|-------|-------|
| Project | ESP_FuelTankGauge |
| Version | 1.0 |
| Created | 2026-01-20 |

---

## 1. Technology Stack

### 1.1 Development Framework

| Component | Selection | Rationale |
|-----------|-----------|-----------|
| **Framework** | Arduino with ESP32 Core | Simpler API, faster development, good library ecosystem |
| **Build System** | PlatformIO | Better dependency management, VSCode integration |
| **Display Library** | Arduino_GFX_Library | Native Waveshare support, lightweight, direct pixel access |
| **Language** | C++ (Arduino dialect) | Standard for Arduino ecosystem |

### 1.2 Why Arduino_GFX over LVGL?

| Criteria | Arduino_GFX | LVGL |
|----------|-------------|------|
| Complexity | Simple, direct drawing | Complex widget system |
| Memory footprint | Small | Large (~100KB+ Flash) |
| Learning curve | Minimal | Significant |
| For simple UI (2 bars) | ✅ Perfect fit | Overkill |
| Custom pixel drawing | ✅ Native | Possible but abstracted |
| Waveshare support | ✅ Demo code available | Requires setup |

**Conclusion:** Arduino_GFX is ideal for this project's simple UI requirements.

### 1.3 Required Libraries

```ini
# platformio.ini lib_deps
lib_deps = 
    moononournation/GFX Library for Arduino@^1.5.6
```

---

## 2. Project Structure

```
ESP_FuelTankGauge/
├── doc/                          # Documentation
│   ├── PROJECT_SPEC.md
│   ├── HARDWARE.md
│   ├── SOFTWARE_ARCHITECTURE.md
│   ├── UI_SPECIFICATION.md
│   ├── datasheets/               # PDF datasheets
│   └── resources/                # Waveshare demo code
│
├── src/                          # Source code
│   ├── main.cpp                  # Entry point
│   ├── config.h                  # All configuration parameters
│   │
│   ├── display/                  # Display module
│   │   ├── display.h             # Display interface
│   │   ├── display.cpp           # Display initialization
│   │   ├── gauge.h               # Gauge drawing interface
│   │   └── gauge.cpp             # Bar gauge rendering
│   │
│   ├── sensor/                   # Sensor module
│   │   ├── fuel_sensor.h         # Fuel sensor interface
│   │   └── fuel_sensor.cpp       # ADC reading & conversion
│   │
│   └── test/                     # Test mode
│       ├── test_mode.h           # Test mode interface
│       └── test_mode.cpp         # Simulated fuel cycling
│
├── include/                      # PlatformIO includes
│   └── README
│
├── lib/                          # Project-specific libraries
│   └── README
│
├── test/                         # Unit tests
│   └── README
│
├── platformio.ini                # Build configuration
└── README.md                     # Project readme
```

---

## 3. Module Design

### 3.1 Module Dependency Diagram

```
                    ┌──────────────┐
                    │   main.cpp   │
                    │  (Entry)     │
                    └──────┬───────┘
                           │
           ┌───────────────┼───────────────┐
           │               │               │
           ▼               ▼               ▼
    ┌────────────┐  ┌────────────┐  ┌────────────┐
    │  display/  │  │  sensor/   │  │   modes/   │
    │   gauge    │  │fuel_sensor │  │ demo/debug │
    └─────┬──────┘  └─────┬──────┘  └─────┬──────┘
          │               │               │
          ▼               │               │
    ┌────────────┐        │               │
    │  display/  │        │               │
    │  display   │        │               │
    └─────┬──────┘        │               │
          │               │               │
          ▼               ▼               ▼
    ┌─────────────────────────────────────────┐
    │              config.h                    │
    │        (All configuration)               │
    └─────────────────────────────────────────┘
```

### 3.2 Module Responsibilities

#### config.h
- Centralized configuration constants
- Pin definitions
- Calibration values
- Color definitions
- Feature flags (e.g., TEST_MODE)

#### display/display
- Initialize ST7789 LCD via SPI
- Control backlight
- Provide display object to other modules
- Screen clearing and basic operations

#### display/gauge
- Render bar gauge graphics
- Draw percentage text
- Handle color selection based on level
- Draw segment lines
- Optimize partial screen updates

#### sensor/fuel_sensor
- Initialize ADC with calibration
- Read ADC values from both channels
- Convert ADC to resistance
- Convert resistance to percentage
- Apply smoothing/filtering

#### modes/demo_mode
- Generate simulated fuel levels
- Cycle between 0-100% for testing without hardware
- Configurable cycle speed and tank offset

#### modes/debug_mode
- Render debug overlay on top of gauge display
- Show GPIO pin number, raw ADC value, voltage, resistance
- Configurable which debug info to display

---

## 4. Detailed Module Specifications

### 4.1 config.h

See [CONFIG_REFERENCE.md](CONFIG_REFERENCE.md) for complete configuration documentation.

```cpp
#ifndef CONFIG_H
#define CONFIG_H

// ============== OPERATING MODE ==============
// Select ONE operating mode by setting it to 1, others to 0
#define MODE_NORMAL           1       // Normal operation with real ADC sensors
#define MODE_DEMO             0       // Demo mode: cycles gauge 0-100% without ADC  
#define MODE_DEBUG            0       // Debug mode: shows gauge + ADC diagnostics

// Demo mode settings
#define DEMO_CYCLE_SPEED_MS   50      // Milliseconds per 1% change
#define DEMO_TANK2_OFFSET     50      // Tank 2 offset from Tank 1

// ============== HARDWARE PINS ==============
// LCD (fixed by hardware)
#define PIN_LCD_MOSI          4
#define PIN_LCD_CLK           5
#define PIN_LCD_DC            6
#define PIN_LCD_CS            7
#define PIN_LCD_RST           14
#define PIN_LCD_BL            15

// ADC for fuel sensors
#define PIN_TANK1             0       // GPIO0 = ADC1_CH0
#define PIN_TANK2             1       // GPIO1 = ADC1_CH1

// ============== VOLTAGE DIVIDER ==============
#define VREF_VOLTAGE          3.3f    // Volts
#define R_REFERENCE           100.0f  // Ohms
#define R_SENDER_FULL         33.0f   // Ohms at full tank
#define R_SENDER_EMPTY        240.0f  // Ohms at empty tank

// ============== DISPLAY ==============
#define SCREEN_WIDTH          170
#define SCREEN_HEIGHT         320
#define SCREEN_ROTATION       0       // 0 = Portrait

// ============== GAUGE LAYOUT ==============
#define GAUGE_TOP_MARGIN      60      // Space for percentage text
#define GAUGE_BOTTOM_MARGIN   10
#define GAUGE_SIDE_MARGIN     15
#define GAUGE_CENTER_GAP      10      // Gap between two gauges
#define GAUGE_SEGMENTS        20      // Visual segment count
#define GAUGE_SEGMENT_GAP     2       // Pixels between segments
#define GAUGE_BORDER_WIDTH    2

// ============== COLOR THRESHOLDS ==============
#define THRESHOLD_RED         20      // 0-20% = Red
#define THRESHOLD_YELLOW      40      // 20-40% = Yellow
                                      // 40-100% = Green

// ============== COLORS (RGB565) ==============
#define COLOR_BACKGROUND      0x0000  // Black
#define COLOR_BORDER          0xFFFF  // White
#define COLOR_SEGMENT_LINE    0x4208  // Dark gray
#define COLOR_RED             0xF800  // Red
#define COLOR_YELLOW          0xFFE0  // Yellow  
#define COLOR_GREEN           0x07E0  // Green
#define COLOR_TEXT_BG         0x0000  // Black (for text background)

// ============== FILTERING ==============
#define SMOOTHING_ALPHA       0.2f    // EMA factor (0-1, lower=smoother)
#define UPDATE_INTERVAL_MS    100     // Main loop interval
#define REDRAW_THRESHOLD      1       // Min % change to redraw

// ============== ADC ==============
#define ADC_SAMPLES           10      // Samples to average per read
#define ADC_ATTENUATION       ADC_ATTEN_DB_12  // Full 0-3.3V range

#endif // CONFIG_H
```

### 4.2 display/display.h

```cpp
#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino_GFX_Library.h>

/**
 * @brief Initialize the LCD display
 * @return true if successful, false otherwise
 */
bool display_init();

/**
 * @brief Get the GFX display object
 * @return Pointer to Arduino_GFX object
 */
Arduino_GFX* display_get_gfx();

/**
 * @brief Clear the entire screen
 * @param color Fill color (RGB565)
 */
void display_clear(uint16_t color);

/**
 * @brief Set backlight brightness
 * @param brightness 0-255 (0=off, 255=full)
 */
void display_set_backlight(uint8_t brightness);

#endif // DISPLAY_H
```

### 4.3 display/gauge.h

```cpp
#ifndef GAUGE_H
#define GAUGE_H

#include <stdint.h>

/**
 * @brief Tank identifier
 */
typedef enum {
    TANK_1 = 0,
    TANK_2 = 1
} tank_id_t;

/**
 * @brief Initialize gauge rendering
 * Call after display_init()
 */
void gauge_init();

/**
 * @brief Draw complete gauge for a tank
 * @param tank Which tank (TANK_1 or TANK_2)
 * @param percent Fuel level 0-100
 */
void gauge_draw(tank_id_t tank, uint8_t percent);

/**
 * @brief Update gauge if level changed significantly
 * @param tank Which tank
 * @param percent New fuel level
 * @return true if gauge was redrawn
 */
bool gauge_update(tank_id_t tank, uint8_t percent);

/**
 * @brief Draw both gauges
 * @param percent1 Tank 1 level
 * @param percent2 Tank 2 level
 */
void gauge_draw_both(uint8_t percent1, uint8_t percent2);

/**
 * @brief Get color for a given percentage
 * @param percent Fuel level 0-100
 * @return RGB565 color
 */
uint16_t gauge_get_color(uint8_t percent);

/**
 * @brief Draw initial screen layout (borders, labels)
 */
void gauge_draw_layout();

#endif // GAUGE_H
```

### 4.4 sensor/fuel_sensor.h

```cpp
#ifndef FUEL_SENSOR_H
#define FUEL_SENSOR_H

#include <stdint.h>
#include "config.h"

/**
 * @brief Initialize ADC for fuel sensor reading
 */
void fuel_sensor_init();

/**
 * @brief Read raw ADC value for a tank
 * @param tank TANK_1 or TANK_2
 * @return Raw ADC value (0-4095)
 */
int fuel_sensor_read_raw(tank_id_t tank);

/**
 * @brief Read fuel level as percentage
 * @param tank TANK_1 or TANK_2
 * @return Fuel percentage 0-100
 */
uint8_t fuel_sensor_read_percent(tank_id_t tank);

/**
 * @brief Get smoothed/filtered fuel level
 * @param tank TANK_1 or TANK_2
 * @return Smoothed percentage 0-100
 */
uint8_t fuel_sensor_get_smoothed(tank_id_t tank);

/**
 * @brief Update sensor readings (call periodically)
 * Reads both sensors and updates smoothed values
 */
void fuel_sensor_update();

/**
 * @brief Convert ADC value to resistance
 * @param adc_value Raw ADC reading
 * @return Calculated resistance in ohms
 */
float fuel_sensor_adc_to_resistance(int adc_value);

/**
 * @brief Convert resistance to fuel percentage
 * @param resistance Sender resistance in ohms
 * @return Fuel percentage 0-100
 */
uint8_t fuel_sensor_resistance_to_percent(float resistance);

#endif // FUEL_SENSOR_H
```

### 4.5 modes/demo_mode.h

```cpp
#ifndef DEMO_MODE_H
#define DEMO_MODE_H

#include <stdint.h>

/**
 * @brief Initialize demo mode
 */
void demo_mode_init();

/**
 * @brief Update demo mode simulation
 * Call periodically to advance simulated levels
 */
void demo_mode_update();

/**
 * @brief Get simulated fuel level for Tank 1
 * @return Simulated percentage 0-100
 */
uint8_t demo_mode_get_tank1();

/**
 * @brief Get simulated fuel level for Tank 2
 * @return Simulated percentage 0-100
 */
uint8_t demo_mode_get_tank2();

#endif // DEMO_MODE_H
```

### 4.6 modes/debug_mode.h

```cpp
#ifndef DEBUG_MODE_H
#define DEBUG_MODE_H

#include <stdint.h>

/**
 * @brief Debug information structure for a tank
 */
typedef struct {
    uint8_t gpio_pin;       // GPIO pin number
    int raw_adc;            // Raw ADC value (0-4095)
    float voltage;          // Calculated voltage (V)
    float resistance;       // Calculated resistance (ohms)
    uint8_t percent;        // Calculated percentage (0-100)
} debug_info_t;

/**
 * @brief Draw debug overlay for a tank
 * @param tank TANK_1 or TANK_2
 * @param info Debug information to display
 */
void debug_mode_draw_overlay(tank_id_t tank, const debug_info_t* info);

/**
 * @brief Clear debug overlay area for a tank
 * @param tank TANK_1 or TANK_2
 */
void debug_mode_clear_overlay(tank_id_t tank);

/**
 * @brief Get debug info for a tank
 * @param tank TANK_1 or TANK_2
 * @param info Pointer to store debug information
 */
void debug_mode_get_info(tank_id_t tank, debug_info_t* info);

#endif // DEBUG_MODE_H
```

---

## 5. Main Application Flow

### 5.1 Initialization Sequence

```cpp
void setup() {
    // 1. Initialize serial for debugging
    Serial.begin(115200);
    
    // 2. Initialize display
    if (!display_init()) {
        Serial.println("Display init failed!");
        while(1);
    }
    
    // 3. Initialize gauge rendering
    gauge_init();
    gauge_draw_layout();
    
    // 4. Initialize based on operating mode
    #if MODE_DEMO
        demo_mode_init();
        Serial.println("Running in DEMO mode");
    #else
        // Both NORMAL and DEBUG modes use real sensors
        fuel_sensor_init();
        #if MODE_DEBUG
            Serial.println("Running in DEBUG mode");
        #else
            Serial.println("Running in NORMAL mode");
        #endif
    #endif
    
    // 5. Draw initial state
    gauge_draw_both(0, 0);
}
```

### 5.2 Main Loop

```cpp
void loop() {
    static uint32_t last_update = 0;
    static uint8_t last_tank1 = 255;
    static uint8_t last_tank2 = 255;
    
    uint32_t now = millis();
    
    // Update at fixed interval
    if (now - last_update >= MAIN_LOOP_INTERVAL_MS) {
        last_update = now;
        
        uint8_t tank1, tank2;
        
        // Get fuel levels based on operating mode
        #if MODE_DEMO
            demo_mode_update();
            tank1 = demo_mode_get_tank1();
            tank2 = demo_mode_get_tank2();
        #else
            // Both NORMAL and DEBUG use real sensors
            fuel_sensor_update();
            tank1 = fuel_sensor_get_smoothed(TANK_1);
            tank2 = fuel_sensor_get_smoothed(TANK_2);
        #endif
        
        // Update display only if changed
        if (abs(tank1 - last_tank1) >= MIN_CHANGE_PERCENT) {
            gauge_update(TANK_1, tank1);
            last_tank1 = tank1;
        }
        
        if (abs(tank2 - last_tank2) >= MIN_CHANGE_PERCENT) {
            gauge_update(TANK_2, tank2);
            last_tank2 = tank2;
        }
        
        // Draw debug overlay in DEBUG mode
        #if MODE_DEBUG
            debug_info_t info1, info2;
            debug_mode_get_info(TANK_1, &info1);
            debug_mode_get_info(TANK_2, &info2);
            debug_mode_draw_overlay(TANK_1, &info1);
            debug_mode_draw_overlay(TANK_2, &info2);
        #endif
    }
}
```

### 5.3 State Diagram

```
                    ┌─────────────┐
                    │   POWER ON  │
                    └──────┬──────┘
                           │
                           ▼
                    ┌─────────────┐
                    │  INIT LCD   │
                    └──────┬──────┘
                           │
                           ▼
                    ┌─────────────┐
                    │ DRAW LAYOUT │
                    └──────┬──────┘
                           │
                           ▼
              ┌────────────┴────────────┐
              │     CHECK MODE          │
              └────────────┬────────────┘
                           │
         ┌─────────────────┼─────────────────┐
         │                 │                 │
         ▼                 ▼                 ▼
   ┌──────────┐     ┌──────────┐     ┌──────────┐
   │  NORMAL  │     │   DEMO   │     │  DEBUG   │
   │   MODE   │     │   MODE   │     │   MODE   │
   └────┬─────┘     └────┬─────┘     └────┬─────┘
        │                │                │
        ▼                ▼                ▼
   ┌────────────────────────────────────────────┐
   │                MAIN LOOP                   │◄──┐
   ├────────────────────────────────────────────┤   │
   │ NORMAL: Read ADC → Update gauges           │   │
   │ DEMO:   Cycle values → Update gauges       │   │
   │ DEBUG:  Read ADC → Update gauges + overlay │───┘
   └────────────────────────────────────────────┘
              │  2. Apply smoothing    │         │
              │  3. Check for change   │         │
              │  4. Update display     │         │
              │  5. Wait interval      │─────────┘
              │                        │
              └────────────────────────┘
```

---

## 6. Algorithm Details

### 6.1 ADC to Fuel Percentage Conversion

```cpp
/**
 * Convert ADC reading to fuel percentage
 * 
 * 1. ADC to Voltage:
 *    V = ADC * 3.3 / 4096
 * 
 * 2. Voltage to Resistance (voltage divider):
 *    R_sender = R_ref * V / (Vref - V)
 * 
 * 3. Resistance to Percentage (linear interpolation):
 *    percent = 100 * (R_empty - R_sender) / (R_empty - R_full)
 */

float adc_to_voltage(int adc_value) {
    return (float)adc_value * 3.3f / 4096.0f;
}

float voltage_to_resistance(float voltage) {
    if (voltage >= VREF_VOLTAGE) return 999999.0f;  // Open circuit
    if (voltage <= 0) return 0.0f;                   // Short circuit
    return R_REFERENCE * voltage / (VREF_VOLTAGE - voltage);
}

uint8_t resistance_to_percent(float resistance) {
    // Clamp to valid range
    if (resistance <= R_SENDER_FULL) return 100;
    if (resistance >= R_SENDER_EMPTY) return 0;
    
    // Linear interpolation
    float range = R_SENDER_EMPTY - R_SENDER_FULL;
    float position = R_SENDER_EMPTY - resistance;
    float percent = 100.0f * position / range;
    
    return (uint8_t)constrain(percent, 0, 100);
}
```

### 6.2 Exponential Moving Average (EMA) Smoothing

```cpp
/**
 * EMA smoothing reduces noise while maintaining responsiveness
 * 
 * smoothed = alpha * new_value + (1 - alpha) * old_smoothed
 * 
 * Lower alpha = smoother but slower response
 * Higher alpha = faster response but more noise
 */

float smoothed_tank1 = 50.0f;  // Initial value
float smoothed_tank2 = 50.0f;

void update_smoothed(uint8_t raw1, uint8_t raw2) {
    smoothed_tank1 = SMOOTHING_ALPHA * raw1 + (1.0f - SMOOTHING_ALPHA) * smoothed_tank1;
    smoothed_tank2 = SMOOTHING_ALPHA * raw2 + (1.0f - SMOOTHING_ALPHA) * smoothed_tank2;
}

uint8_t get_smoothed_tank1() {
    return (uint8_t)round(smoothed_tank1);
}
```

### 6.3 Bar Fill Algorithm

```cpp
/**
 * Draw bar fill with color zones
 * 
 * Bar is divided into segments, but fill is continuous.
 * Color changes based on fill level, not segment.
 */

void draw_bar_fill(int x, int y, int width, int height, uint8_t percent) {
    int fill_height = height * percent / 100;
    int fill_top = y + height - fill_height;
    
    // Draw filled portion with appropriate colors
    for (int py = fill_top; py < y + height; py++) {
        // Calculate percentage at this y position
        int local_percent = 100 - ((py - y) * 100 / height);
        uint16_t color = get_color_for_percent(local_percent);
        
        gfx->drawFastHLine(x, py, width, color);
    }
    
    // Draw empty portion (background)
    gfx->fillRect(x, y, width, height - fill_height, COLOR_BACKGROUND);
    
    // Draw segment lines
    int segment_height = height / GAUGE_SEGMENTS;
    for (int i = 1; i < GAUGE_SEGMENTS; i++) {
        int line_y = y + i * segment_height;
        gfx->drawFastHLine(x, line_y, width, COLOR_SEGMENT_LINE);
    }
}
```

---

## 7. Memory Considerations

### 7.1 Estimated Memory Usage

| Component | Flash | RAM | Notes |
|-----------|-------|-----|-------|
| Arduino core | ~200KB | ~40KB | ESP32 base |
| Arduino_GFX | ~50KB | ~5KB | Display library |
| Application code | ~20KB | ~2KB | Our code |
| Frame buffer | 0 | 0 | No double buffering |
| **Total** | ~270KB | ~47KB | Well within 8MB/512KB |

### 7.2 Optimization Strategies

1. **No frame buffer**: Draw directly to display
2. **Partial updates**: Only redraw changed bars
3. **Integer math**: Avoid floating point where possible
4. **Const strings**: Use PROGMEM for static strings

---

## 8. Error Handling

### 8.1 Error Conditions

| Error | Detection | Response |
|-------|-----------|----------|
| Display init fail | `gfx->begin()` returns false | Halt with serial message |
| ADC read fail | Timeout or invalid value | Use last valid reading |
| Sensor disconnected | Resistance out of range | Show "---" or warning |
| Out of range | Percent < 0 or > 100 | Clamp to valid range |

### 8.2 Watchdog

Consider enabling ESP32 watchdog timer to recover from hangs:

```cpp
#include "esp_task_wdt.h"

void setup() {
    esp_task_wdt_init(10, true);  // 10 second timeout, panic reset
    esp_task_wdt_add(NULL);       // Add current task
}

void loop() {
    esp_task_wdt_reset();         // Feed watchdog
    // ... rest of loop
}
```

---

## 9. PlatformIO Configuration

### 9.1 platformio.ini

```ini
[env:esp32-c6-devkitc-1]
platform = espressif32
board = esp32-c6-devkitc-1
framework = arduino

; Monitor settings
monitor_speed = 115200

; Build flags
build_flags = 
    -DARDUINO_USB_CDC_ON_BOOT=1
    -DCORE_DEBUG_LEVEL=3

; Libraries
lib_deps = 
    moononournation/GFX Library for Arduino@^1.5.6

; Board-specific settings
board_build.flash_mode = dio
board_build.flash_size = 8MB
board_build.arduino.memory_type = qio_opi
```

---

## 10. Testing Strategy

### 10.1 Unit Testing

Test individual functions in isolation:

| Function | Test Cases |
|----------|------------|
| `adc_to_voltage()` | 0, 2048, 4095 |
| `voltage_to_resistance()` | 0V, 1.65V, 3.3V, edge cases |
| `resistance_to_percent()` | 33Ω, 137Ω, 240Ω, out of range |
| `get_color_for_percent()` | 10%, 30%, 70%, edge cases |

### 10.2 Integration Testing

Test module combinations:

1. **Sensor + Display**: Read real sensors, verify display
2. **Test Mode + Display**: Verify cycling animation
3. **Full System**: End-to-end with potentiometers

### 10.3 Test Mode Usage

Enable test mode in `config.h`:

```cpp
#define TEST_MODE_ENABLED     true
#define TEST_CYCLE_SPEED_MS   50   // Cycle every 50ms
```

Test mode behavior:
- Tank 1: Cycles 0% → 100% → 0%
- Tank 2: Same cycle, offset by 50% (opposite phase)
- Allows visual verification without sensors

---

## 11. Future Enhancements

### 11.1 Potential Features

| Feature | Effort | Value |
|---------|--------|-------|
| WiFi web dashboard | Medium | High |
| Low fuel alerts (LED/buzzer) | Low | Medium |
| Data logging to SD card | Medium | Medium |
| Bluetooth configuration | Medium | Low |
| Multiple calibration profiles | Low | Medium |
| Sleep mode for battery | Medium | High (if battery powered) |

### 11.2 Extension Points

The modular design allows easy extension:
- Add new sensor types in `sensor/`
- Add web server in `network/`
- Add alerts in `alerts/`
- Modify UI in `display/gauge.cpp`
