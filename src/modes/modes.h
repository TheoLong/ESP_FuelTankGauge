#ifndef MODES_H
#define MODES_H

#include "config.h"
#include <stdint.h>

// ============================================================================
// Operating Mode Enum
// ============================================================================

typedef enum {
    OP_MODE_NORMAL = 0,   // Real ADC readings from fuel senders
    OP_MODE_DEMO = 1,     // Simulated cycling values for testing
    OP_MODE_DEBUG = 2     // Real ADC readings with diagnostic overlay
} OperatingMode;

// ============================================================================
// Runtime Mode Management
// ============================================================================

/**
 * @brief Initialize the mode system with default mode
 */
void mode_init();

/**
 * @brief Get the current operating mode
 */
OperatingMode mode_get_current();

/**
 * @brief Set the operating mode
 * @param mode The new operating mode
 */
void mode_set(OperatingMode mode);

/**
 * @brief Cycle to the next mode (Normal -> Debug -> Demo w/brightness -> Normal)
 * In Demo mode, cycles through brightness levels before returning to Normal
 * @return The new mode after cycling
 */
OperatingMode mode_cycle_next();

/**
 * @brief Get the mode name as a string
 */
const char* mode_get_name(OperatingMode mode);

/**
 * @brief Get current demo brightness step (0 = brightest)
 */
int demo_get_brightness_step();

/**
 * @brief Get total number of demo brightness levels
 */
int demo_get_brightness_levels();

// ============================================================================
// Button Handling
// ============================================================================

/**
 * @brief Initialize the BOOT button for mode switching
 */
void button_init();

/**
 * @brief Check for button press (with debounce)
 * @return true if button was pressed (falling edge detected)
 */
bool button_check_press();

// ============================================================================
// Debug Overlay Region (for gauge clipping)
// ============================================================================

/**
 * @brief Get the Y start position of the debug overlay
 */
int16_t debug_get_overlay_y();

/**
 * @brief Get the height of the debug overlay
 */
int16_t debug_get_overlay_height();

// ============================================================================
// Demo Mode - Simulated cycling values for testing without sensors
// ============================================================================

/**
 * @brief Initialize demo mode state
 */
void demo_mode_init();

/**
 * @brief Update demo mode values (call each frame)
 * @param tank1_percent Pointer to tank 1 percentage output
 * @param tank2_percent Pointer to tank 2 percentage output
 */
void demo_mode_update(float* tank1_percent, float* tank2_percent);

// ============================================================================
// Debug Mode - Real ADC readings plus diagnostic overlay
// ============================================================================

/**
 * @brief Draw debug overlay with raw sensor data
 * @param tank1_raw Raw ADC value for tank 1
 * @param tank1_voltage Voltage for tank 1
 * @param tank1_resistance Resistance for tank 1
 * @param tank2_raw Raw ADC value for tank 2
 * @param tank2_voltage Voltage for tank 2
 * @param tank2_resistance Resistance for tank 2
 */
void debug_draw_overlay(uint16_t tank1_raw, float tank1_voltage, float tank1_resistance,
                        uint16_t tank2_raw, float tank2_voltage, float tank2_resistance);

#endif // MODES_H
