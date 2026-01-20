# Software Architecture

## Document Information

| Field | Value |
|-------|-------|
| Project | ESP_FuelTankGauge |
| Version | 2.0 |
| Updated | 2026-01-20 |

---

## 1. Technology Stack

### 1.1 Development Framework

| Component | Selection | Rationale |
|-----------|-----------|-----------|
| **Framework** | Arduino with ESP32 Core | Simpler API, faster development, good library ecosystem |
| **Build System** | PlatformIO | Better dependency management, VSCode integration |
| **Display Library** | LovyanGFX | Fast, flexible, excellent ST7789 support |
| **Language** | C++ (Arduino dialect) | Standard for Arduino ecosystem |

### 1.2 Why LovyanGFX?

| Criteria | LovyanGFX | Arduino_GFX |
|----------|-----------|-------------|
| Performance | Very fast DMA | Fast |
| ST7789 support | Excellent | Good |
| PWM backlight | Built-in support | Manual |
| Memory footprint | Moderate | Smaller |
| Active development | Yes | Yes |

**Conclusion:** LovyanGFX provides the best combination of speed and features for this project.

### 1.3 Required Libraries

```ini
# platformio.ini lib_deps
lib_deps = 
    lovyan03/LovyanGFX@^1.2.7
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
│   ├── CONFIG_REFERENCE.md
│   └── WAVESHARE_REFERENCE.md
│
├── src/                          # Source code
│   ├── main.cpp                  # Entry point & main loop
│   ├── config.h                  # All configuration parameters
│   │
│   ├── display/                  # Display module
│   │   ├── display.h             # Display interface & LovyanGFX setup
│   │   ├── display.cpp           # Display initialization
│   │   ├── gauge.h               # Gauge drawing interface
│   │   ├── gauge.cpp             # Bar gauge rendering
│   │   ├── brightness.h          # Brightness control interface
│   │   └── brightness.cpp        # Auto-brightness via ADC
│   │
│   ├── sensor/                   # Sensor module
│   │   ├── fuel_sensor.h         # Fuel sensor interface
│   │   └── fuel_sensor.cpp       # ADC reading, conversion, damping
│   │
│   └── modes/                    # Operating modes
│       ├── modes.h               # Mode management interface
│       └── modes.cpp             # Demo, debug, button handling
│
├── test/                         # Unit tests
│   └── test_fuel_gauge.cpp       # Fuel sensor unit tests
│
├── boards/                       # Custom board definitions
│   └── waveshare_esp32c6_lcd.json
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
       ┌───────────────────┼───────────────────┐
       │                   │                   │
       ▼                   ▼                   ▼
┌────────────┐      ┌────────────┐      ┌────────────┐
│  display/  │      │  sensor/   │      │   modes/   │
│   gauge    │      │fuel_sensor │      │   modes    │
└─────┬──────┘      └─────┬──────┘      └─────┬──────┘
      │                   │                   │
      ▼                   │                   │
┌────────────┐            │                   │
│  display/  │            │                   │
│  display   │            │                   │
└─────┬──────┘            │                   │
      │                   │                   │
      ▼                   │                   │
┌────────────┐            │                   │
│  display/  │            │                   │
│ brightness │            │                   │
└─────┬──────┘            │                   │
      │                   │                   │
      ▼                   ▼                   ▼
┌─────────────────────────────────────────────────┐
│                   config.h                       │
│             (All configuration)                  │
└─────────────────────────────────────────────────┘
```

### 3.2 Module Responsibilities

#### config.h
- Centralized configuration constants
- Pin definitions for LCD, ADC, button
- Calibration values (voltage divider, sender resistance)
- Color definitions (RGB565)
- Timing parameters
- Feature flags (FUEL_DAMPING_ENABLE, BRIGHTNESS_AUTO_ENABLE)

#### display/display
- LovyanGFX LCD initialization and configuration
- ST7789 SPI setup with DMA
- PWM backlight control (active LOW)
- Provides display object to other modules

