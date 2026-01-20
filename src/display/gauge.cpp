#include "gauge.h"
#include "display.h"
#include "../modes/modes.h"

#ifndef NATIVE_BUILD
#include <Arduino.h>
#endif

// Check if a Y range overlaps with the debug overlay (only in debug mode)
static bool overlaps_debug_region(int16_t y_start, int16_t height) {
    if (mode_get_current() != OP_MODE_DEBUG) {
        return false;
    }
    int16_t debug_y = debug_get_overlay_y();
    int16_t debug_h = debug_get_overlay_height();
    int16_t y_end = y_start + height;
    int16_t debug_end = debug_y + debug_h;
    
    // Check if ranges overlap
    return (y_start < debug_end && y_end > debug_y);
}

// Get the static color for a segment based on its position (not fill level)
// Bottom segments are red, middle are yellow, top are green
static uint16_t get_segment_color(int segment_index) {
    // segment_index 0 = bottom, GAUGE_SEGMENT_COUNT-1 = top
    float segment_percent = ((float)segment_index / (float)GAUGE_SEGMENT_COUNT) * 100.0f;
    
    if (segment_percent < FUEL_THRESHOLD_RED) {
        return UI_COLOR_RED;
    } else if (segment_percent < FUEL_THRESHOLD_YELLOW) {
        return UI_COLOR_YELLOW;
    } else {
        return UI_COLOR_GREEN;
    }
}

uint16_t gauge_get_color_for_percent(float percent) {
    if (percent < 0.0f) percent = 0.0f;
    if (percent > 100.0f) percent = 100.0f;
    
    if (percent <= FUEL_THRESHOLD_RED) {
        return UI_COLOR_RED;
    } else if (percent <= FUEL_THRESHOLD_YELLOW) {
        return UI_COLOR_YELLOW;
    } else {
        return UI_COLOR_GREEN;
    }
}

int gauge_get_filled_segments(float percent) {
    if (percent < 0.0f) percent = 0.0f;
    if (percent > 100.0f) percent = 100.0f;
    
    // Calculate number of segments to fill
    float segments_float = (percent / 100.0f) * GAUGE_SEGMENT_COUNT;
    int segments = (int)(segments_float + 0.5f);
    
    if (segments > GAUGE_SEGMENT_COUNT) segments = GAUGE_SEGMENT_COUNT;
    if (segments < 0) segments = 0;
    
    return segments;
}

void gauge_draw_label(int16_t x, int16_t y, int tank_number) {
    // Labels removed per user request - left/right position is obvious
    (void)x;
    (void)y;
    (void)tank_number;
}

void gauge_draw_gallons(int16_t x, int16_t y, float percent) {
    if (percent < 0.0f) percent = 0.0f;
    if (percent > 100.0f) percent = 100.0f;
    
    // Convert percentage to gallons
    int gallons = (int)((percent / 100.0f) * TANK_CAPACITY_GALLONS + 0.5f);
    if (gallons > TANK_CAPACITY_GALLONS) gallons = TANK_CAPACITY_GALLONS;
    if (gallons < 0) gallons = 0;
    
    // Clear area for text - smaller size to avoid overlap
    int16_t text_height = 16;  // Size 2 font height
    display_fill_rect(x, y, GAUGE_WIDTH, text_height, UI_COLOR_BACKGROUND);
    
    // Use size 2 for gallons display
    display_set_text_size(2);
    display_set_text_color(UI_COLOR_TEXT);
    
    // Format gallon string: "XXG" (compact to fit)
    char buf[8];
    int idx = 0;
    
    if (gallons >= 10) {
        buf[idx++] = '0' + (gallons / 10);
        buf[idx++] = '0' + (gallons % 10);
    } else {
        buf[idx++] = ' ';
        buf[idx++] = '0' + gallons;
    }
    buf[idx++] = 'G';
    buf[idx] = '\0';
    
    // Center text: 3 chars * 12 pixels (size 2) = 36 pixels
    int16_t text_width = 3 * 12;
    int16_t text_x = x + (GAUGE_WIDTH - text_width) / 2;
    
    display_set_cursor(text_x, y);
    display_print(buf);
}

