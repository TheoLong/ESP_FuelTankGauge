#ifndef BRIGHTNESS_H
#define BRIGHTNESS_H

#include "config.h"
#include <stdint.h>

/**
 * @brief Initialize the brightness control system
 * Sets up ADC for brightness sensing if enabled
 */
void brightness_init();

/**
 * @brief Update brightness based on ADC reading (if enabled)
 * Call this periodically from the main loop
 * Respects BRIGHTNESS_UPDATE_MS rate limiting internally
 */
void brightness_update();

/**
 * @brief Set brightness manually (0-255)
 * @param level Brightness level (0=off, 255=max)
 */
void brightness_set(uint8_t level);

/**
 * @brief Get current brightness level
 * @return Current brightness (0-255)
 */
uint8_t brightness_get();

/**
 * @brief Read the raw ADC value from brightness sensor
 * @return Raw ADC value (0-4095)
 */
uint16_t brightness_read_raw();

/**
 * @brief Read the input voltage (before divider)
 * @return Calculated input voltage in volts
 */
float brightness_read_voltage();

/**
 * @brief Check if auto-brightness is enabled
 * @return true if BRIGHTNESS_AUTO_ENABLE is set
 */
bool brightness_is_auto_enabled();

#endif // BRIGHTNESS_H
