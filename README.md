# ESP_FuelTankGauge

A dual-tank fuel gauge display for the Waveshare ESP32-C6-LCD-1.9 development board.

## Features

- **Dual Tank Display**: Shows two fuel tank levels side-by-side
- **Gallon & Percentage Display**: Gallons shown above each bar, percentage below
- **Static Color Zones**: Red (bottom 20%), Yellow (middle 20-40%), Green (top 40-100%)
- **Pixel-Level Fill**: Smooth transitions as fuel level changes
- **Configurable Tank Capacity**: Default 50 gallons, easily adjustable
- **Three Operating Modes**:
  - **Normal**: Real ADC sensor input
  - **Demo**: Cycling simulation for testing
  - **Debug**: Sensor diagnostics overlay

## Hardware

- **Board**: Waveshare ESP32-C6-LCD-1.9
- **Display**: 1.9" ST7789 LCD (170×320 pixels)
- **MCU**: ESP32-C6FH4 (QFN32)
- **Fuel Sensors**: Standard automotive resistive senders (33-240 ohms)

## Display Layout

```
┌─────────────────────────────────────┐
│      37G            │      21G      │  ← Gallons (top)
├─────────────────────┼───────────────┤
│   ┌─────────┐       │   ┌─────────┐ │
│   │ GREEN   │       │   │ (empty) │ │  ← Top = Green zone
│   │ YELLOW  │       │   │ GREEN   │ │  ← Middle = Yellow zone
│   │ RED     │       │   │ YELLOW  │ │  ← Bottom = Red zone
│   └─────────┘       │   └─────────┘ │
│      75%            │      42%      │  ← Percentage (bottom)
└─────────────────────┴───────────────┘
```

## Configuration

All settings are in `src/config.h`:

| Setting | Default | Description |
|---------|---------|-------------|
| `TANK_CAPACITY_GALLONS` | 50 | Tank size in gallons |
| `DEMO_CYCLE_SPEED_MS` | 150 | Demo mode speed (ms per 1%) |
| `GAUGE_SEGMENT_HEIGHT` | 13 | Segment height in pixels |
| `GAUGE_SEGMENT_GAP` | 1 | Gap between segments |
| `GAUGE_BAR_WIDTH` | 60 | Bar width in pixels |

## Building

Requires PlatformIO:

```bash
pio run              # Build
pio run -t upload    # Upload to device
pio device monitor   # Serial monitor
```

## Documentation

- [Deployment Guide](doc/DEPLOYMENT.md) - **Start here** for uploading code
- [UI Specification](doc/UI_SPECIFICATION.md) - Display layout and rendering details
- [Configuration Reference](doc/CONFIG_REFERENCE.md) - All configurable parameters
- [Hardware Reference](doc/HARDWARE.md) - Pin assignments and circuits
- [Software Architecture](doc/SOFTWARE_ARCHITECTURE.md) - Code structure