void gauge_draw_percentage(int16_t x, int16_t y, float percent) {
    if (percent < 0.0f) percent = 0.0f;
    if (percent > 100.0f) percent = 100.0f;
    
    int pct_int = (int)(percent + 0.5f);
    if (pct_int > 100) pct_int = 100;
    
    // Clear area for text
    int16_t text_height = 16;
    display_fill_rect(x, y, GAUGE_WIDTH, text_height, UI_COLOR_BACKGROUND);
    
    display_set_text_size(2);
    display_set_text_color(UI_COLOR_TEXT);
    
    // Format: "XX%" or "100%"
    char buf[8];
    int idx = 0;
    
    if (pct_int >= 100) {
        buf[idx++] = '1';
        buf[idx++] = '0';
        buf[idx++] = '0';
    } else if (pct_int >= 10) {
        buf[idx++] = '0' + (pct_int / 10);
        buf[idx++] = '0' + (pct_int % 10);
    } else {
        buf[idx++] = ' ';
        buf[idx++] = '0' + pct_int;
    }
    buf[idx++] = '%';
    buf[idx] = '\0';
    
    // Center text
    int num_chars = (pct_int >= 100) ? 4 : 3;
    int16_t text_width = num_chars * 12;
    int16_t text_x = x + (GAUGE_WIDTH - text_width) / 2;
    
    display_set_cursor(text_x, y);
    display_print(buf);
}

