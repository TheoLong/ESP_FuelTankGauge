#ifndef DISPLAY_H
#define DISPLAY_H

#include "config.h"
#include <stdint.h>

#ifndef NATIVE_BUILD
#include <LovyanGFX.hpp>

// ST7789 LCD Configuration for Waveshare ESP32-C6-LCD-1.9
class LGFX : public lgfx::LGFX_Device {
    lgfx::Panel_ST7789 _panel_instance;
    lgfx::Bus_SPI _bus_instance;
    lgfx::Light_PWM _light_instance;
    
public:
    LGFX(void) {
        // Bus configuration
        {
            auto cfg = _bus_instance.config();
            cfg.spi_host = SPI2_HOST;
            cfg.spi_mode = 0;
            cfg.freq_write = 40000000;
            cfg.freq_read = 16000000;
            cfg.spi_3wire = false;
            cfg.use_lock = true;
            cfg.dma_channel = SPI_DMA_CH_AUTO;
            cfg.pin_sclk = LCD_PIN_CLK;
            cfg.pin_mosi = LCD_PIN_MOSI;
            cfg.pin_miso = -1;
            cfg.pin_dc = LCD_PIN_DC;
            _bus_instance.config(cfg);
            _panel_instance.setBus(&_bus_instance);
        }
        
        // Panel configuration
        {
            auto cfg = _panel_instance.config();
            cfg.pin_cs = LCD_PIN_CS;
            cfg.pin_rst = LCD_PIN_RST;
            cfg.pin_busy = -1;
            cfg.memory_width = 170;
            cfg.memory_height = 320;
            cfg.panel_width = 170;
            cfg.panel_height = 320;
            cfg.offset_x = 35;
            cfg.offset_y = 0;
            cfg.offset_rotation = 0;
            cfg.dummy_read_pixel = 8;
            cfg.dummy_read_bits = 1;
            cfg.readable = false;
            cfg.invert = true;
            cfg.rgb_order = false;
            cfg.dlen_16bit = false;
            cfg.bus_shared = false;
            _panel_instance.config(cfg);
        }
        
        // Backlight configuration - Waveshare board uses ACTIVE LOW backlight!
        {
            auto cfg = _light_instance.config();
            cfg.pin_bl = LCD_PIN_BL;
            cfg.invert = true;   // ACTIVE LOW - set true to invert
            cfg.freq = 44100;
            cfg.pwm_channel = 0;
            _light_instance.config(cfg);
            _panel_instance.setLight(&_light_instance);
        }
        
        setPanel(&_panel_instance);
    }
};
#endif

/**
 * @brief Initialize the LCD display
 * @return true if successful, false otherwise
 */
bool display_init();

/**
 * @brief Get the GFX display object
 * @return Pointer to LGFX object (nullptr in native build)
 */
#ifndef NATIVE_BUILD
LGFX* display_get_gfx();
#endif

/**
 * @brief Clear the entire screen with specified color
 * @param color Fill color (RGB565)
 */
void display_clear(uint16_t color);

/**
 * @brief Turn backlight on
 */
void display_backlight_on();

/**
 * @brief Turn backlight off
 */
void display_backlight_off();

/**
 * @brief Set display brightness
 * @param level Brightness level (0-255, 0=off, 255=max)
 */
void display_set_brightness(uint8_t level);

/**
 * @brief Fill a rectangle
 * @param x X position
 * @param y Y position
 * @param w Width
 * @param h Height
 * @param color RGB565 color
 */
void display_fill_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);

/**
 * @brief Draw a rectangle outline
 * @param x X position
 * @param y Y position
 * @param w Width
 * @param h Height
 * @param color RGB565 color
 */
void display_draw_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);

/**
 * @brief Draw a horizontal line
 * @param x X position
 * @param y Y position
 * @param w Width
 * @param color RGB565 color
 */
void display_draw_hline(int16_t x, int16_t y, int16_t w, uint16_t color);

/**
 * @brief Set text cursor position
 * @param x X position
 * @param y Y position
 */
void display_set_cursor(int16_t x, int16_t y);

/**
 * @brief Set text color
 * @param color RGB565 color
 */
void display_set_text_color(uint16_t color);

/**
 * @brief Set text size
 * @param size Text size multiplier (1-4)
 */
void display_set_text_size(uint8_t size);

/**
 * @brief Print text at current cursor
 * @param text Text to print
 */
void display_print(const char* text);

/**
 * @brief Print integer at current cursor
 * @param value Integer value
 */
void display_print_int(int value);

/**
 * @brief Print float at current cursor
 * @param value Float value
 * @param decimals Number of decimal places
 */
void display_print_float(float value, int decimals);

/**
 * @brief Get text width for a string at current size
 * @param text Text string
 * @param size Text size
 * @return Width in pixels
 */
int16_t display_get_text_width(const char* text, uint8_t size);

#endif // DISPLAY_H
