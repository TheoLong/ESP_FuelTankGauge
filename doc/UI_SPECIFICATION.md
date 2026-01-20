# UI Specification

## Document Information

| Field | Value |
|-------|-------|
| Project | ESP_FuelTankGauge |
| Version | 1.0 |
| Created | 2026-01-20 |

---

## 1. Display Overview

### 1.1 Physical Display

| Parameter | Value |
|-----------|-------|
| Display | Waveshare 1.9" LCD (ST7789) |
| Resolution | 170 × 320 pixels |
| Orientation | Portrait |
| Color Depth | 16-bit RGB565 (65K colors) |

### 1.2 Coordinate System

```
(0,0) ─────────────────────► X (170)
  │
  │
  │
  │
  │
  │
  │
  ▼
  Y (320)
```

---

## 2. Screen Layout

### 2.1 Overall Layout

The screen is divided into two equal halves, one for each fuel tank.

```
┌─────────────────────────────────────────────────────────────────────┐
│                                                                     │ ← Y=0
│                         TITLE AREA (Optional)                       │
│                          "FUEL GAUGE"                               │
│                                                                     │ ← Y=25
├─────────────────────────────┬───────────────────────────────────────┤
│                             │                                       │
│         TANK 1              │              TANK 2                   │
│                             │                                       │
│    ┌─────────────────┐      │      ┌─────────────────┐              │ ← Y=40
│    │                 │      │      │                 │              │
│    │    [  75%  ]    │      │      │    [  42%  ]    │              │
│    │                 │      │      │                 │              │
│    └─────────────────┘      │      └─────────────────┘              │ ← Y=60
│                             │                                       │
│    ┌───────────────────┐    │    ┌───────────────────┐              │ ← Y=70
│    │░░░░░░░░░░░░░░░░░░░│    │    │                   │              │
│    │░░░░░░░░░░░░░░░░░░░│    │    │                   │              │
│    │░░░░░░░░░░░░░░░░░░░│    │    │                   │ ← Segment    │
│    │░░░░░░░░░░░░░░░░░░░│    │    │                   │   Lines      │
│    │███████████████████│    │    │                   │              │
│    │███████████████████│    │    │███████████████████│              │
│    │███████████████████│    │    │███████████████████│              │
│    │███████████████████│    │    │███████████████████│              │
│    │▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓│    │    │███████████████████│              │
│    │▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓│    │    │▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓│              │
│    │▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒│    │    │▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓│              │
│    │▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒│    │    │▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒│              │
│    └───────────────────┘    │    └───────────────────┘              │ ← Y=300
│                             │                                       │
│           "T1"              │              "T2"                     │ ← Y=305
│                             │                                       │
└─────────────────────────────┴───────────────────────────────────────┘ ← Y=320
X=0                          X=85                                   X=170

Legend:
░░░ = Empty (Black background)
███ = Green (40-100%)
▓▓▓ = Yellow (20-40%)
▒▒▒ = Red (0-20%)
```

### 2.2 Layout Dimensions

| Element | X Start | X End | Y Start | Y End | Width | Height |
|---------|---------|-------|---------|-------|-------|--------|
| Screen | 0 | 170 | 0 | 320 | 170 | 320 |
| Tank 1 Zone | 0 | 85 | 0 | 320 | 85 | 320 |
| Tank 2 Zone | 85 | 170 | 0 | 320 | 85 | 320 |
| Tank 1 Percentage | 10 | 75 | 40 | 60 | 65 | 20 |
| Tank 2 Percentage | 95 | 160 | 40 | 60 | 65 | 20 |
| Tank 1 Bar | 17 | 68 | 70 | 300 | 51 | 230 |
| Tank 2 Bar | 102 | 153 | 70 | 300 | 51 | 230 |

### 2.3 Calculated Constants