void gauge_redraw_bar(int16_t x, int16_t y, float percent) {
    if (percent < 0.0f) percent = 0.0f;
    if (percent > 100.0f) percent = 100.0f;
    
    // Internal padding between border and segments (all sides)
    const int BORDER_PADDING = 1;
    
    // Segment drawing area (inside the padding)
    int16_t seg_draw_x = x + BORDER_PADDING;
    int16_t seg_draw_width = GAUGE_WIDTH - (BORDER_PADDING * 2);
    
    // Calculate total segment area dimensions (segments + gaps between them)
    int segment_area_height = GAUGE_SEGMENT_COUNT * (GAUGE_SEGMENT_HEIGHT + GAUGE_SEGMENT_GAP) - GAUGE_SEGMENT_GAP;
    
    // Total bar height including internal padding on top and bottom
    int total_bar_height = segment_area_height + (BORDER_PADDING * 2);
    
    // Border is 1 pixel outside the bar area
    int16_t border_top = y - 1;
    int16_t border_bottom = y + total_bar_height;
    int16_t border_left = x - 1;
    int16_t border_right = x + GAUGE_WIDTH;
    
    // Draw gauge frame (skip parts that overlap debug region)
    // Top border line
    if (!overlaps_debug_region(border_top, 1)) {
        display_draw_hline(border_left, border_top, GAUGE_WIDTH + 2, UI_COLOR_BORDER);
    }
    // Bottom border line
    if (!overlaps_debug_region(border_bottom, 1)) {
        display_draw_hline(border_left, border_bottom, GAUGE_WIDTH + 2, UI_COLOR_BORDER);
    }
    // Left and right borders - draw pixel by pixel to avoid debug region
    for (int16_t by = border_top; by <= border_bottom; by++) {
        if (!overlaps_debug_region(by, 1)) {
            display_fill_rect(border_left, by, 1, 1, UI_COLOR_BORDER);   // Left
            display_fill_rect(border_right, by, 1, 1, UI_COLOR_BORDER);  // Right
        }
    }
    
    // Clear the internal padding area (between border and segments)
    // Top padding (full width inside border)
    if (!overlaps_debug_region(y, BORDER_PADDING)) {
        display_fill_rect(x, y, GAUGE_WIDTH, BORDER_PADDING, UI_COLOR_BACKGROUND);
    }
    // Bottom padding (full width inside border)
    int16_t bottom_pad_y = y + BORDER_PADDING + segment_area_height;
    if (!overlaps_debug_region(bottom_pad_y, BORDER_PADDING)) {
        display_fill_rect(x, bottom_pad_y, GAUGE_WIDTH, BORDER_PADDING, UI_COLOR_BACKGROUND);
    }
    // Left padding (between left border and segments, full height of segment area)
    for (int16_t py = y + BORDER_PADDING; py < y + BORDER_PADDING + segment_area_height; py++) {
        if (!overlaps_debug_region(py, 1)) {
            display_fill_rect(x, py, BORDER_PADDING, 1, UI_COLOR_BACKGROUND);
        }
    }
    // Right padding (between segments and right border, full height of segment area)
    for (int16_t py = y + BORDER_PADDING; py < y + BORDER_PADDING + segment_area_height; py++) {
        if (!overlaps_debug_region(py, 1)) {
            display_fill_rect(x + GAUGE_WIDTH - BORDER_PADDING, py, BORDER_PADDING, 1, UI_COLOR_BACKGROUND);
        }
    }
    
    // Segments start after the top padding
    int16_t segment_start_y = y + BORDER_PADDING;
    
    // Calculate pixel-level fill
    // Total fillable pixels (excluding gaps)
    int total_fill_pixels = GAUGE_SEGMENT_COUNT * GAUGE_SEGMENT_HEIGHT;
    int filled_pixels = (int)((percent / 100.0f) * total_fill_pixels + 0.5f);
    if (filled_pixels > total_fill_pixels) filled_pixels = total_fill_pixels;
    if (filled_pixels < 0) filled_pixels = 0;
    
    int pixels_remaining = filled_pixels;
    
    // Draw each segment from bottom (segment 0) to top
    for (int seg = 0; seg < GAUGE_SEGMENT_COUNT; seg++) {
        // Calculate Y position for this segment (bottom-up)
        int segment_from_top = GAUGE_SEGMENT_COUNT - 1 - seg;
        int16_t seg_y = segment_start_y + (segment_from_top * (GAUGE_SEGMENT_HEIGHT + GAUGE_SEGMENT_GAP));
        
        // Skip segments that overlap with debug region in debug mode
        if (overlaps_debug_region(seg_y, GAUGE_SEGMENT_HEIGHT)) {
            if (pixels_remaining >= GAUGE_SEGMENT_HEIGHT) {
                pixels_remaining -= GAUGE_SEGMENT_HEIGHT;
            } else {
                pixels_remaining = 0;
            }
            continue;
        }
        
        // Get the static color for this segment position
        uint16_t seg_color = get_segment_color(seg);
        
        if (pixels_remaining >= GAUGE_SEGMENT_HEIGHT) {
            // Fully filled segment
            display_fill_rect(seg_draw_x, seg_y, seg_draw_width, GAUGE_SEGMENT_HEIGHT, seg_color);
            pixels_remaining -= GAUGE_SEGMENT_HEIGHT;
        } else if (pixels_remaining > 0) {
            // Partially filled segment - fill from bottom
            int filled_in_seg = pixels_remaining;
            int empty_in_seg = GAUGE_SEGMENT_HEIGHT - filled_in_seg;
            
            // Empty part (top of segment)
            if (empty_in_seg > 0) {
                display_fill_rect(seg_draw_x, seg_y, seg_draw_width, empty_in_seg, UI_COLOR_EMPTY);
            }
            // Filled part (bottom of segment)
            if (filled_in_seg > 0) {
                display_fill_rect(seg_draw_x, seg_y + empty_in_seg, seg_draw_width, filled_in_seg, seg_color);
            }
            
            pixels_remaining = 0;
        } else {
            // Empty segment
            display_fill_rect(seg_draw_x, seg_y, seg_draw_width, GAUGE_SEGMENT_HEIGHT, UI_COLOR_EMPTY);
        }
    }
}

