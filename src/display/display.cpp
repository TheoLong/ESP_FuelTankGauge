#include "display.h"
#include <stdint.h>
#include <string.h>
#include <Arduino.h>

#ifndef NATIVE_BUILD
// LovyanGFX display instance
static LGFX *gfx = nullptr;

bool display_init() {
    Serial.println("[DISPLAY] Starting display initialization...");
    Serial.println("[DISPLAY] Pin Configuration:");
    Serial.print("  MOSI: GPIO"); Serial.println(LCD_PIN_MOSI);
    Serial.print("  CLK:  GPIO"); Serial.println(LCD_PIN_CLK);
    Serial.print("  DC:   GPIO"); Serial.println(LCD_PIN_DC);
    Serial.print("  CS:   GPIO"); Serial.println(LCD_PIN_CS);
    Serial.print("  RST:  GPIO"); Serial.println(LCD_PIN_RST);
    Serial.print("  BL:   GPIO"); Serial.println(LCD_PIN_BL);
    Serial.print("  Rotation: "); Serial.println(LCD_ROTATION);
    Serial.print("  Width: "); Serial.print(LCD_WIDTH); Serial.print(" Height: "); Serial.println(LCD_HEIGHT);
    
    // Manually turn on backlight first (ACTIVE LOW!)
    Serial.println("[DISPLAY] Manually setting backlight pin LOW (active low)");
    pinMode(LCD_PIN_BL, OUTPUT);
    digitalWrite(LCD_PIN_BL, LOW);  // Active LOW - LOW turns backlight ON
    
    Serial.println("[DISPLAY] Creating LGFX object...");
    gfx = new LGFX();
    
    if (!gfx) {
        Serial.println("[DISPLAY] ERROR: Failed to create LGFX object!");
        return false;
    }
    Serial.println("[DISPLAY] LGFX object created successfully");
    
    // Initialize display
    Serial.println("[DISPLAY] Calling gfx->init()...");
    gfx->init();
    Serial.println("[DISPLAY] gfx->init() completed");
    
    Serial.print("[DISPLAY] Setting rotation to "); Serial.println(LCD_ROTATION);
    gfx->setRotation(LCD_ROTATION);
    
    // Turn on backlight via LovyanGFX as well
    Serial.println("[DISPLAY] Setting backlight to 255 via LovyanGFX");
    gfx->setBrightness(255);
    
    // Add a small delay for display stabilization
    delay(50);
    
    // Clear to background color
    Serial.println("[DISPLAY] Clearing screen to background color");
    display_clear(UI_COLOR_BACKGROUND);
    
    Serial.println("[DISPLAY] Initialization complete!");
    return true;
}

void display_clear(uint16_t color) {
    if (gfx) {
        Serial.print("[DISPLAY] fillScreen color: 0x");
        Serial.println(color, HEX);
        gfx->fillScreen(color);
    } else {
        Serial.println("[DISPLAY] WARNING: gfx is null in display_clear!");
    }
}

void display_set_brightness(uint8_t level) {
    if (gfx) {
        gfx->setBrightness(level);
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
void display_set_brightness(uint8_t level) { (void)level; }
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