```cpp
// Screen dimensions
#define SCREEN_WIDTH          170
#define SCREEN_HEIGHT         320

// Zone division
#define ZONE_WIDTH            (SCREEN_WIDTH / 2)    // 85 pixels per tank

// Percentage text area
#define PCT_TEXT_Y            40
#define PCT_TEXT_HEIGHT       20

// Bar gauge area
#define BAR_TOP               70
#define BAR_BOTTOM            300
#define BAR_HEIGHT            (BAR_BOTTOM - BAR_TOP)  // 230 pixels

// Bar dimensions
#define BAR_WIDTH             51
#define BAR_MARGIN            17  // From edge of zone to bar

// Bar positions
#define BAR1_X                BAR_MARGIN                    // 17
#define BAR2_X                (ZONE_WIDTH + BAR_MARGIN)     // 102

// Label area
#define LABEL_Y               305
```

---

## 3. Bar Gauge Design

### 3.1 Segment Structure

The bar is divided into 20 visual segments for easier reading. Each segment has a thin divider line.

```
Bar Height = 230 pixels
Segments = 20
Segment Height = 230 / 20 = 11.5 pixels (use 11 with remainder at top)

Segment visual structure:
┌─────────────────────┐
│███████████████████│ ← Segment 20 (top, 100%)
├─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┤ ← Divider line (1px, dark gray)
│███████████████████│ ← Segment 19 (95%)
├─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┤
│███████████████████│ ← Segment 18 (90%)
...
│▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒│ ← Segment 2 (10%)
├─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┤
│▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒│ ← Segment 1 (bottom, 5%)
└─────────────────────┘
```

### 3.2 Continuous Fill (Not Discrete)

The fill is **continuous**, not aligned to segment boundaries:

```
At 75% fill (not 80% or 70%):

┌─────────────────────┐
│                    │ ← Empty (black)
├─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┤
│                    │
├─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┤
│                    │
├─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┤
│████████████████████│ ← Fill starts here (75% point)
│████████████████████│   Fill is GREEN (40-100% zone)
├─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┤
│████████████████████│
├─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┤
...
│████████████████████│
├─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┤
│▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓│ ← YELLOW zone (20-40%)
├─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┤
│▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓│
├─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┤
│▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒│ ← RED zone (0-20%)
├─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┤
│▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒│
└─────────────────────┘
```

### 3.3 Color Zones

The bar displays all three color zones within the filled area:

| Zone | Percentage | Color | RGB565 Hex |
|------|------------|-------|------------|
| Red | 0% - 20% | Red | `0xF800` |
| Yellow | 20% - 40% | Yellow | `0xFFE0` |
| Green | 40% - 100% | Green | `0x07E0` |

**Important:** Even if tank is at 100%, all three colors are visible in the bar.

---

## 4. Percentage Text Display

### 4.1 Text Specifications

| Property | Value |
|----------|-------|
| Font | Built-in Arduino_GFX font |
| Text Size | 2 (16 pixels high) |
| Format | "XX%" right-aligned |
| Color | Matches current fuel level zone |

### 4.2 Text Color Mapping

The percentage text color reflects the current fuel level:

| Level | Text Color | Example |
|-------|------------|---------|
| 0-20% | Red | <span style="color:red">**15%**</span> |
| 20-40% | Yellow | <span style="color:yellow">**35%**</span> |
| 40-100% | Green | <span style="color:green">**75%**</span> |

### 4.3 Text Layout

```cpp
// Text size 2 = approximately 12x16 pixels per character
// "100%" = 4 characters = ~48 pixels wide

void draw_percentage_text(int zone_center_x, int y, uint8_t percent) {
    char buf[5];
    sprintf(buf, "%d%%", percent);
    
    // Calculate text width for centering
    int text_width = strlen(buf) * 12;  // Approximate
    int text_x = zone_center_x - (text_width / 2);
    
    uint16_t color = get_color_for_percent(percent);
    
    // Clear previous text area
    gfx->fillRect(zone_center_x - 30, y, 60, 20, COLOR_BACKGROUND);
    
    // Draw new text
    gfx->setTextColor(color);
    gfx->setTextSize(2);
    gfx->setCursor(text_x, y);
    gfx->print(buf);
}
```

