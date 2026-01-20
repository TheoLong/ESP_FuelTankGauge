#include "modes.h"
#include "../display/display.h"
#include "../display/brightness.h"

#ifndef NATIVE_BUILD
#include <Arduino.h>
#endif

// ============================================================================
// Runtime Mode Management
// ============================================================================

static OperatingMode current_mode = OP_MODE_DEMO;  // Default mode

// Forward declaration for debug overlay state (defined below)
static bool debug_overlay_drawn = false;

// Demo mode brightness cycling (5 levels)
#define DEMO_BRIGHTNESS_LEVELS 5
static int demo_brightness_step = 0;  // 0 = full brightness, 4 = lowest
static const uint8_t demo_brightness_values[DEMO_BRIGHTNESS_LEVELS] = {
    255,  // Level 0: 100% (full)
    192,  // Level 1: 75%
    128,  // Level 2: 50%
    64,   // Level 3: 25%
    32    // Level 4: ~12% (minimum, but not off)
};

void mode_init() {
    current_mode = (OperatingMode)DEFAULT_MODE;
}

OperatingMode mode_get_current() {
    return current_mode;
}

void mode_set(OperatingMode mode) {
    current_mode = mode;
}

OperatingMode mode_cycle_next() {
    // Reset debug overlay state when leaving debug mode
    if (current_mode == OP_MODE_DEBUG) {
        debug_overlay_drawn = false;
    }
    
    // Sequence: Normal -> Debug -> Demo (with brightness cycling) -> Normal
    switch (current_mode) {
        case OP_MODE_NORMAL:
            current_mode = OP_MODE_DEBUG;
            debug_overlay_drawn = false;  // Force redraw when entering debug
            brightness_set(255);  // Full brightness for debug
            break;
        case OP_MODE_DEBUG:
            current_mode = OP_MODE_DEMO;
            demo_brightness_step = 0;  // Start at full brightness
            brightness_set(demo_brightness_values[0]);
            break;
        case OP_MODE_DEMO:
            // Cycle through brightness levels before leaving demo mode
            demo_brightness_step++;
            if (demo_brightness_step >= DEMO_BRIGHTNESS_LEVELS) {
                // Done with brightness demo, go back to Normal
                demo_brightness_step = 0;
                brightness_set(255);  // Restore full brightness
                current_mode = OP_MODE_NORMAL;
            } else {
                // Set next brightness level
                brightness_set(demo_brightness_values[demo_brightness_step]);
            }
            break;
        default:
            current_mode = OP_MODE_NORMAL;
            brightness_set(255);
            break;
    }
    return current_mode;
}

const char* mode_get_name(OperatingMode mode) {
    switch (mode) {
        case OP_MODE_NORMAL: return "NORMAL";
        case OP_MODE_DEMO:   return "DEMO";
        case OP_MODE_DEBUG:  return "DEBUG";
        default:             return "UNKNOWN";
    }
}

int demo_get_brightness_step() {
    return demo_brightness_step;
}

int demo_get_brightness_levels() {
    return DEMO_BRIGHTNESS_LEVELS;
}

// ============================================================================
// Button Handling
// ============================================================================

static bool button_last_state = true;  // HIGH when not pressed (pulled up)
static unsigned long button_last_change = 0;
static bool button_pressed_pending = false;

void button_init() {
#ifndef NATIVE_BUILD
    pinMode(PIN_BOOT_BUTTON, INPUT_PULLUP);
    button_last_state = digitalRead(PIN_BOOT_BUTTON);
    button_last_change = millis();
#endif
}

bool button_check_press() {
#ifndef NATIVE_BUILD
    bool current_state = digitalRead(PIN_BOOT_BUTTON);
    unsigned long now = millis();
    
    // Check for state change
    if (current_state != button_last_state) {
        button_last_change = now;
        button_last_state = current_state;
        
        // Button pressed (transition from HIGH to LOW)
        if (current_state == LOW) {
            button_pressed_pending = true;
        }
    }
    
    // Return press after debounce period
    if (button_pressed_pending && (now - button_last_change >= BUTTON_DEBOUNCE_MS)) {
        button_pressed_pending = false;
        return true;
    }
#endif
    return false;
}

