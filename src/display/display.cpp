#include "display.h"
#include <stdint.h>
#include <string.h>

#ifndef NATIVE_BUILD
// LovyanGFX display instance
static LGFX *gfx = nullptr;

bool display_init() {
    gfx = new LGFX();
    
    if (!gfx) {
        return false;
    }
    
    // Initialize display
    gfx->init();
    gfx->setRotation(LCD_ROTATION);
    
    // Turn on backlight
    gfx->setBrightness(255);
    
    // Clear to background color
    display_clear(UI_COLOR_BACKGROUND);
    
    return true;
}

void display_clear(uint16_t color) {
    if (gfx) {
        gfx->fillScreen(color);
    }
}

void display_backlight_on() {
    if (gfx) {
        gfx->setBrightness(255);
    }
}

void display_backlight_off() {
    if (gfx) {
        gfx->setBrightness(0);
    }
}

void display_fill_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (gfx) {
        gfx->fillRect(x, y, w, h, color);
    }
}

void display_draw_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (gfx) {
        gfx->drawRect(x, y, w, h, color);
    }
}

void display_draw_hline(int16_t x, int16_t y, int16_t w, uint16_t color) {
    if (gfx) {
        gfx->drawFastHLine(x, y, w, color);
    }
}

void display_set_cursor(int16_t x, int16_t y) {
    if (gfx) {
        gfx->setCursor(x, y);
    }
}

void display_set_text_color(uint16_t color) {
    if (gfx) {
        gfx->setTextColor(color);
    }
}

void display_set_text_size(uint8_t size) {
    if (gfx) {
        gfx->setTextSize(size);
    }
}

void display_print(const char* text) {
    if (gfx) {
        gfx->print(text);
    }
}

void display_print_int(int value) {
    if (gfx) {
        gfx->print(value);
    }
}

void display_print_float(float value, int decimals) {
    if (gfx) {
        gfx->print(value, decimals);
    }
}

int16_t display_get_text_width(const char* text, uint8_t size) {
    // Base font is 6 pixels wide per character
    return strlen(text) * 6 * size;
}

#else
// Native build stubs for testing
bool display_init() { return true; }
void display_clear(uint16_t color) { (void)color; }
void display_backlight_on() {}
void display_backlight_off() {}
void display_fill_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    (void)x; (void)y; (void)w; (void)h; (void)color;
}
void display_draw_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    (void)x; (void)y; (void)w; (void)h; (void)color;
}
void display_draw_hline(int16_t x, int16_t y, int16_t w, uint16_t color) {
    (void)x; (void)y; (void)w; (void)color;
}
void display_set_cursor(int16_t x, int16_t y) { (void)x; (void)y; }
void display_set_text_color(uint16_t color) { (void)color; }
void display_set_text_size(uint8_t size) { (void)size; }
void display_print(const char* text) { (void)text; }
void display_print_int(int value) { (void)value; }
void display_print_float(float value, int decimals) { (void)value; (void)decimals; }
int16_t display_get_text_width(const char* text, uint8_t size) {
    int len = 0;
    while (text[len]) len++;
    return len * 6 * size;
}
#endif