---

## 5. Visual Elements

### 5.1 Color Palette (RGB565)

| Element | Color Name | RGB565 | RGB888 |
|---------|------------|--------|--------|
| Background | Black | `0x0000` | (0, 0, 0) |
| Border | White | `0xFFFF` | (255, 255, 255) |
| Segment Line | Dark Gray | `0x4208` | (66, 66, 66) |
| Red Zone | Pure Red | `0xF800` | (255, 0, 0) |
| Yellow Zone | Pure Yellow | `0xFFE0` | (255, 255, 0) |
| Green Zone | Pure Green | `0x07E0` | (0, 255, 0) |
| Empty Area | Black | `0x0000` | (0, 0, 0) |

### 5.2 RGB565 Conversion

```cpp
// Convert RGB888 to RGB565
#define RGB565(r, g, b) (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3))

// Examples:
// Red:    RGB565(255, 0, 0)     = 0xF800
// Green:  RGB565(0, 255, 0)     = 0x07E0
// Blue:   RGB565(0, 0, 255)     = 0x001F
// Yellow: RGB565(255, 255, 0)   = 0xFFE0
// White:  RGB565(255, 255, 255) = 0xFFFF
// Black:  RGB565(0, 0, 0)       = 0x0000
```

### 5.3 Border Style

| Property | Value |
|----------|-------|
| Width | 2 pixels |
| Color | White (`0xFFFF`) |
| Style | Solid rectangle around each bar |

```cpp
void draw_bar_border(int x, int y, int width, int height) {
    // Draw 2-pixel wide border
    gfx->drawRect(x - 1, y - 1, width + 2, height + 2, COLOR_BORDER);
    gfx->drawRect(x - 2, y - 2, width + 4, height + 4, COLOR_BORDER);
}
```

---

## 6. Rendering Algorithm

### 6.1 Pixel-by-Pixel Bar Rendering

Since the UI is simple, we can draw the bar pixel by pixel for maximum control:

```cpp
/**
 * Draw a fuel gauge bar with color zones
 * 
 * @param bar_x     Left edge of bar
 * @param bar_y     Top edge of bar
 * @param bar_w     Width of bar
 * @param bar_h     Height of bar
 * @param percent   Fuel level 0-100
 */
void draw_fuel_bar(int bar_x, int bar_y, int bar_w, int bar_h, uint8_t percent) {
    // Calculate fill height (from bottom)
    int fill_height = (bar_h * percent) / 100;
    int fill_start_y = bar_y + bar_h - fill_height;
    
    // Draw each horizontal line
    for (int y = bar_y; y < bar_y + bar_h; y++) {
        // Calculate what percentage this Y position represents
        // Bottom of bar = 0%, Top of bar = 100%
        int y_percent = 100 - ((y - bar_y) * 100 / bar_h);
        
        if (y >= fill_start_y) {
            // Filled area - choose color based on position
            uint16_t fill_color;
            if (y_percent <= THRESHOLD_RED) {
                fill_color = COLOR_RED;
            } else if (y_percent <= THRESHOLD_YELLOW) {
                fill_color = COLOR_YELLOW;
            } else {
                fill_color = COLOR_GREEN;
            }
            gfx->drawFastHLine(bar_x, y, bar_w, fill_color);
        } else {
            // Empty area
            gfx->drawFastHLine(bar_x, y, bar_w, COLOR_BACKGROUND);
        }
    }
    
    // Draw segment divider lines
    int segment_height = bar_h / GAUGE_SEGMENTS;
    for (int i = 1; i < GAUGE_SEGMENTS; i++) {
        int line_y = bar_y + i * segment_height;
        gfx->drawFastHLine(bar_x, line_y, bar_w, COLOR_SEGMENT_LINE);
    }
    
    // Draw border
    draw_bar_border(bar_x, bar_y, bar_w, bar_h);
}
```