// ============================================================================
// Demo Mode Implementation
// ============================================================================

// Demo state
static float demo_tank1 = 50.0f;
static float demo_tank2 = 75.0f;
static float demo_direction1 = 1.0f;  // 1 = increasing, -1 = decreasing
static float demo_direction2 = -1.0f;
static unsigned long demo_last_update = 0;

void demo_mode_init() {
    demo_tank1 = DEMO_START_TANK1;
    demo_tank2 = DEMO_START_TANK2;
    demo_direction1 = 1.0f;
    demo_direction2 = -1.0f;
    demo_last_update = 0;
}

void demo_mode_update(float* tank1_percent, float* tank2_percent) {
    #ifndef NATIVE_BUILD
    unsigned long now = millis();
    #else
    static unsigned long fake_millis = 0;
    fake_millis += 100; // Simulate 100ms per call in native tests
    unsigned long now = fake_millis;
    #endif
    
    // Only update at specified interval
    if (now - demo_last_update < DEMO_CYCLE_INTERVAL_MS) {
        *tank1_percent = demo_tank1;
        *tank2_percent = demo_tank2;
        return;
    }
    demo_last_update = now;
    
    // Calculate step size based on cycle speed
    float step = DEMO_CYCLE_STEP;
    
    // Update tank 1 (cycles between 0-100)
    demo_tank1 += step * demo_direction1;
    if (demo_tank1 >= 100.0f) {
        demo_tank1 = 100.0f;
        demo_direction1 = -1.0f;
    } else if (demo_tank1 <= 0.0f) {
        demo_tank1 = 0.0f;
        demo_direction1 = 1.0f;
    }
    
    // Update tank 2 (cycles opposite direction)
    demo_tank2 += step * demo_direction2;
    if (demo_tank2 >= 100.0f) {
        demo_tank2 = 100.0f;
        demo_direction2 = -1.0f;
    } else if (demo_tank2 <= 0.0f) {
        demo_tank2 = 0.0f;
        demo_direction2 = 1.0f;
    }
    
    *tank1_percent = demo_tank1;
    *tank2_percent = demo_tank2;
}

// ============================================================================
// Debug Mode Implementation
// ============================================================================

// Debug overlay position - centered on screen
// In portrait mode: LCD_WIDTH=170 (screen width), LCD_HEIGHT=320 (screen height)
#define DEBUG_OVERLAY_HEIGHT  120     // Height of debug overlay area (9 lines including separator)
#define DEBUG_OVERLAY_Y     ((LCD_HEIGHT - DEBUG_OVERLAY_HEIGHT) / 2)  // Center vertically
#define DEBUG_OVERLAY_X     3         // Left margin
#define DEBUG_LINE_SPACING  12        // Pixels between lines (text size 1 = 8px + 4px gap)

// Expose overlay region for gauge clipping
int16_t debug_get_overlay_y() {
    return DEBUG_OVERLAY_Y - 3;  // Include border
}

int16_t debug_get_overlay_height() {
    return DEBUG_OVERLAY_HEIGHT + 6;  // Include border
}

// Track last drawn values to avoid unnecessary redraws
// (debug_overlay_drawn is defined at top of file for mode_cycle_next access)
static uint16_t last_tank1_raw = 0xFFFF;
static uint16_t last_tank2_raw = 0xFFFF;
static float last_tank1_voltage = -1;
static float last_tank2_voltage = -1;
static float last_tank1_resistance = -1;
static float last_tank2_resistance = -1;

void debug_draw_value(int16_t x, int16_t y, const char* label, float value, int decimals) {
    display_set_text_size(1);
    display_set_text_color(UI_COLOR_DEBUG);
    display_set_cursor(x, y);
    display_print(label);
    display_print(": ");
    
    if (decimals == 0) {
        display_print_int((int)value);
    } else {
        display_print_float(value, decimals);
    }
}

// Clear a single text line area completely (label + value)
static void debug_clear_line(int16_t x, int16_t y, int16_t width) {
    // Clear the entire line (including label) - simpler and more reliable
    display_fill_rect(x, y, width, 10, UI_COLOR_BACKGROUND);
}

