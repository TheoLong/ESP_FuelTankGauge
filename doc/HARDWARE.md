# Hardware Documentation

## Document Information

| Field | Value |
|-------|-------|
| Project | ESP_FuelTankGauge |
| Version | 1.0 |
| Created | 2026-01-20 |

---

## 1. Development Board: Waveshare ESP32-C6-LCD-1.9

### 1.1 Overview

The ESP32-C6-LCD-1.9 is a compact development board featuring the ESP32-C6 microcontroller with an integrated 1.9" LCD display. It provides WiFi 6, Bluetooth 5, and Zigbee/Thread connectivity, making it ideal for IoT and HMI applications.

**Product Link:** [Waveshare ESP32-C6-LCD-1.9](https://www.waveshare.com/wiki/ESP32-C6-LCD-1.9)

### 1.2 ESP32-C6FH4 Specifications

| Parameter | Value |
|-----------|-------|
| CPU | 32-bit RISC-V single-core |
| Clock Speed | Up to 160 MHz |
| Flash | 4 MB (stacked) |
| HP SRAM | 512 KB |
| LP SRAM | 16 KB |
| ROM | 320 KB |
| WiFi | 802.11ax (WiFi 6), 2.4 GHz |
| Bluetooth | BLE 5.0 |
| IEEE 802.15.4 | Zigbee 3.0, Thread |
| ADC | 12-bit SAR, 7 channels |
| GPIO | 23 (some shared with peripherals) |
| Operating Voltage | 3.3V |
| Operating Temperature | -40°C to 105°C |

### 1.3 Onboard Components

| Component | Description |
|-----------|-------------|
| ESP32-C6FH4 | Main MCU with stacked 4MB Flash (QFN32 package) |
| ST7789 LCD | 1.9" 170×320 TFT display |
| TCA9554 | I2C GPIO expander (8-channel) |
| QMI8658 | 6-axis IMU (accelerometer + gyroscope) |
| WS2812 | RGB LED (non-touch version only) |
| MX1.25 Connector | 3.7V LiPo battery interface |
| TF Card Slot | MicroSD card support |
| USB Type-C | Power and programming interface |
| Ceramic Antenna | Onboard 2.4GHz antenna |

### 1.4 Board Dimensions

**ESP32-C6-LCD-1.9 (Without Touch)**
- Width: 27.0 mm
- Height: 58.5 mm
- Display: 1.9" diagonal

---

## 2. Display Specifications

### 2.1 LCD Parameters

| Parameter | Value |
|-----------|-------|
| Display Type | TFT LCD |
| Controller | ST7789 |
| Size | 1.9 inches (diagonal) |
| Resolution | 170 × 320 pixels |
| Color Depth | 262K colors (18-bit, displayed as 16-bit RGB565) |
| Interface | SPI (4-wire) |
| Viewing Angle | IPS (wide angle) |
| Backlight | LED with PWM control |

### 2.2 Display Orientation

For this project, we use **Portrait Mode**:
- Width: 170 pixels
- Height: 320 pixels
- Rotation: 0 (default)

### 2.3 LCD Pin Connections

| Signal | GPIO | Description |
|--------|------|-------------|
| MOSI | GPIO4 | SPI data out |
| CLK | GPIO5 | SPI clock |
| DC | GPIO6 | Data/Command select |
| CS | GPIO7 | Chip select |
| RST | GPIO14 | Reset (active low) |
| BL | GPIO15 | Backlight enable (PWM, **active LOW**) |

> **Note:** The backlight is **active LOW** - PWM 0 = full brightness, PWM 255 = off.

### 2.4 ST7789 Initialization

The display requires specific offset parameters due to the panel size:
- Column offset (left): 35
- Row offset (top): 0

```cpp
// LovyanGFX initialization (simplified)
class LGFX : public lgfx::LGFX_Device {
    lgfx::Panel_ST7789 _panel_instance;
    lgfx::Bus_SPI _bus_instance;
    lgfx::Light_PWM _light_instance;
    
public:
    LGFX(void) {
        // Panel configuration
        auto cfg = _panel_instance.config();
        cfg.memory_width = 170;
        cfg.memory_height = 320;
        cfg.offset_x = 35;
        cfg.offset_y = 0;
        cfg.invert = true;
        _panel_instance.config(cfg);
        
        // Backlight - ACTIVE LOW (invert = true)
        auto bl_cfg = _light_instance.config();
        bl_cfg.pin_bl = 15;
        bl_cfg.invert = true;  // Active LOW backlight
        _light_instance.config(bl_cfg);
    }
};
```

---

## 3. Fuel Tank Sending Unit

### 3.1 Sender Specifications

Typical automotive fuel tank sending units use a resistive float mechanism:

| Parameter | Value |
|-----------|-------|
| Type | Resistive Float Sender |
| Empty Resistance | 240Ω |
| Full Resistance | 33Ω |
| Wiper Movement | Linear with fuel level |
| Connections | 2-wire (signal + ground) |

> **Note:** Some senders have reversed polarity (33Ω empty, 240Ω full). Verify your sender's specifications and adjust configuration accordingly.

### 3.2 Sender Wiring

```
Sending Unit (in tank)
     │
     ├─── Signal ──────────► To voltage divider circuit
     │
     └─── Ground ──────────► To common ground
```

---

## 4. Voltage Divider Circuit

### 4.1 Circuit Design

To interface the 33-240Ω fuel sender with the ESP32's 3.3V ADC, we use a voltage divider with a reference resistor.

```
            Vref (3.3V)
               │
               │
              ┌┴┐
              │ │ R_ref (100Ω)
              │ │
              └┬┘
               │
               ├──────────────► To GPIO0 (ADC1_CH0) - Tank 1
               │                   or GPIO1 (ADC1_CH1) - Tank 2
              ┌┴┐
              │ │ R_sender (33-240Ω)
              │ │ (Fuel Tank Sender)
              └┬┘
               │
              GND
```

### 4.2 Design Calculations

**Voltage at ADC input:**
$$V_{ADC} = V_{ref} \times \frac{R_{sender}}{R_{ref} + R_{sender}}$$

**Calculate resistance from ADC voltage:**
$$R_{sender} = R_{ref} \times \frac{V_{ADC}}{V_{ref} - V_{ADC}}$$

**With R_ref = 100Ω and Vref = 3.3V:**

| Tank State | R_sender | V_ADC | ADC Value (12-bit) |
|------------|----------|-------|-------------------|
| Full | 33Ω | 0.82V | ~1017 |
| Half | 137Ω | 1.91V | ~2369 |
| Empty | 240Ω | 2.33V | ~2890 |

This provides ~1873 ADC counts of useful range (good resolution).

### 4.3 Component Selection

| Component | Value | Rating | Notes |
|-----------|-------|--------|-------|
| R_ref | 100Ω ± 1% | 1/4W | Metal film resistor recommended |
| Capacitor (optional) | 100nF | 16V | Parallel with R_sender for noise filtering |

### 4.4 Protection Considerations

- **Input protection**: The ESP32-C6 ADC input should not exceed 3.3V. With the voltage divider design, maximum voltage is clamped by Vref.
- **ESD protection**: Consider adding TVS diode for automotive environments
- **Noise filtering**: Add 100nF capacitor at ADC input for stable readings

---

## 5. Complete Pin Mapping

### 5.1 Pins Used by Onboard Peripherals

| GPIO | Function | Notes |
|------|----------|-------|
| GPIO4 | LCD MOSI | SPI data |
| GPIO5 | LCD CLK | SPI clock |
| GPIO6 | LCD DC | Data/Command |
| GPIO7 | LCD CS | Chip select |
| GPIO14 | LCD RST | Reset |
| GPIO15 | LCD BL | Backlight |
| GPIO8 | I2C SCL | QMI8658/Touch |
| GPIO18 | I2C SDA | QMI8658/Touch |
| GPIO19 | SD MISO | SD card |
| GPIO20 | SD CS | SD card |
| GPIO3 | WS2812 | RGB LED |

### 5.2 Available Pins for Fuel Sensors

| GPIO | ADC Channel | Use |
|------|-------------|-----|
| GPIO0 | ADC1_CH0 | **Tank 1 Sensor** |
| GPIO1 | ADC1_CH1 | **Tank 2 Sensor** |
| GPIO2 | ADC1_CH2 | **Brightness ADC** (auto-dimming input) |
| GPIO9 | - | **BOOT Button** (mode switching) |
| GPIO21 | - | Digital I/O (spare) |
| GPIO22 | - | Digital I/O (spare) |
| GPIO23 | - | Digital I/O (spare) |

### 5.3 ESP32-C6 ADC Specifications

| Parameter | Value |
|-----------|-------|
| Resolution | 12-bit (0-4095) |
| Reference Voltage | ~3.3V (with calibration) |
| Input Range (12dB attenuation) | 0-3.3V |
| Channels | 7 (ADC1_CH0 to ADC1_CH6) |
| Sampling Rate | Up to 2 Msps |
| Calibration | Curve fitting supported |

---

## 6. Power Considerations

### 6.1 Power Options

| Source | Voltage | Notes |
|--------|---------|-------|
| USB Type-C | 5V | Primary development power |
| LiPo Battery | 3.7V | Via MX1.25 connector |

### 6.2 Current Consumption Estimates

| Component | Current | Notes |
|-----------|---------|-------|
| ESP32-C6 (active) | ~80mA | WiFi disabled |
| LCD Display | ~20mA | Backlight at 50% |
| Voltage Dividers | ~33mA | 2× (3.3V/100Ω) worst case |
| **Total** | ~133mA | Typical operation |

### 6.3 Automotive Power Integration

For vehicle installation:
- Use DC-DC converter (12V → 5V) for USB power
- Consider adding reverse polarity protection
- Add capacitors for voltage spike filtering
- Use fused connection to vehicle power

---

## 7. Wiring Diagram

### 7.1 Fuel Sensor Connections

```
┌────────────────────────────────────────────────────────────────┐
│                    ESP32-C6-LCD-1.9                             │
│                                                                  │
│   ┌──────┐                                                      │
│   │ LCD  │                                                      │
│   │1.9"  │                                                      │
│   └──────┘                                                      │
│                                                                  │
│   Expansion Header:                                              │
│   ┌─────────────────────────────────────────────────────────┐   │
│   │ 3V3  GND  GPIO0  GPIO1  GPIO2  GPIO21  GPIO22  GPIO23   │   │
│   └──┬────┬─────┬──────┬─────────────────────────────────────┘  │
│      │    │     │      │                                        │
└──────┼────┼─────┼──────┼────────────────────────────────────────┘
       │    │     │      │
       │    │     │      └─────────────────┐
       │    │     │                        │
       │    │     │    ┌───────────┐       │    ┌───────────┐
       │    │     │    │  R_ref    │       │    │  R_ref    │
       │    │     │    │  100Ω     │       │    │  100Ω     │
       │    │     │    └─────┬─────┘       │    └─────┬─────┘
       │    │     │          │             │          │
       │    │     └──────────┤             └──────────┤
       │    │                │                        │
       │    │         ┌──────┴──────┐          ┌──────┴──────┐
       │    │         │ Tank 1     │          │ Tank 2     │
       │    │         │ Sender     │          │ Sender     │
       │    │         │ (33-240Ω)  │          │ (33-240Ω)  │
       │    │         └──────┬──────┘          └──────┬──────┘
       │    │                │                        │
       │    └────────────────┴────────────────────────┘
       │                     │
       └─────────────────────┘ (3.3V to top of voltage dividers)
```

### 7.2 Bill of Materials (BOM)

| Qty | Component | Value | Part Number | Notes |
|-----|-----------|-------|-------------|-------|
| 1 | ESP32-C6-LCD-1.9 | - | Waveshare 30937 | Development board |
| 2 | Fuel Tank Sender | 33-240Ω | - | Standard automotive |
| 2 | Resistor | 100Ω 1% | - | 1/4W metal film |
| 2 | Capacitor | 100nF | - | Optional, noise filter |
| 1 | USB Cable | Type-A to Type-C | - | Programming/power |
| - | Wire | 22 AWG | - | Connections |

---

## 8. Assembly Notes

### 8.1 Voltage Divider Assembly

1. Solder 100Ω resistor between 3.3V pin and signal wire
2. Connect signal wire to GPIO0 (Tank 1) or GPIO1 (Tank 2)
3. Connect fuel sender between signal wire and GND
4. Optional: Add 100nF capacitor across sender for filtering

### 8.2 Sender Installation

1. Verify sender resistance range matches configuration (33-240Ω)
2. Check if sender is grounded through mounting or needs separate ground wire
3. Route wires away from high-current/high-heat areas
4. Use appropriate wire gauge for distance (22 AWG for short runs)

### 8.3 Board Mounting

1. Mount board in enclosure with display visible
2. Ensure ventilation if enclosed
3. Protect from moisture in automotive environments
4. Consider conformal coating for harsh environments

---

## 9. Troubleshooting

### 9.1 ADC Reading Issues

| Symptom | Possible Cause | Solution |
|---------|----------------|----------|
| Always reads 0 | Open circuit | Check sender wiring |
| Always reads max | Short circuit | Check for shorts to ground |
| Erratic readings | Noise | Add filtering capacitor |
| Readings inverted | Sender polarity | Swap R_full/R_empty in config |

### 9.2 Display Issues

| Symptom | Possible Cause | Solution |
|---------|----------------|----------|
| Blank screen | Backlight off | Check GPIO15 |
| Wrong colors | Rotation issue | Verify rotation parameter |
| Partial display | Offset wrong | Check col/row offsets |

---

## 10. Reference Documents

All datasheets are stored locally in [doc/datasheets/](datasheets/):

- [ESP32-C6-LCD-1.9-Schematic.pdf](datasheets/ESP32-C6-LCD-1.9-Schematic.pdf) - Board schematic
- [ESP32-C6_Technical_Reference_Manual.pdf](datasheets/ESP32-C6_Technical_Reference_Manual.pdf) - Full TRM
- [ESP32-C6_Series_Datasheet.pdf](datasheets/ESP32-C6_Series_Datasheet.pdf) - MCU datasheet
- [ST7789T3_Datasheet.pdf](datasheets/ST7789T3_Datasheet.pdf) - LCD controller
- [QMI8658C_Datasheet.pdf](datasheets/QMI8658C_Datasheet.pdf) - IMU (not used in this project)
