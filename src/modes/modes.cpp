#include "modes.h"
#include "../display/display.h"

#ifndef NATIVE_BUILD
#include <Arduino.h>
#endif

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

float demo_get_tank1_percent() {
    return demo_tank1;
}

float demo_get_tank2_percent() {
    return demo_tank2;
}

// ============================================================================
// Debug Mode Implementation
// ============================================================================

// Debug overlay position (bottom of screen)
#define DEBUG_OVERLAY_Y     (LCD_WIDTH - 80)  // In portrait mode, WIDTH is height
#define DEBUG_OVERLAY_X     5
// Use DEBUG_LINE_HEIGHT from config.h

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

void debug_clear_overlay() {
    display_fill_rect(0, DEBUG_OVERLAY_Y, LCD_HEIGHT, 80, UI_COLOR_BACKGROUND);
}

void debug_draw_overlay(uint16_t tank1_raw, float tank1_voltage, float tank1_resistance,
                        uint16_t tank2_raw, float tank2_voltage, float tank2_resistance) {
    // Clear overlay area first
    debug_clear_overlay();
    
    // Draw divider line
    display_draw_hline(0, DEBUG_OVERLAY_Y - 2, LCD_HEIGHT, UI_COLOR_BORDER);
    
    int16_t y = DEBUG_OVERLAY_Y;
    int16_t x1 = DEBUG_OVERLAY_X;
    int16_t x2 = LCD_HEIGHT / 2 + 5;
    
    // Header row
    display_set_text_size(1);
    display_set_text_color(UI_COLOR_DEBUG);
    display_set_cursor(x1, y);
    display_print("T1 DEBUG");
    display_set_cursor(x2, y);
    display_print("T2 DEBUG");
    y += DEBUG_LINE_HEIGHT;
    
    // Raw ADC values
    debug_draw_value(x1, y, "ADC", tank1_raw, 0);
    debug_draw_value(x2, y, "ADC", tank2_raw, 0);
    y += DEBUG_LINE_HEIGHT;
    
    // Voltage values
    debug_draw_value(x1, y, "V", tank1_voltage, 2);
    debug_draw_value(x2, y, "V", tank2_voltage, 2);
    y += DEBUG_LINE_HEIGHT;
    
    // Resistance values
    debug_draw_value(x1, y, "R", tank1_resistance, 1);
    debug_draw_value(x2, y, "R", tank2_resistance, 1);
    y += DEBUG_LINE_HEIGHT;
    
    // Mode indicator
    display_set_cursor(x1, y);
    display_set_text_color(UI_COLOR_YELLOW);
    display_print("MODE: DEBUG");
}
