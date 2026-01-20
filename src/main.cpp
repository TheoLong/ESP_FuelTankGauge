/**
 * ESP32-C6 Dual Fuel Tank Gauge
 * 
 * Main application entry point for the fuel tank gauge display.
 * Supports three operating modes (switchable at runtime via BOOT button):
 *   - NORMAL: Real ADC readings from fuel senders
 *   - DEMO: Simulated cycling values for testing without hardware
 *   - DEBUG: Real ADC readings with diagnostic overlay
 * 
 * Hardware: Waveshare ESP32-C6-LCD-1.9
 * Display: ST7789 170x320 LCD in portrait orientation
 * Sensors: 33-240 ohm fuel tank senders via voltage divider
 * 
 * Press BOOT button (GPIO9) to cycle modes: Normal → Demo → Debug → Normal
 */

#include <Arduino.h>
#include "config.h"
#include "display/display.h"
#include "display/gauge.h"
#include "display/brightness.h"
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
static bool force_redraw = false;  // Force full redraw on mode change

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
    
    // Wait for serial to be ready (important for USB CDC)
    delay(2000);  // Give time for USB serial to connect
    
    Serial.println();
    Serial.println("========================================");
    Serial.println("  ESP32-C6 Dual Fuel Tank Gauge");
    Serial.println("  Debug Build - " __DATE__ " " __TIME__);
    Serial.println("========================================");
    Serial.println();
    
    // Print chip info
    Serial.print("[BOOT] ESP32-C6 Chip Revision: ");
    Serial.println(ESP.getChipRevision());
    Serial.print("[BOOT] CPU Frequency: ");
    Serial.print(ESP.getCpuFreqMHz());
    Serial.println(" MHz");
    Serial.print("[BOOT] Free Heap: ");
    Serial.print(ESP.getFreeHeap());
    Serial.println(" bytes");
    Serial.println();
    
    // Initialize mode system
    mode_init();
    Serial.print("Starting in mode: ");
    Serial.println(mode_get_name(mode_get_current()));
    
    // Initialize BOOT button for mode switching
    button_init();
    Serial.println("BOOT button initialized (press to cycle modes)");
    
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
    
    // Initialize fuel sensors (needed for Normal and Debug modes)
    Serial.print("Initializing fuel sensors... ");
    fuel_sensor_init();
    Serial.println("OK");
    
    // Initialize brightness control (auto-dimming)
    brightness_init();
    
    // Initialize demo mode
    Serial.println("Initializing demo mode...");
    demo_mode_init();
    
    // Calculate gauge positions for portrait mode (170 wide x 320 tall)
    // In portrait mode (rotation 0): 
    //   - Screen width = 170 pixels (narrow)
    //   - Screen height = 320 pixels (tall)
    int16_t screen_width = LCD_WIDTH;    // 170 pixels
    int16_t screen_height = LCD_HEIGHT;  // 320 pixels
    
    Serial.print("[LAYOUT] Screen dimensions: ");
    Serial.print(screen_width);
    Serial.print("x");
    Serial.println(screen_height);
    
    // Position gauges side by side, centered horizontally
    // Each gauge is GAUGE_WIDTH wide with a gap between them
    int16_t gap_between = 20;  // Gap between the two gauges
    int16_t total_gauge_width = GAUGE_WIDTH * 2 + gap_between;
    int16_t start_x = (screen_width - total_gauge_width) / 2;
    
    tank1_x = start_x;
    tank2_x = start_x + GAUGE_WIDTH + gap_between;
    
    // Vertical position - Layout: [Gallons text] [Bar] [Percentage text]
    // Maximize bar usage - minimal margins
    int text_height = 18;  // Height for text rows (size 2 = ~16px + small padding)
    int top_margin = 1;    // Small margin from top of screen
    int bottom_margin = 2; // Small margin from bottom
    int total_bar_height = GAUGE_SEGMENT_COUNT * (GAUGE_SEGMENT_HEIGHT + GAUGE_SEGMENT_GAP) - GAUGE_SEGMENT_GAP;
    
    // Bar starts after top margin + top text
    gauge_y = top_margin + text_height;
    
    Serial.print("[LAYOUT] Bar starts at y=");
    Serial.print(gauge_y);
    Serial.print(" total_bar_height=");
    Serial.print(total_bar_height);
    Serial.print(" bar ends at y=");
    Serial.println(gauge_y + total_bar_height);
    
    Serial.print("Gauge positions - Tank1: (");
    Serial.print(tank1_x);
    Serial.print(", ");
    Serial.print(gauge_y);
    Serial.print(") Tank2: (");
    Serial.print(tank2_x);
    Serial.print(", ");
    Serial.print(gauge_y);
    Serial.println(")");
    
    Serial.print("[LAYOUT] GAUGE_WIDTH=");
    Serial.print(GAUGE_WIDTH);
    Serial.print(" total_gauge_width=");
    Serial.print(total_gauge_width);
    Serial.print(" start_x=");
    Serial.println(start_x);
    
    Serial.print("[LAYOUT] total_bar_height=");
    Serial.print(total_bar_height);
    Serial.print(" gauge_y=");
    Serial.println(gauge_y);    
    Serial.println("Setup complete!");
    Serial.println();
}