void debug_draw_overlay(uint16_t tank1_raw, float tank1_voltage, float tank1_resistance,
                        uint16_t tank2_raw, float tank2_voltage, float tank2_resistance) {
    // Check if any values changed (with some tolerance for floats)
    bool values_changed = !debug_overlay_drawn ||
        tank1_raw != last_tank1_raw ||
        tank2_raw != last_tank2_raw ||
        (int)(tank1_voltage * 100) != (int)(last_tank1_voltage * 100) ||
        (int)(tank2_voltage * 100) != (int)(last_tank2_voltage * 100) ||
        (int)tank1_resistance != (int)last_tank1_resistance ||
        (int)tank2_resistance != (int)last_tank2_resistance;
    
    if (!values_changed) {
        return;  // Nothing changed, skip redraw
    }
    
    // Only do full clear on first draw, otherwise clear just the value areas
    if (!debug_overlay_drawn) {
        // First draw - clear entire region and draw border
        display_fill_rect(0, DEBUG_OVERLAY_Y - 3, LCD_WIDTH, DEBUG_OVERLAY_HEIGHT + 6, UI_COLOR_BACKGROUND);
        display_draw_rect(0, DEBUG_OVERLAY_Y - 3, LCD_WIDTH, DEBUG_OVERLAY_HEIGHT + 6, UI_COLOR_BORDER);
    }
    
    int16_t y = DEBUG_OVERLAY_Y;
    int16_t x1 = DEBUG_OVERLAY_X + 3;
    int16_t x2 = LCD_WIDTH / 2 + 5;  // Second column starts at screen center + margin
    int16_t col_width = LCD_WIDTH / 2 - 10;
    
    // Header row with pin numbers (static, only draw once)
    if (!debug_overlay_drawn) {
        display_set_text_size(1);
        display_set_text_color(UI_COLOR_DEBUG);
        display_set_cursor(x1, y);
        display_print("T1 GPIO ");
        display_print_int(PIN_TANK1_ADC);
        display_set_cursor(x2, y);
        display_print("T2 GPIO ");
        display_print_int(PIN_TANK2_ADC);
    }
    y += DEBUG_LINE_SPACING;
    
    // Raw ADC values - clear and redraw only if changed
    if (!debug_overlay_drawn || tank1_raw != last_tank1_raw) {
        debug_clear_line(x1, y, col_width);
        debug_draw_value(x1, y, "ADC", tank1_raw, 0);
    }
    if (!debug_overlay_drawn || tank2_raw != last_tank2_raw) {
        debug_clear_line(x2, y, col_width);
        debug_draw_value(x2, y, "ADC", tank2_raw, 0);
    }
    y += DEBUG_LINE_SPACING;
    
    // Voltage values
    if (!debug_overlay_drawn || (int)(tank1_voltage * 100) != (int)(last_tank1_voltage * 100)) {
        debug_clear_line(x1, y, col_width);
        debug_draw_value(x1, y, "V", tank1_voltage, 2);
    }
    if (!debug_overlay_drawn || (int)(tank2_voltage * 100) != (int)(last_tank2_voltage * 100)) {
        debug_clear_line(x2, y, col_width);
        debug_draw_value(x2, y, "V", tank2_voltage, 2);
    }
    y += DEBUG_LINE_SPACING;
    
    // Resistance values
    if (!debug_overlay_drawn || (int)tank1_resistance != (int)last_tank1_resistance) {
        debug_clear_line(x1, y, col_width);
        debug_draw_value(x1, y, "R", tank1_resistance, 0);
    }
    if (!debug_overlay_drawn || (int)tank2_resistance != (int)last_tank2_resistance) {
        debug_clear_line(x2, y, col_width);
        debug_draw_value(x2, y, "R", tank2_resistance, 0);
    }
    y += DEBUG_LINE_SPACING;
    
    // Percentage values
    float pct1 = (tank1_resistance > 0) ? 100.0f * (SENDER_R_EMPTY - tank1_resistance) / (SENDER_R_EMPTY - SENDER_R_FULL) : 0;
    float pct2 = (tank2_resistance > 0) ? 100.0f * (SENDER_R_EMPTY - tank2_resistance) / (SENDER_R_EMPTY - SENDER_R_FULL) : 0;
    if (pct1 < 0) pct1 = 0; if (pct1 > 100) pct1 = 100;
    if (pct2 < 0) pct2 = 0; if (pct2 > 100) pct2 = 100;
    
    if (!debug_overlay_drawn || (int)tank1_resistance != (int)last_tank1_resistance) {
        debug_clear_line(x1, y, col_width);
        display_set_text_color(UI_COLOR_YELLOW);
        debug_draw_value(x1, y, "%", pct1, 0);
    }
    if (!debug_overlay_drawn || (int)tank2_resistance != (int)last_tank2_resistance) {
        debug_clear_line(x2, y, col_width);
        display_set_text_color(UI_COLOR_YELLOW);
        debug_draw_value(x2, y, "%", pct2, 0);
    }
    y += DEBUG_LINE_SPACING;
    
    // Separator line and Variable Brightness header
    if (!debug_overlay_drawn) {
        // Draw horizontal separator line
        display_draw_hline(x1, y + 4, LCD_WIDTH - x1 - 6, UI_COLOR_BORDER);
    }
    y += DEBUG_LINE_SPACING;
    
    // Variable Brightness header
    if (!debug_overlay_drawn) {
        display_set_text_size(1);
        display_set_text_color(UI_COLOR_DEBUG);
        display_set_cursor(x1, y);
        display_print("Var Brightness");
    }
    y += DEBUG_LINE_SPACING;
    
    // Enable status line with GPIO on right
    if (!debug_overlay_drawn) {
        display_set_text_size(1);
        display_set_text_color(UI_COLOR_DEBUG);
        display_set_cursor(x1, y);
        display_print("Enable: ");
        if (brightness_is_auto_enabled()) {
            display_print("true");
        } else {
            display_print("false");
        }
        display_set_cursor(x2, y);
        display_print("GPIO ");
        display_print_int(PIN_BRIGHTNESS_ADC);
    }
    y += DEBUG_LINE_SPACING;
    
    // Brightness line 2: Raw ADC value and ADC pin voltage (always update)
    {
        uint16_t bri_raw = brightness_read_raw();
        // Calculate voltage at ADC pin (before voltage divider math)
        float adc_voltage = ((float)bri_raw / 4095.0f) * 3.3f;
        
        debug_clear_line(x1, y, col_width);
        display_set_text_color(UI_COLOR_DEBUG);
        display_set_cursor(x1, y);
        display_print("ADC: ");
        display_print_int(bri_raw);
        
        debug_clear_line(x2, y, col_width);
        display_set_cursor(x2, y);
        display_print("Vpin: ");
        display_print_float(adc_voltage, 2);
    }
    y += DEBUG_LINE_SPACING;
    
    // Brightness line 3: Interpreted input voltage and percentage (always update)
    {
        float bri_voltage = brightness_read_voltage();
        // Calculate brightness percentage (0V = 0%, 14V = 100%)
        float bri_pct = (bri_voltage / 14.0f) * 100.0f;
        if (bri_pct < 0) bri_pct = 0;
        if (bri_pct > 100) bri_pct = 100;
        
        debug_clear_line(x1, y, col_width);
        display_set_text_color(UI_COLOR_DEBUG);
        display_set_cursor(x1, y);
        display_print("Vin: ");
        display_print_float(bri_voltage, 1);
        
        debug_clear_line(x2, y, col_width);
        display_set_cursor(x2, y);
        display_print("%: ");
        display_print_int((int)bri_pct);
    }
    
    // Update cached values
    last_tank1_raw = tank1_raw;
    last_tank2_raw = tank2_raw;
    last_tank1_voltage = tank1_voltage;
    last_tank2_voltage = tank2_voltage;
    last_tank1_resistance = tank1_resistance;
    last_tank2_resistance = tank2_resistance;
    debug_overlay_drawn = true;
}
