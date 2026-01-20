# Waveshare ESP32-C6-LCD-1.9 Reference

## Document Information

| Field | Value |
|-------|-------|
| Source | [Waveshare Wiki](https://www.waveshare.com/wiki/ESP32-C6-LCD-1.9) |
| Retrieved | 2026-01-20 |
| Product SKU | 30937 (Non-touch), 30941 (Touch) |

---

## 1. Product Overview

The ESP32-C6-LCD-1.9 is a compact development board featuring the ESP32-C6 microcontroller with an integrated 1.9" LCD display.

### Key Features

- ESP32-C6 high-performance 32-bit RISC-V processor, up to 160MHz
- Integrated WiFi 6, Bluetooth 5 and IEEE 802.15.4 (Zigbee 3.0 and Thread)
- Built-in 512KB HP SRAM, 16KB LP SRAM, 320KB ROM, stacked 8MB Flash
- Onboard 1.9" LCD screen (170×320 resolution, 262K colors)
- Onboard QMI8658 6-axis IMU (3-axis accelerometer + 3-axis gyroscope)
- Onboard USB Type-C port for power and programming
- Onboard 3.7V MX1.25 lithium battery interface
- Onboard TF card slot for external storage
- Onboard Pico-compatible interface for expandability
- WS2812 RGB LED (non-touch version only)

---

## 2. Onboard Components

| # | Component | Description |
|---|-----------|-------------|
| 1 | ESP32-C6FH8 | Main MCU, stacked 8MB Flash, up to 160MHz |
| 2 | TCA9554 | I2C GPIO expander (8-channel) |
| 3 | MX1.25 Battery | 3.7V LiPo battery connector |
| 4 | RESET Button | System reset |
| 5 | USB Type-C | Power input and data/programming |
| 6 | TF Card Slot | MicroSD card (on back) |
| 7 | Charge LED | Turns off when fully charged |
| 8 | BOOT Button | For firmware download mode |
| 9 | WS2812 | RGB LED (non-touch version, on back) |
| 10 | QMI8658 | 6-axis IMU (on back) |
| 11 | IPEX1 | External antenna (touch version, requires resoldering) |
| 12 | Ceramic Antenna | Onboard 2.4GHz antenna |

---

## 3. Pin Definitions

### 3.1 LCD Display Pins

| Function | GPIO | Notes |
|----------|------|-------|
| MOSI | GPIO4 | SPI data |
| CLK/SCK | GPIO5 | SPI clock |
| DC | GPIO6 | Data/Command |
| CS | GPIO7 | Chip select |
| RST | GPIO14 | Reset |
| BL | GPIO15 | Backlight (PWM) |

### 3.2 I2C Pins

| Function | GPIO | Notes |
|----------|------|-------|
| SDA | GPIO18 | QMI8658, Touch (CST816) |
| SCL | GPIO8 | QMI8658, Touch (CST816) |

### 3.3 SD Card Pins

| Function | GPIO | Notes |
|----------|------|-------|
| MISO | GPIO19 | SPI data in |
| CS | GPIO20 | Chip select |
| MOSI | GPIO4 | Shared with LCD |
| CLK | GPIO5 | Shared with LCD |

### 3.4 WS2812 RGB LED

| Function | GPIO | Notes |
|----------|------|-------|
| DATA | GPIO3 | Non-touch version only |

### 3.5 ADC Channels (ESP32-C6)

| GPIO | ADC Channel | Notes |
|------|-------------|-------|
| GPIO0 | ADC1_CH0 | Available on header |
| GPIO1 | ADC1_CH1 | Available on header |
| GPIO2 | ADC1_CH2 | Available on header |
| GPIO3 | ADC1_CH3 | Used by WS2812 |
| GPIO4 | ADC1_CH4 | Used by LCD |
| GPIO5 | ADC1_CH5 | Used by LCD |
| GPIO6 | ADC1_CH6 | Used by LCD |

**Available for fuel sensors: GPIO0 (ADC1_CH0) and GPIO1 (ADC1_CH1)**

---

## 4. LCD Parameters

| Parameter | Value |
|-----------|-------|
| Size | 1.9 inches (diagonal) |
| Controller | ST7789 |
| Resolution | 170 × 320 pixels |
| Color Depth | 262K colors (18-bit) |
| Interface | SPI |
| Viewing | IPS (wide angle) |

### Initialization Parameters (Arduino_GFX)

```cpp
Arduino_DataBus *bus = new Arduino_HWSPI(
    6,    // DC
    7,    // CS
    5,    // SCK
    4     // MOSI
);

Arduino_GFX *gfx = new Arduino_ST7789(
    bus,
    14,   // RST
    0,    // rotation (0=portrait)
    0,    // IPS mode
    170,  // width
    320,  // height
    35,   // col_offset1 (left)
    0,    // row_offset1 (top)
    35,   // col_offset2 (right)
    0     // row_offset2 (bottom)
);
```

---

## 5. Development Environment

### 5.1 Supported Frameworks

| Framework | IDE | Notes |
|-----------|-----|-------|
| Arduino | Arduino IDE 2.x | Beginner-friendly |
| ESP-IDF | VS Code + ESP-IDF | Professional, more control |
| PlatformIO | VS Code | Recommended for this project |

### 5.2 Arduino Board Package

- **Package**: esp32 by Espressif Systems
- **Version**: ≥3.0.7 (required for ESP32-C6)
- **Board**: "ESP32C6 Dev Module"

### 5.3 Required Libraries

| Library | Version | Purpose |
|---------|---------|---------|
| Arduino_GFX_Library | ≥1.5.6 | LCD display driver |
| LVGL | 8.4.0 | GUI framework (optional) |
| Freenove_WS2812_Lib_for_ESP32 | 2.0.0 | RGB LED (optional) |

---

## 6. Demo Code

### 6.1 Available Examples

| Demo | Description |
|------|-------------|
| 01_ADC_Test | Read system voltage via ADC |
| 02_I2C_QMI8658 | Read IMU sensor data |
| 03_SD_Card | Mount and read/write SD card |
| 04_WS2812_Test | Control RGB LED |
| 05_WIFI_AP | Create WiFi access point |
| 06_WIFI_STA | Connect to WiFi network |
| 07_Hello_World_GFX | Basic LCD drawing |
| 08_LVGL_Test | LVGL GUI demo |

### 6.2 ADC Example (Key Code)

```cpp
#include "esp_adc/adc_oneshot.h"

static adc_oneshot_unit_handle_t adc1_handle;

void adc_bsp_init(void) {
    // Calibration config
    adc_cali_curve_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT_1,
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_12,
    };
    
    // ADC unit config
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc1_handle));
    
    // Channel config
    adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN_DB_12,    // 0-3.3V range
        .bitwidth = ADC_BITWIDTH_12, // 12-bit resolution
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_0, &config));
}

void adc_get_value(float *value, int *data) {
    int adc_raw;
    esp_err_t err = adc_oneshot_read(adc1_handle, ADC_CHANNEL_0, &adc_raw);
    if (err == ESP_OK) {
        *value = ((float)adc_raw * 3.3f / 4096.0f);
        *data = adc_raw;
    }
}
```

### 6.3 GFX Display Example (Key Code)

```cpp
#include <Arduino_GFX_Library.h>

#define GFX_BL 15

Arduino_DataBus *bus = new Arduino_HWSPI(6, 7, 5, 4);
Arduino_GFX *gfx = new Arduino_ST7789(bus, 14, 0, 0, 170, 320, 35, 0, 35, 0);

void setup() {
    if (!gfx->begin()) {
        Serial.println("gfx->begin() failed!");
    }
    gfx->fillScreen(BLACK);
    
    pinMode(GFX_BL, OUTPUT);
    digitalWrite(GFX_BL, LOW);  // Backlight ON (active low)
    
    gfx->setTextColor(WHITE);
    gfx->setTextSize(2);
    gfx->setCursor(10, 10);
    gfx->println("Hello World!");
}
```

---

## 7. Physical Dimensions

### ESP32-C6-LCD-1.9 (Non-touch)

- Board Width: 27.0 mm
- Board Height: 58.5 mm
- Display Diagonal: 1.9 inches

### Mounting

- Standard 2.54mm header pins on edges
- Pico-compatible pinout

---

## 8. Power Specifications

| Parameter | Value |
|-----------|-------|
| USB Input | 5V via Type-C |
| Battery Input | 3.7V LiPo (MX1.25) |
| Operating Voltage | 3.3V (internal) |
| Battery Charging | Onboard charger |

---

## 9. Local Resources

### Datasheets (Downloaded)

- [ESP32-C6-LCD-1.9-Schematic.pdf](datasheets/ESP32-C6-LCD-1.9-Schematic.pdf)
- [ESP32-C6_Technical_Reference_Manual.pdf](datasheets/ESP32-C6_Technical_Reference_Manual.pdf)
- [ESP32-C6_Series_Datasheet.pdf](datasheets/ESP32-C6_Series_Datasheet.pdf)
- [ST7789T3_Datasheet.pdf](datasheets/ST7789T3_Datasheet.pdf)
- [QMI8658C_Datasheet.pdf](datasheets/QMI8658C_Datasheet.pdf)

### Demo Code (Downloaded)

- [resources/ESP32-C6-LCD-1.9-Demo-V2/](resources/ESP32-C6-LCD-1.9-Demo-V2/) - Official Waveshare examples

---

## 10. External Links

- [Product Wiki](https://www.waveshare.com/wiki/ESP32-C6-LCD-1.9)
- [Product Page](https://www.waveshare.com/esp32-c6-lcd-1.9.htm)
- [Arduino_GFX Library](https://github.com/moononournation/Arduino_GFX)
- [ESP32-C6 Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/)
