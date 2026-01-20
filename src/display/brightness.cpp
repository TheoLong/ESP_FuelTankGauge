#include "brightness.h"
#include "display.h"

#ifndef NATIVE_BUILD
#include <Arduino.h>
#endif

// Current brightness level
static uint8_t current_brightness = BRIGHTNESS_OUTPUT_MAX;
static unsigned long last_update_time = 0;

void brightness_init() {
#ifndef NATIVE_BUILD
    // Always configure ADC pin for brightness sensing (for debug display)
    pinMode(PIN_BRIGHTNESS_ADC, INPUT);
    analogReadResolution(ADC_RESOLUTION);
    
    #if BRIGHTNESS_AUTO_ENABLE
        Serial.print("[BRIGHTNESS] Auto-dimming enabled on GPIO");
        Serial.println(PIN_BRIGHTNESS_ADC);
        Serial.print("[BRIGHTNESS] Voltage range: ");
        Serial.print(BRIGHTNESS_VOLTAGE_MIN, 1);
        Serial.print("V - ");
        Serial.print(BRIGHTNESS_VOLTAGE_MAX, 1);
        Serial.println("V");
    #else
        Serial.print("[BRIGHTNESS] Auto-dimming disabled (GPIO");
        Serial.print(PIN_BRIGHTNESS_ADC);
        Serial.println(" still readable for debug)");
    #endif
    
    // Set initial brightness
    brightness_set(BRIGHTNESS_OUTPUT_MAX);
#endif
}

void brightness_update() {
#ifndef NATIVE_BUILD
    #if BRIGHTNESS_AUTO_ENABLE
        unsigned long now = millis();
        
        // Rate limit updates
        if (now - last_update_time < BRIGHTNESS_UPDATE_MS) {
            return;
        }
        last_update_time = now;
        
        // Read input voltage
        float voltage = brightness_read_voltage();
        
        // Map voltage to brightness
        uint8_t new_brightness;
        
        if (voltage <= BRIGHTNESS_VOLTAGE_MIN) {
            new_brightness = BRIGHTNESS_OUTPUT_MIN;
        } else if (voltage >= BRIGHTNESS_VOLTAGE_MAX) {
            new_brightness = BRIGHTNESS_OUTPUT_MAX;
        } else {
            // Linear interpolation
            float ratio = (voltage - BRIGHTNESS_VOLTAGE_MIN) / 
                         (BRIGHTNESS_VOLTAGE_MAX - BRIGHTNESS_VOLTAGE_MIN);
            new_brightness = BRIGHTNESS_OUTPUT_MIN + 
                            (uint8_t)(ratio * (BRIGHTNESS_OUTPUT_MAX - BRIGHTNESS_OUTPUT_MIN));
        }
        
        // Only update if changed significantly (avoid flicker)
        if (abs((int)new_brightness - (int)current_brightness) >= 5) {
            brightness_set(new_brightness);
            
            // Debug output
            Serial.print("[BRIGHTNESS] V=");
            Serial.print(voltage, 2);
            Serial.print(" -> ");
            Serial.println(new_brightness);
        }
    #endif
#endif
}

void brightness_set(uint8_t level) {
    current_brightness = level;
    
#ifndef NATIVE_BUILD
    // Use the display's brightness control
    display_set_brightness(level);
#endif
}

uint8_t brightness_get() {
    return current_brightness;
}

uint16_t brightness_read_raw() {
#ifndef NATIVE_BUILD
    // Always read ADC (for debug display), regardless of auto-enable
    uint32_t sum = 0;
    for (int i = 0; i < BRIGHTNESS_SAMPLES; i++) {
        sum += analogRead(PIN_BRIGHTNESS_ADC);
        delayMicroseconds(100);
    }
    return (uint16_t)(sum / BRIGHTNESS_SAMPLES);
#else
    return 0;
#endif
}

float brightness_read_voltage() {
#ifndef NATIVE_BUILD
    // Always read voltage (for debug display), regardless of auto-enable
    uint16_t raw = brightness_read_raw();
    
    // Convert ADC to voltage at the ADC pin
    float adc_voltage = ((float)raw / (float)ADC_MAX_VALUE) * ADC_VREF;
    
    // Calculate input voltage using voltage divider formula
    // Vin = Vout * (R1 + R2) / R2
    float input_voltage = adc_voltage * (BRIGHTNESS_DIVIDER_R1 + BRIGHTNESS_DIVIDER_R2) 
                         / BRIGHTNESS_DIVIDER_R2;
    
    return input_voltage;
#else
    return 0.0f;
#endif
}

bool brightness_is_auto_enabled() {
#if BRIGHTNESS_AUTO_ENABLE
    return true;
#else
    return false;
#endif
}
