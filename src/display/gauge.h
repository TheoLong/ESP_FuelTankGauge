#ifndef GAUGE_H
#define GAUGE_H

#include "config.h"
#include <stdint.h>

/**
 * @brief Get the RGB565 color for a given fuel percentage
 * @param percent Fuel percentage (0-100)
 * @return RGB565 color code
 */
uint16_t gauge_get_color_for_percent(float percent);

/**
 * @brief Calculate how many segments to fill for a given percentage
 * @param percent Fuel percentage (0-100)
 * @return Number of segments to fill (0 to GAUGE_SEGMENT_COUNT)
 */
int gauge_get_filled_segments(float percent);

/**
 * @brief Draw a complete fuel gauge bar
 * @param x X position of gauge left edge
 * @param y Y position of gauge top edge
 * @param percent Current fuel percentage (0-100)
 * @param tank_number Tank identifier (1 or 2)
 */
void gauge_draw(int16_t x, int16_t y, float percent, int tank_number);

/**
 * @brief Draw the tank label above the gauge
 * @param x X position of gauge left edge
 * @param y Y position (above gauge)
 * @param tank_number Tank identifier (1 or 2)
 */
void gauge_draw_label(int16_t x, int16_t y, int tank_number);

/**
 * @brief Draw the percentage readout below the gauge
 * @param x X position of gauge left edge
 * @param y Y position (below gauge)
 * @param percent Current fuel percentage (0-100)
 */
void gauge_draw_percentage(int16_t x, int16_t y, float percent);

/**
 * @brief Update gauge display only if value changed significantly
 * @param x X position of gauge left edge
 * @param y Y position of gauge top edge
 * @param old_percent Previous percentage value
 * @param new_percent New percentage value
 * @param tank_number Tank identifier (1 or 2)
 * @return true if gauge was redrawn, false if no change needed
 */
bool gauge_update_if_changed(int16_t x, int16_t y, float old_percent, 
                              float new_percent, int tank_number);

/**
 * @brief Clear and redraw just the gauge bar area (for updates)
 * @param x X position of gauge left edge
 * @param y Y position of gauge top edge
 * @param percent Current fuel percentage (0-100)
 */
void gauge_redraw_bar(int16_t x, int16_t y, float percent);

#endif // GAUGE_H