#### display/gauge
- Render segmented bar gauges
- Draw percentage text and labels
- Color zone mapping (red/yellow/green)
- Optimized partial updates

#### display/brightness
- Initialize brightness ADC (GPIO2)
- Read ambient/dimmer voltage
- Map voltage to brightness level
- Auto-update brightness (when enabled)

#### sensor/fuel_sensor
- Initialize ADC for tank sensors (GPIO0, GPIO1)
- Read raw ADC with averaging
- Convert ADC → voltage → resistance → percentage
- EMA damping for stable readings

#### modes/modes
- Runtime mode switching (BOOT button)
- Demo mode: simulated cycling with brightness levels
- Debug mode: diagnostic overlay
- Button debounce handling

---

## 4. Module Interfaces

### 4.1 display/display.h

```cpp
// Initialize the LCD display
bool display_init();

// Get the LovyanGFX display object
LGFX* display_get();

// Clear entire screen with color
void display_clear(uint16_t color);

// Set backlight brightness (0-255)
void display_set_brightness(uint8_t brightness);
```

### 4.2 display/gauge.h

```cpp
// Get RGB565 color for fuel percentage
uint16_t gauge_get_color_for_percent(float percent);

// Calculate filled segments for percentage
int gauge_get_filled_segments(float percent);

// Draw complete fuel gauge bar
void gauge_draw(int16_t x, int16_t y, float percent, int tank_number);

// Draw tank label above gauge
void gauge_draw_label(int16_t x, int16_t y, int tank_number);

// Draw percentage below gauge
void gauge_draw_percentage(int16_t x, int16_t y, float percent);

// Update gauge if value changed
bool gauge_update_if_changed(int16_t x, int16_t y, float old_percent, 
                              float new_percent, int tank_number);
```

### 4.3 display/brightness.h

```cpp
// Initialize brightness control
void brightness_init();

// Update brightness from ADC (rate-limited)
void brightness_update();

// Manual brightness control
void brightness_set(uint8_t level);
uint8_t brightness_get();

// Read raw ADC and voltage (always available for debug)
uint16_t brightness_read_raw();
float brightness_read_voltage();

// Check if auto-brightness enabled
bool brightness_is_auto_enabled();
```

### 4.4 sensor/fuel_sensor.h

```cpp
// Fuel reading structure
typedef struct {
    uint16_t raw_adc;       // Raw ADC value (0-4095)
    float voltage;          // Voltage at ADC pin
    float resistance;       // Sender resistance (ohms)
    float percent;          // Fuel percentage (0-100)
    bool valid;             // Within expected range
} FuelReading;

// Initialize ADC pins
void fuel_sensor_init();

// Read raw ADC value
uint16_t fuel_sensor_read_raw(int tank_number);

// Conversion functions
float fuel_sensor_adc_to_voltage(uint16_t raw_adc);
float fuel_sensor_voltage_to_resistance(float voltage);
float fuel_sensor_resistance_to_percent(float resistance);

// Read with averaging
FuelReading fuel_sensor_read_averaged(int tank_number, int num_samples);

// Read with EMA damping (uses FUEL_DAMPING_ALPHA)
FuelReading fuel_sensor_read_damped(int tank_number, int num_samples);
```

### 4.5 modes/modes.h

```cpp
// Operating modes
typedef enum {
    OP_MODE_NORMAL = 0,   // Real ADC readings
    OP_MODE_DEMO = 1,     // Simulated cycling
    OP_MODE_DEBUG = 2     // ADC + diagnostic overlay
} OperatingMode;

// Mode management
void mode_init();
OperatingMode mode_get_current();
void mode_set(OperatingMode mode);
OperatingMode mode_cycle_next();
const char* mode_get_name(OperatingMode mode);

// Demo mode brightness
int demo_get_brightness_step();
int demo_get_brightness_levels();

// Button handling
void button_init();
bool button_check_press();

// Debug overlay info
int16_t debug_get_overlay_y();
int16_t debug_get_overlay_height();

// Demo mode
void demo_mode_init();
void demo_mode_update(float* tank1_percent, float* tank2_percent);

// Debug overlay
void debug_draw_overlay(float tank1_percent, float tank2_percent);
```