void gauge_draw(int16_t x, int16_t y, float percent, int tank_number) {
    (void)tank_number;
    
    // y is top of the bar area (inside the border)
    // Layout: [Gallons text] [Border + padding + Bar + padding + Border] [Percentage text]
    
    const int BORDER_PADDING = 1;
    int segment_area_height = GAUGE_SEGMENT_COUNT * (GAUGE_SEGMENT_HEIGHT + GAUGE_SEGMENT_GAP) - GAUGE_SEGMENT_GAP;
    int total_bar_height = segment_area_height + (BORDER_PADDING * 2);
    
    // Draw gallons ABOVE the bar (skip if overlaps debug region)
    // Gallons text ends 2 pixels above the border (border is at y-1)
    if (!overlaps_debug_region(y - 18, 16)) {
        gauge_draw_gallons(x, y - 18, percent);
    }
    
    // Draw the bar gauge (handles debug region internally)
    gauge_redraw_bar(x, y, percent);
    
    // Draw percentage BELOW the bar (skip if overlaps debug region)
    // Border bottom is at y + total_bar_height, text starts 6 pixels below for more spacing
    int16_t pct_y = y + total_bar_height + 6;
    if (!overlaps_debug_region(pct_y, 16)) {
        gauge_draw_percentage(x, pct_y, percent);
    }
}

bool gauge_update_if_changed(int16_t x, int16_t y, float old_percent, 
                              float new_percent, int tank_number) {
    (void)tank_number;
    
    // Calculate pixel-level fill for both
    int total_fill_pixels = GAUGE_SEGMENT_COUNT * GAUGE_SEGMENT_HEIGHT;
    int old_pixels = (int)((old_percent / 100.0f) * total_fill_pixels + 0.5f);
    int new_pixels = (int)((new_percent / 100.0f) * total_fill_pixels + 0.5f);
    
    // Check if displayed gallon value changed
    int old_gallons = (int)((old_percent / 100.0f) * TANK_CAPACITY_GALLONS + 0.5f);
    int new_gallons = (int)((new_percent / 100.0f) * TANK_CAPACITY_GALLONS + 0.5f);
    
    // Check if displayed percentage changed
    int old_pct = (int)(old_percent + 0.5f);
    int new_pct = (int)(new_percent + 0.5f);
    
    if (old_pixels != new_pixels || old_gallons != new_gallons || old_pct != new_pct) {
        gauge_redraw_bar(x, y, new_percent);
        
        // Update gallon display ABOVE bar
        gauge_draw_gallons(x, y - 18, new_percent);
        
        // Update percentage display BELOW bar
        const int BORDER_PADDING = 1;
        int segment_area_height = GAUGE_SEGMENT_COUNT * (GAUGE_SEGMENT_HEIGHT + GAUGE_SEGMENT_GAP) - GAUGE_SEGMENT_GAP;
        int total_bar_height = segment_area_height + (BORDER_PADDING * 2);
        gauge_draw_percentage(x, y + total_bar_height + 6, new_percent);
        
        return true;
    }
    
    return false;
}

void gauge_draw_full_ui(float tank1_percent, float tank2_percent) {
    // Clear screen
    display_clear(UI_COLOR_BACKGROUND);
    
    // Calculate gauge positions (centered in portrait mode)
    // Tank 1 on left half, Tank 2 on right half
    int16_t center_gap = 20;
    int16_t tank1_x = (LCD_HEIGHT / 2 - GAUGE_WIDTH) / 2;
    int16_t tank2_x = LCD_HEIGHT / 2 + (LCD_HEIGHT / 2 - GAUGE_WIDTH) / 2;
    
    // Vertical position - centered
    int total_bar_height = GAUGE_SEGMENT_COUNT * (GAUGE_SEGMENT_HEIGHT + GAUGE_SEGMENT_GAP) - GAUGE_SEGMENT_GAP;
    int total_element_height = UI_LABEL_OFFSET + total_bar_height + UI_PERCENT_OFFSET + (UI_FONT_SIZE_PERCENT * 8);
    int16_t gauge_y = (LCD_WIDTH - total_element_height) / 2 + UI_LABEL_OFFSET;
    
    // Draw both gauges
    gauge_draw(tank1_x, gauge_y, tank1_percent, 1);
    gauge_draw(tank2_x, gauge_y, tank2_percent, 2);
}
