#ifndef MODES_H
#define MODES_H

#include "config.h"
#include <stdint.h>

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

/**
 * @brief Get current demo tank 1 percentage
 */
float demo_get_tank1_percent();

/**
 * @brief Get current demo tank 2 percentage  
 */
float demo_get_tank2_percent();

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

/**
 * @brief Clear the debug overlay area
 */
void debug_clear_overlay();

/**
 * @brief Draw a single debug value line
 * @param x X position
 * @param y Y position
 * @param label Label text
 * @param value Numeric value
 * @param decimals Number of decimal places (0 for integer)
 */
void debug_draw_value(int16_t x, int16_t y, const char* label, float value, int decimals);

#endif // MODES_H