### 6.2 Optimized Partial Update

To avoid flickering and improve performance, only update the portion that changed:

```cpp
void update_fuel_bar(int bar_x, int bar_y, int bar_w, int bar_h, 
                     uint8_t old_percent, uint8_t new_percent) {
    if (old_percent == new_percent) return;
    
    int old_fill_h = (bar_h * old_percent) / 100;
    int new_fill_h = (bar_h * new_percent) / 100;
    
    int old_top = bar_y + bar_h - old_fill_h;
    int new_top = bar_y + bar_h - new_fill_h;
    
    if (new_percent > old_percent) {
        // Level increased - fill the new area
        for (int y = new_top; y < old_top; y++) {
            int y_percent = 100 - ((y - bar_y) * 100 / bar_h);
            uint16_t color = get_color_for_percent(y_percent);
            gfx->drawFastHLine(bar_x, y, bar_w, color);
        }
    } else {
        // Level decreased - clear the old area
        for (int y = old_top; y < new_top; y++) {
            gfx->drawFastHLine(bar_x, y, bar_w, COLOR_BACKGROUND);
        }
    }
    
    // Redraw segment lines in affected area
    redraw_segment_lines_in_range(bar_x, bar_w, min(old_top, new_top), max(old_top, new_top));
}
```

---

## 7. Operating Modes & Display Variations

### 7.1 Normal Mode

Standard display with gauges and percentage only:
- Two bar gauges (left and right)
- Percentage text above each bar
- Color-coded levels (Red/Yellow/Green)

### 7.2 Demo Mode

Same display as Normal mode, but with simulated cycling values:

```cpp
// Demo mode cycling pattern
// Tank 1: 0% → 100% → 0% ...
// Tank 2: Offset by DEMO_TANK2_OFFSET (default 50%)

static uint8_t demo_value = 0;
static bool demo_direction = true;  // true = increasing

void demo_mode_update() {
    if (demo_direction) {
        demo_value++;
        if (demo_value >= 100) demo_direction = false;
    } else {
        demo_value--;
        if (demo_value == 0) demo_direction = true;
    }
}

uint8_t demo_get_tank1() { return demo_value; }
uint8_t demo_get_tank2() { return (demo_value + DEMO_TANK2_OFFSET) % 101; }
```

### 7.3 Debug Mode Display

Debug mode shows additional diagnostic information overlaid on the gauge:

```
┌────────────────────────┬────────────────────────┐
│        [ 75% ]         │        [ 42% ]         │
│       (GREEN)          │       (YELLOW)         │
│                        │                        │
│    ┌──────────────┐    │    ┌──────────────┐    │
│    │██████████████│    │    │              │    │
│    │██████████████│    │    │              │    │
│    │██████████████│    │    │              │    │
│    │██████████████│    │    │██████████████│    │
│    │┌────────────┐│    │    │┌────────────┐│    │
│    ││PIN: GPIO0  ││    │    ││PIN: GPIO1  ││    │
│    ││ADC: 1842   ││    │    ││ADC: 2567   ││    │
│    ││V:   1.48V  ││    │    ││V:   2.07V  ││    │
│    ││R:   82Ω    ││    │    ││R:   168Ω   ││    │
│    │└────────────┘│    │    │└────────────┘│    │
│    │▓▓▓▓▓▓▓▓▓▓▓▓▓▓│    │    │▓▓▓▓▓▓▓▓▓▓▓▓▓▓│    │
│    │▒▒▒▒▒▒▒▒▒▒▒▒▒▒│    │    │▒▒▒▒▒▒▒▒▒▒▒▒▒▒│    │
│    └──────────────┘    │    └──────────────┘    │
│          T1            │          T2            │
└────────────────────────┴────────────────────────┘
```

