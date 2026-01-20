/**
 * ESP32-C6 Dual Fuel Tank Gauge
 * 
 * Main application entry point for the fuel tank gauge display.
 * Supports three operating modes (configured in config.h):
 *   - MODE_NORMAL: Real ADC readings from fuel senders
 *   - MODE_DEMO: Simulated cycling values for testing without hardware
 *   - MODE_DEBUG: Real ADC readings with diagnostic overlay
 * 
 * Hardware: Waveshare ESP32-C6-LCD-1.9
 * Display: ST7789 170x320 LCD in portrait orientation
 * Sensors: 33-240 ohm fuel tank senders via voltage divider
 */

#include <Arduino.h>
#include "config.h"
#include "display/display.h"
#include "display/gauge.h"
#include "sensor/fuel_sensor.h"
#include "modes/modes.h"

// ============================================================================
// Application State
// ============================================================================

static float tank1_percent = 0.0f;
static float tank2_percent = 0.0f;
static float prev_tank1_percent = -1.0f;  // Force initial draw
static float prev_tank2_percent = -1.0f;

static unsigned long last_update_time = 0;
static bool initial_draw_done = false;

// Gauge positions (calculated in setup)
static int16_t tank1_x = 0;
static int16_t tank2_x = 0;
static int16_t gauge_y = 0;

// ============================================================================
// Setup
// ============================================================================

void setup() {
    // Initialize serial for debugging
    Serial.begin(115200);
    delay(100);
    
    Serial.println();
    Serial.println("================================");
    Serial.println("ESP32-C6 Dual Fuel Tank Gauge");
    Serial.println("================================");
    
    // Print current mode
    #if MODE_DEMO
    Serial.println("Mode: DEMO (simulated values)");
    #elif MODE_DEBUG
    Serial.println("Mode: DEBUG (real ADC + diagnostics)");
    #else
    Serial.println("Mode: NORMAL (real ADC readings)");
    #endif
    
    // Initialize display
    Serial.print("Initializing display... ");
    if (!display_init()) {
        Serial.println("FAILED!");
        // Blink or indicate error somehow
        while (1) {
            delay(1000);
        }
    }
    Serial.println("OK");
    
    // Initialize fuel sensors (in Normal and Debug modes)
    #if !MODE_DEMO
    Serial.print("Initializing fuel sensors... ");
    fuel_sensor_init();
    Serial.println("OK");
    #endif
    
    // Initialize demo mode if enabled
    #if MODE_DEMO
    Serial.println("Initializing demo mode...");
    demo_mode_init();
    #endif
    
    // Calculate gauge positions (portrait mode: LCD_HEIGHT is width, LCD_WIDTH is height)
    // Screen is 170 wide x 320 tall in portrait
    int16_t screen_width = LCD_HEIGHT;   // 320 pixels
    int16_t screen_height = LCD_WIDTH;   // 170 pixels
    
    // Position gauges side by side, centered
    int16_t total_gauge_width = GAUGE_WIDTH * 2 + 40;  // Two gauges + gap
    int16_t start_x = (screen_width - total_gauge_width) / 2;
    
    tank1_x = start_x;
    tank2_x = start_x + GAUGE_WIDTH + 40;
    
    // Vertical position - centered with room for labels and percentages
    int total_bar_height = GAUGE_SEGMENT_COUNT * (GAUGE_SEGMENT_HEIGHT + GAUGE_SEGMENT_GAP) - GAUGE_SEGMENT_GAP;
    int total_element_height = UI_LABEL_OFFSET + total_bar_height + UI_PERCENT_OFFSET + 24;
    gauge_y = (screen_height - total_element_height) / 2 + UI_LABEL_OFFSET;
    
    Serial.print("Gauge positions - Tank1: (");
    Serial.print(tank1_x);
    Serial.print(", ");
    Serial.print(gauge_y);
    Serial.print(") Tank2: (");
    Serial.print(tank2_x);
    Serial.print(", ");
    Serial.print(gauge_y);
    Serial.println(")");
    
    Serial.println("Setup complete!");
    Serial.println();
}

// ============================================================================
// Main Loop
// ============================================================================

void loop() {
    unsigned long now = millis();
    
    // Rate limit updates
    if (now - last_update_time < UPDATE_INTERVAL_MS && initial_draw_done) {
        return;
    }
    last_update_time = now;
    
    // ========================================================================
    // Read/Update Tank Values Based on Mode
    // ========================================================================
    
    #if MODE_DEMO
    // Demo mode: Use simulated cycling values
    demo_mode_update(&tank1_percent, &tank2_percent);
    
    #else
    // Normal or Debug mode: Read real sensors
    FuelReading reading1 = fuel_sensor_read_averaged(1, ADC_SAMPLES);
    FuelReading reading2 = fuel_sensor_read_averaged(2, ADC_SAMPLES);
    
    tank1_percent = reading1.percent;
    tank2_percent = reading2.percent;
    
    // Debug mode: Also update overlay with raw values
    #if MODE_DEBUG
    debug_draw_overlay(
        reading1.raw_adc, reading1.voltage, reading1.resistance,
        reading2.raw_adc, reading2.voltage, reading2.resistance
    );
    #endif
    
    #endif // MODE_DEMO
    
    // ========================================================================
    // Update Display
    // ========================================================================
    
    if (!initial_draw_done) {
        // First draw - render everything
        display_clear(UI_COLOR_BACKGROUND);
        gauge_draw(tank1_x, gauge_y, tank1_percent, 1);
        gauge_draw(tank2_x, gauge_y, tank2_percent, 2);
        
        prev_tank1_percent = tank1_percent;
        prev_tank2_percent = tank2_percent;
        initial_draw_done = true;
        
        Serial.println("Initial display drawn");
    } else {
        // Subsequent draws - only update if changed
        bool tank1_changed = gauge_update_if_changed(tank1_x, gauge_y, 
                                                      prev_tank1_percent, 
                                                      tank1_percent, 1);
        bool tank2_changed = gauge_update_if_changed(tank2_x, gauge_y, 
                                                      prev_tank2_percent, 
                                                      tank2_percent, 2);
        
        if (tank1_changed) {
            prev_tank1_percent = tank1_percent;
        }
        if (tank2_changed) {
            prev_tank2_percent = tank2_percent;
        }
    }
    
    // Debug output to serial
    #if MODE_DEBUG || MODE_DEMO
    static unsigned long last_serial_print = 0;
    if (now - last_serial_print >= 1000) {
        last_serial_print = now;
        Serial.print("Tank1: ");
        Serial.print(tank1_percent, 1);
        Serial.print("% | Tank2: ");
        Serial.print(tank2_percent, 1);
        Serial.println("%");
    }
    #endif
}
