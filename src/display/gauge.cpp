#include "gauge.h"
#include "display.h"

#ifndef NATIVE_BUILD
#include <Arduino.h>
#endif

// Helper to convert percentage to segment Y position (bottom-up fill)
static int16_t get_segment_y(int16_t gauge_y, int segment_index) {
    // Segments fill from bottom (segment 0) to top (segment N-1)
    // gauge_y is top of gauge bar
    int segment_from_top = GAUGE_SEGMENT_COUNT - 1 - segment_index;
    return gauge_y + (segment_from_top * (GAUGE_SEGMENT_HEIGHT + GAUGE_SEGMENT_GAP));
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
    // Each segment represents 100/GAUGE_SEGMENT_COUNT percent
    float segments_float = (percent / 100.0f) * GAUGE_SEGMENT_COUNT;
    int segments = (int)(segments_float + 0.5f); // Round to nearest
    
    if (segments > GAUGE_SEGMENT_COUNT) segments = GAUGE_SEGMENT_COUNT;
    if (segments < 0) segments = 0;
    
    return segments;
}

void gauge_draw_label(int16_t x, int16_t y, int tank_number) {
    display_set_text_size(UI_FONT_SIZE_LABEL);
    display_set_text_color(UI_COLOR_TEXT);
    
    // Center the label above the gauge
    const char* label = (tank_number == 1) ? "TANK 1" : "TANK 2";
    int16_t text_width = display_get_text_width(label, UI_FONT_SIZE_LABEL);
    int16_t label_x = x + (GAUGE_WIDTH - text_width) / 2;
    
    display_set_cursor(label_x, y);
    display_print(label);
}

void gauge_draw_percentage(int16_t x, int16_t y, float percent) {
    if (percent < 0.0f) percent = 0.0f;
    if (percent > 100.0f) percent = 100.0f;
    
    // Clear a wider percentage area to avoid residue from "100%"
    // Use a larger width to ensure complete clearing
    int16_t clear_width = GAUGE_WIDTH + 20;  // Extra width for safety
    int16_t clear_x = x - 10;  // Start a bit to the left
    display_fill_rect(clear_x, y, clear_width, UI_FONT_SIZE_PERCENT * 8, UI_COLOR_BACKGROUND);
    
    display_set_text_size(UI_FONT_SIZE_PERCENT);
    display_set_text_color(UI_COLOR_TEXT);
    
    // Format percentage string
    char buf[8];
    int pct_int = (int)(percent + 0.5f);
    
    // Build string manually for native compatibility
    int idx = 0;
    if (pct_int >= 100) {
        buf[idx++] = '1';
        buf[idx++] = '0';
        buf[idx++] = '0';
    } else if (pct_int >= 10) {
        buf[idx++] = '0' + (pct_int / 10);
        buf[idx++] = '0' + (pct_int % 10);
    } else {
        buf[idx++] = '0' + pct_int;
    }
    buf[idx++] = '%';
    buf[idx] = '\0';
    
    int16_t text_width = display_get_text_width(buf, UI_FONT_SIZE_PERCENT);
    int16_t pct_x = x + (GAUGE_WIDTH - text_width) / 2;
    
    display_set_cursor(pct_x, y);
    display_print(buf);
}

void gauge_redraw_bar(int16_t x, int16_t y, float percent) {
    int filled_segments = gauge_get_filled_segments(percent);
    uint16_t fill_color = gauge_get_color_for_percent(percent);
    
    // Draw gauge frame
    int total_height = GAUGE_SEGMENT_COUNT * (GAUGE_SEGMENT_HEIGHT + GAUGE_SEGMENT_GAP) - GAUGE_SEGMENT_GAP;
    display_draw_rect(x - 2, y - 2, GAUGE_WIDTH + 4, total_height + 4, UI_COLOR_BORDER);
    
    // Draw each segment
    for (int i = 0; i < GAUGE_SEGMENT_COUNT; i++) {
        int16_t seg_y = get_segment_y(y, i);
        
        if (i < filled_segments) {
            // Filled segment with color based on overall percentage
            display_fill_rect(x, seg_y, GAUGE_WIDTH, GAUGE_SEGMENT_HEIGHT, fill_color);
        } else {
            // Empty segment
            display_fill_rect(x, seg_y, GAUGE_WIDTH, GAUGE_SEGMENT_HEIGHT, UI_COLOR_EMPTY);
        }
    }
}

void gauge_draw(int16_t x, int16_t y, float percent, int tank_number) {
    // Draw label above gauge
    gauge_draw_label(x, y - UI_LABEL_OFFSET, tank_number);
    
    // Draw the bar gauge
    gauge_redraw_bar(x, y, percent);
    
    // Draw percentage below gauge
    int total_height = GAUGE_SEGMENT_COUNT * (GAUGE_SEGMENT_HEIGHT + GAUGE_SEGMENT_GAP) - GAUGE_SEGMENT_GAP;
    gauge_draw_percentage(x, y + total_height + UI_PERCENT_OFFSET, percent);
}

bool gauge_update_if_changed(int16_t x, int16_t y, float old_percent, 
                              float new_percent, int tank_number) {
    // Check if change is significant enough to warrant redraw
    int old_segments = gauge_get_filled_segments(old_percent);
    int new_segments = gauge_get_filled_segments(new_percent);
    
    // Also check if color zone changed
    uint16_t old_color = gauge_get_color_for_percent(old_percent);
    uint16_t new_color = gauge_get_color_for_percent(new_percent);
    
    // Also check if displayed percentage value changed
    int old_display = (int)(old_percent + 0.5f);
    int new_display = (int)(new_percent + 0.5f);
    
    if (old_segments != new_segments || old_color != new_color || old_display != new_display) {
        gauge_redraw_bar(x, y, new_percent);
        
        // Update percentage display
        int total_height = GAUGE_SEGMENT_COUNT * (GAUGE_SEGMENT_HEIGHT + GAUGE_SEGMENT_GAP) - GAUGE_SEGMENT_GAP;
        gauge_draw_percentage(x, y + total_height + UI_PERCENT_OFFSET, new_percent);
        
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