### 7.4 Debug Overlay Specifications

| Element | Value | Notes |
|---------|-------|-------|
| Position | Inside bar, mid-height | Centered vertically |
| Background | Semi-transparent black | Or solid black |
| Text Color | Cyan for labels | Yellow for values |
| Font Size | 1 (6×8 pixels) | Small to fit inside bar |
| Line Spacing | 12 pixels | Between debug lines |

**Debug Information Displayed:**

| Line | Label | Example | Description |
|------|-------|---------|-------------|
| 1 | PIN: | GPIO0 | ADC GPIO pin number |
| 2 | ADC: | 1842 | Raw ADC value (0-4095) |
| 3 | V: | 1.48V | Calculated voltage |
| 4 | R: | 82Ω | Calculated resistance |

### 7.5 Debug Mode Implementation

```cpp
void debug_mode_draw_overlay(tank_id_t tank, const debug_info_t* info) {
    int bar_x = (tank == TANK_1) ? BAR1_X : BAR2_X;
    int overlay_y = DEBUG_INFO_Y;
    int overlay_w = GAUGE_BAR_WIDTH - 4;
    int overlay_h = 4 * DEBUG_LINE_HEIGHT + 4;
    
    // Draw background box
    gfx->fillRect(bar_x + 2, overlay_y, overlay_w, overlay_h, COLOR_BACKGROUND);
    gfx->drawRect(bar_x + 2, overlay_y, overlay_w, overlay_h, COLOR_DEBUG_LABEL);
    
    // Draw debug info lines
    gfx->setTextSize(TEXT_SIZE_DEBUG);
    int text_x = bar_x + 4;
    int y = overlay_y + 2;
    
    #if DEBUG_SHOW_PIN
        gfx->setTextColor(COLOR_DEBUG_LABEL);
        gfx->setCursor(text_x, y);
        gfx->print("PIN:");
        gfx->setTextColor(COLOR_DEBUG_VALUE);
        gfx->print("GPIO");
        gfx->print(info->gpio_pin);
        y += DEBUG_LINE_HEIGHT;
    #endif
    
    #if DEBUG_SHOW_RAW_ADC
        gfx->setTextColor(COLOR_DEBUG_LABEL);
        gfx->setCursor(text_x, y);
        gfx->print("ADC:");
        gfx->setTextColor(COLOR_DEBUG_VALUE);
        gfx->print(info->raw_adc);
        y += DEBUG_LINE_HEIGHT;
    #endif
    
    #if DEBUG_SHOW_VOLTAGE
        gfx->setTextColor(COLOR_DEBUG_LABEL);
        gfx->setCursor(text_x, y);
        gfx->print("V:");
        gfx->setTextColor(COLOR_DEBUG_VALUE);
        gfx->print(info->voltage, 2);
        gfx->print("V");
        y += DEBUG_LINE_HEIGHT;
    #endif
    
    #if DEBUG_SHOW_RESISTANCE
        gfx->setTextColor(COLOR_DEBUG_LABEL);
        gfx->setCursor(text_x, y);
        gfx->print("R:");
        gfx->setTextColor(COLOR_DEBUG_VALUE);
        gfx->print((int)info->resistance);
        gfx->print((char)0xF4);  // Ω symbol or use "ohm"
    #endif
}
```

### 7.6 Update Behavior

| Aspect | Behavior |
|--------|----------|
| Refresh Rate | 10 Hz (100ms interval) |
| Minimum Change | 1% to trigger gauge redraw |
| Smoothing | EMA filter on sensor readings |
| Animation | None (instant updates) |
| Debug Overlay | Updates every cycle in DEBUG mode |

---

## 8. Sample Renderings

### 8.1 Empty Tanks (0% / 0%)

