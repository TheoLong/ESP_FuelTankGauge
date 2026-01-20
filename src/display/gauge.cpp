#include "gauge.h"
#include "display.h"

#ifndef NATIVE_BUILD
#include <Arduino.h>
#endif

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
    
    // Calculate total bar dimensions
    int total_height = GAUGE_SEGMENT_COUNT * (GAUGE_SEGMENT_HEIGHT + GAUGE_SEGMENT_GAP) - GAUGE_SEGMENT_GAP;
    
    // Draw gauge frame
    display_draw_rect(x - 2, y - 2, GAUGE_WIDTH + 4, total_height + 4, UI_COLOR_BORDER);
    
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
        int16_t seg_y = y + (segment_from_top * (GAUGE_SEGMENT_HEIGHT + GAUGE_SEGMENT_GAP));
        
        // Get the static color for this segment position
        uint16_t seg_color = get_segment_color(seg);
        
        if (pixels_remaining >= GAUGE_SEGMENT_HEIGHT) {
            // Fully filled segment
            display_fill_rect(x, seg_y, GAUGE_WIDTH, GAUGE_SEGMENT_HEIGHT, seg_color);
            pixels_remaining -= GAUGE_SEGMENT_HEIGHT;
        } else if (pixels_remaining > 0) {
            // Partially filled segment - fill from bottom
            int filled_in_seg = pixels_remaining;
            int empty_in_seg = GAUGE_SEGMENT_HEIGHT - filled_in_seg;
            
            // Empty part (top of segment)
            display_fill_rect(x, seg_y, GAUGE_WIDTH, empty_in_seg, UI_COLOR_EMPTY);
            // Filled part (bottom of segment)
            display_fill_rect(x, seg_y + empty_in_seg, GAUGE_WIDTH, filled_in_seg, seg_color);
            
            pixels_remaining = 0;
        } else {
            // Empty segment
            display_fill_rect(x, seg_y, GAUGE_WIDTH, GAUGE_SEGMENT_HEIGHT, UI_COLOR_EMPTY);
        }
    }
}

void gauge_draw(int16_t x, int16_t y, float percent, int tank_number) {
    (void)tank_number;
    
    // y is top of the bar area
    // Layout: [Gallons text] [Bar] [Percentage text]
    
    // Draw gallons ABOVE the bar (closer to bar)
    gauge_draw_gallons(x, y - 18, percent);
    
    // Draw the bar gauge
    gauge_redraw_bar(x, y, percent);
    
    // Draw percentage BELOW the bar (closer to bar)
    int total_height = GAUGE_SEGMENT_COUNT * (GAUGE_SEGMENT_HEIGHT + GAUGE_SEGMENT_GAP) - GAUGE_SEGMENT_GAP;
    gauge_draw_percentage(x, y + total_height + 3, percent);
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
        int total_height = GAUGE_SEGMENT_COUNT * (GAUGE_SEGMENT_HEIGHT + GAUGE_SEGMENT_GAP) - GAUGE_SEGMENT_GAP;
        gauge_draw_percentage(x, y + total_height + 3, new_percent);
        
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