// ============================================================================
// Main Loop
// ============================================================================

void loop() {
    unsigned long now = millis();
    
    // ========================================================================
    // Update Auto-Brightness (if enabled)
    // ========================================================================
    brightness_update();
    
    // ========================================================================
    // Check for BOOT Button Press (Mode Switch)
    // ========================================================================
    
    if (button_check_press()) {
        OperatingMode new_mode = mode_cycle_next();
        Serial.print("Mode changed to: ");
        Serial.println(mode_get_name(new_mode));
        
        // Force a full redraw when mode changes
        force_redraw = true;
        
        // Reset demo mode when switching to it
        if (new_mode == OP_MODE_DEMO) {
            demo_mode_init();
        }
    }
    
    // Rate limit updates
    if (now - last_update_time < UPDATE_INTERVAL_MS && initial_draw_done && !force_redraw) {
        return;
    }
    last_update_time = now;
    
    // ========================================================================
    // Read/Update Tank Values Based on Current Mode
    // ========================================================================
    
    OperatingMode current = mode_get_current();
    
    // Store sensor readings for debug display (used after gauge update)
    static FuelReading debug_reading1;
    static FuelReading debug_reading2;
    
    if (current == OP_MODE_DEMO) {
        // Demo mode: Use simulated cycling values
        demo_mode_update(&tank1_percent, &tank2_percent);
    } else {
        // Normal or Debug mode: Read real sensors
        debug_reading1 = fuel_sensor_read_averaged(1, ADC_SAMPLES);
        debug_reading2 = fuel_sensor_read_averaged(2, ADC_SAMPLES);
        
        tank1_percent = debug_reading1.percent;
        tank2_percent = debug_reading2.percent;
    }
    
    // ========================================================================
    // Update Display
    // ========================================================================
    
    if (!initial_draw_done || force_redraw) {
        // First draw or mode change - render everything
        display_clear(UI_COLOR_BACKGROUND);
        gauge_draw(tank1_x, gauge_y, tank1_percent, 1);
        gauge_draw(tank2_x, gauge_y, tank2_percent, 2);
        
        prev_tank1_percent = tank1_percent;
        prev_tank2_percent = tank2_percent;
        initial_draw_done = true;
        force_redraw = false;
        
        Serial.println("Display redrawn");
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
    
    // ========================================================================
    // Debug Mode Overlay (drawn AFTER gauge to prevent flashing)
    // ========================================================================
    
    if (current == OP_MODE_DEBUG) {
        debug_draw_overlay(
            debug_reading1.raw_adc, debug_reading1.voltage, debug_reading1.resistance,
            debug_reading2.raw_adc, debug_reading2.voltage, debug_reading2.resistance
        );
    }
    
    // Debug output to serial (in Demo or Debug modes)
    static unsigned long last_serial_print = 0;
    if (now - last_serial_print >= 1000) {
        last_serial_print = now;
        Serial.print("[");
        Serial.print(mode_get_name(mode_get_current()));
        Serial.print("] Tank1: ");
        Serial.print(tank1_percent, 1);
        Serial.print("% | Tank2: ");
        Serial.print(tank2_percent, 1);
        Serial.println("%");
    }
}