```
┌────────────────────────┬────────────────────────┐
│                        │                        │
│         [ 0% ]         │         [ 0% ]         │
│        (RED)           │        (RED)           │
│                        │                        │
│    ┌──────────────┐    │    ┌──────────────┐    │
│    │              │    │    │              │    │
│    │              │    │    │              │    │
│    │              │    │    │              │    │
│    │   (empty)    │    │    │   (empty)    │    │
│    │              │    │    │              │    │
│    │              │    │    │              │    │
│    │              │    │    │              │    │
│    │              │    │    │              │    │
│    └──────────────┘    │    └──────────────┘    │
│                        │                        │
│          T1            │          T2            │
└────────────────────────┴────────────────────────┘
```

### 8.2 Low Fuel Warning (15% / 10%)

```
┌────────────────────────┬────────────────────────┐
│                        │                        │
│        [ 15% ]         │        [ 10% ]         │
│        (RED)           │        (RED)           │
│                        │                        │
│    ┌──────────────┐    │    ┌──────────────┐    │
│    │              │    │    │              │    │
│    │              │    │    │              │    │
│    │              │    │    │              │    │
│    │              │    │    │              │    │
│    │              │    │    │              │    │
│    │              │    │    │              │    │
│    │              │    │    │              │    │
│    │▒▒▒▒▒▒▒▒▒▒▒▒▒▒│    │    │              │    │
│    │▒▒▒▒▒▒▒▒▒▒▒▒▒▒│    │    │▒▒▒▒▒▒▒▒▒▒▒▒▒▒│    │
│    │▒▒▒▒▒▒▒▒▒▒▒▒▒▒│    │    │▒▒▒▒▒▒▒▒▒▒▒▒▒▒│    │
│    └──────────────┘    │    └──────────────┘    │
│          T1            │          T2            │
└────────────────────────┴────────────────────────┘
```

### 8.3 Mid Level (50% / 30%)

```
┌────────────────────────┬────────────────────────┐
│                        │                        │
│        [ 50% ]         │        [ 30% ]         │
│       (GREEN)          │       (YELLOW)         │
│                        │                        │
│    ┌──────────────┐    │    ┌──────────────┐    │
│    │              │    │    │              │    │
│    │              │    │    │              │    │
│    │              │    │    │              │    │
│    │              │    │    │              │    │
│    │██████████████│    │    │              │    │
│    │██████████████│    │    │              │    │
│    │▓▓▓▓▓▓▓▓▓▓▓▓▓▓│    │    │▓▓▓▓▓▓▓▓▓▓▓▓▓▓│    │
│    │▓▓▓▓▓▓▓▓▓▓▓▓▓▓│    │    │▓▓▓▓▓▓▓▓▓▓▓▓▓▓│    │
│    │▒▒▒▒▒▒▒▒▒▒▒▒▒▒│    │    │▒▒▒▒▒▒▒▒▒▒▒▒▒▒│    │
│    │▒▒▒▒▒▒▒▒▒▒▒▒▒▒│    │    │▒▒▒▒▒▒▒▒▒▒▒▒▒▒│    │
│    └──────────────┘    │    └──────────────┘    │
│          T1            │          T2            │
└────────────────────────┴────────────────────────┘
```

### 8.4 Full Tanks (100% / 100%)

```
┌────────────────────────┬────────────────────────┐
│                        │                        │
│       [ 100% ]         │       [ 100% ]         │
│       (GREEN)          │       (GREEN)          │
│                        │                        │
│    ┌──────────────┐    │    ┌──────────────┐    │
│    │██████████████│    │    │██████████████│    │
│    │██████████████│    │    │██████████████│    │
│    │██████████████│    │    │██████████████│    │
│    │██████████████│    │    │██████████████│    │
│    │██████████████│    │    │██████████████│    │
│    │██████████████│    │    │██████████████│    │
│    │▓▓▓▓▓▓▓▓▓▓▓▓▓▓│    │    │▓▓▓▓▓▓▓▓▓▓▓▓▓▓│    │
│    │▓▓▓▓▓▓▓▓▓▓▓▓▓▓│    │    │▓▓▓▓▓▓▓▓▓▓▓▓▓▓│    │
│    │▒▒▒▒▒▒▒▒▒▒▒▒▒▒│    │    │▒▒▒▒▒▒▒▒▒▒▒▒▒▒│    │
│    │▒▒▒▒▒▒▒▒▒▒▒▒▒▒│    │    │▒▒▒▒▒▒▒▒▒▒▒▒▒▒│    │
│    └──────────────┘    │    └──────────────┘    │
│          T1            │          T2            │
└────────────────────────┴────────────────────────┘
```