---

## 5. Main Application Flow

### 5.1 Initialization Sequence

```cpp
void setup() {
    Serial.begin(115200);
    
    // 1. Initialize mode system (sets DEFAULT_MODE)
    mode_init();
    
    // 2. Initialize BOOT button for mode switching
    button_init();
    
    // 3. Initialize LCD display
    display_init();
    
    // 4. Initialize fuel sensors (for Normal/Debug modes)
    fuel_sensor_init();
    
    // 5. Initialize brightness control
    brightness_init();
    
    // 6. Initialize demo mode state
    demo_mode_init();
    
    // 7. Calculate gauge layout positions
    // 8. Draw initial gauge state
}
```

### 5.2 Main Loop

```cpp
void loop() {
    // 1. Check for button press → cycle mode
    if (button_check_press()) {
        mode_cycle_next();
        force_redraw = true;
    }
    
    // 2. Update brightness (if auto-enabled)
    brightness_update();
    
    // 3. Rate-limited update
    if (millis() - last_update >= UPDATE_INTERVAL_MS) {
        
        // 4. Get fuel levels based on mode
        switch (mode_get_current()) {
            case OP_MODE_NORMAL:
            case OP_MODE_DEBUG:
                // Read real sensors with damping
                reading1 = fuel_sensor_read_damped(1, ADC_SAMPLES);
                reading2 = fuel_sensor_read_damped(2, ADC_SAMPLES);
                break;
                
            case OP_MODE_DEMO:
                // Get simulated values
                demo_mode_update(&tank1_percent, &tank2_percent);
                break;
        }
        
        // 5. Update gauge displays
        gauge_update_if_changed(...);
        
        // 6. Draw debug overlay (if in debug mode)
        if (mode_get_current() == OP_MODE_DEBUG) {
            debug_draw_overlay(tank1_percent, tank2_percent);
        }
    }
}
```

### 5.3 Mode Cycling

Button press cycles through modes with special demo brightness handling:

```
Normal → Debug → Demo (100%) → Demo (75%) → Demo (50%) → Demo (25%) → Demo (10%) → Normal
```

---

## 6. Key Algorithms

### 6.1 ADC to Fuel Percentage

```
ADC Value (0-4095)
       ↓
Voltage = ADC × (Vref / ADC_MAX)
       ↓
Resistance = R_ref × Voltage / (Vref - Voltage)
       ↓
Percentage = (R_empty - R) / (R_empty - R_full) × 100
       ↓
Clamp to 0-100%
```

### 6.2 EMA Damping

```cpp
smoothed = alpha × new_value + (1 - alpha) × previous_smoothed
```

With `FUEL_DAMPING_ALPHA = 0.10`:
- 10% of new reading + 90% of previous smoothed value
- Provides stable display with ~10 sample lag

### 6.3 Brightness Voltage Divider

```
Vin (0-14V) → [R1 33k] → Vpin → [R2 10k] → GND

Vpin = Vin × R2 / (R1 + R2)
Vin = Vpin × (R1 + R2) / R2
```

---

## 7. Memory Usage

Typical build output:
```
RAM:   9744 bytes (3.0% of 327680 bytes)
Flash: 275KB (16.6% of 1.6MB)
```

---

## 8. Error Handling

- Display init failure: Halt with serial message
- ADC readings: Clamp to valid range
- Invalid resistance: Mark reading as invalid
- All errors logged via Serial

---

## 9. Future Considerations

- OTA firmware updates via WiFi
- Configuration via web interface
- Multiple sender resistance profiles
- CAN bus integration for vehicle data