---

## 9. Configuration Options

### 9.1 Configurable UI Parameters

These can be modified in `config.h`:

```cpp
// ============== GAUGE LAYOUT ==============
#define GAUGE_SEGMENTS        20      // Number of visual segments
#define GAUGE_SEGMENT_GAP     2       // Pixels for divider lines

// ============== COLOR THRESHOLDS ==============
#define THRESHOLD_RED         20      // 0-X% = Red
#define THRESHOLD_YELLOW      40      // X-Y% = Yellow
                                      // Y-100% = Green

// ============== COLORS (RGB565) ==============
#define COLOR_BACKGROUND      0x0000  // Black
#define COLOR_BORDER          0xFFFF  // White
#define COLOR_SEGMENT_LINE    0x4208  // Dark gray
#define COLOR_RED             0xF800  // Pure red
#define COLOR_YELLOW          0xFFE0  // Pure yellow
#define COLOR_GREEN           0x07E0  // Pure green
```

### 9.2 Alternative Color Schemes

**Cool Blue Theme:**
```cpp
#define COLOR_RED             0xF800  // Red (unchanged)
#define COLOR_YELLOW          0x07FF  // Cyan
#define COLOR_GREEN           0x001F  // Blue
```

**High Contrast:**
```cpp
#define COLOR_RED             0xF800  // Red
#define COLOR_YELLOW          0xFFFF  // White
#define COLOR_GREEN           0x07E0  // Green
#define COLOR_BACKGROUND      0x0000  // Black
#define COLOR_BORDER          0xFFE0  // Yellow
```

---

## 10. Implementation Notes

### 10.1 Drawing Order

1. Clear screen (once at startup)
2. Draw static elements (borders, labels)
3. Draw initial bar state (both tanks at 0%)
4. In main loop: update only changed portions

### 10.2 Avoiding Flicker

- Never clear the entire screen after initialization
- Use partial updates (only redraw changed areas)
- Draw segment lines on top of fill colors
- Update text by clearing only text area, not whole screen

### 10.3 Font Considerations

Arduino_GFX uses built-in fonts:
- Size 1: 6×8 pixels
- Size 2: 12×16 pixels (recommended for percentage)
- Size 3: 18×24 pixels
- Size 4: 24×32 pixels

For 170px wide screen, size 2 provides good readability.

---

## 11. Accessibility Notes

### 11.1 Color Blindness Considerations

The default Red/Yellow/Green scheme may be difficult for color-blind users. Alternatives:

**For Protanopia/Deuteranopia (Red-Green blindness):**
- Use Red/White/Blue instead
- Add patterns or symbols to zones

**Alternative indicators:**
- Add numeric labels at zone boundaries
- Use different fill patterns per zone

### 11.2 Viewing Distance

For automotive use, consider:
- Larger text size if viewing from >1 meter
- Higher contrast colors
- Thicker segment lines

---

## 12. Future UI Enhancements

| Enhancement | Description | Complexity |
|-------------|-------------|------------|
| Tank labels | Show "TANK 1" / "TANK 2" text | Low |
| Fuel icons | Small fuel pump icon above bars | Medium |
| Low fuel warning | Flash red when below threshold | Low |
| Gradient fill | Smooth color transition | Medium |
| Digital fuel level | Show liters/gallons | Medium |
| History graph | Mini line graph of recent levels | High |
