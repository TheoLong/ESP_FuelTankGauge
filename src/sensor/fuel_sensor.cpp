#include "fuel_sensor.h"

#ifndef NATIVE_BUILD
#include <Arduino.h>
#endif

// ============================================================================
// EMA Damping State (per tank)
// ============================================================================

static float ema_tank1_percent = -1.0f;  // -1 indicates uninitialized
static float ema_tank2_percent = -1.0f;
static bool ema_initialized[2] = {false, false};

// Apply EMA damping to a reading
static float apply_ema(float new_value, float* ema_state, bool* initialized) {
#if FUEL_DAMPING_ENABLE
    if (!(*initialized)) {
        // First reading - initialize EMA to current value
        *ema_state = new_value;
        *initialized = true;
        return new_value;
    }
    
    // EMA formula: smoothed = alpha * new + (1 - alpha) * previous
    *ema_state = FUEL_DAMPING_ALPHA * new_value + (1.0f - FUEL_DAMPING_ALPHA) * (*ema_state);
    return *ema_state;
#else
    // Damping disabled - return raw value
    (void)ema_state;
    (void)initialized;
    return new_value;
#endif
}

// ============================================================================
// Pure calculation functions (hardware-independent, testable)
// ============================================================================

float calc_adc_to_voltage(uint16_t raw_adc, float v_ref, int adc_max) {
    return (raw_adc / (float)adc_max) * v_ref;
}

float calc_voltage_to_resistance(float v_adc, float v_ref, float r_ref) {
    // Voltage divider: V_adc = Vref * R_sender / (R_ref + R_sender)
    // Solving for R_sender: R_sender = (V_adc * R_ref) / (Vref - V_adc)
    
    // Protect against division by zero
    if (v_ref <= v_adc || v_adc < 0.001f) {
        return -1.0f; // Invalid
    }
    
    return (v_adc * r_ref) / (v_ref - v_adc);
}

float calc_resistance_to_percent(float resistance, float r_empty, float r_full) {
    // 33-240 ohm sender: 33 ohms = full (100%), 240 ohms = empty (0%)
    // Linear interpolation between these points
    
    if (resistance <= r_full) {
        return 100.0f;
    }
    if (resistance >= r_empty) {
        return 0.0f;
    }
    
    // Linear interpolation: percent = 100 * (r_empty - resistance) / (r_empty - r_full)
    float percent = 100.0f * (r_empty - resistance) / (r_empty - r_full);
    
    // Clamp to 0-100 range
    if (percent < 0.0f) percent = 0.0f;
    if (percent > 100.0f) percent = 100.0f;
    
    return percent;
}

// ============================================================================
// Hardware-dependent functions
// ============================================================================

#ifndef NATIVE_BUILD

void fuel_sensor_init() {
    // Configure ADC pins as inputs
    pinMode(ADC_PIN_TANK1, INPUT);
    pinMode(ADC_PIN_TANK2, INPUT);
    
    // Set ADC resolution to 12 bits (0-4095)
    analogReadResolution(ADC_RESOLUTION);
    
    // Optional: Set attenuation for full 0-3.3V range
    // ESP32-C6 default is 11dB attenuation (0-3.3V range)
    analogSetAttenuation(ADC_11db);
}

uint16_t fuel_sensor_read_raw(int tank_number) {
    int pin = (tank_number == 1) ? ADC_PIN_TANK1 : ADC_PIN_TANK2;
    return analogRead(pin);
}

#else

// Native build stubs
void fuel_sensor_init() {}
uint16_t fuel_sensor_read_raw(int tank_number) { 
    (void)tank_number;
    return 2048; // Mid-range for testing
}

#endif

// ============================================================================
// Wrapper functions using configured values
// ============================================================================

float fuel_sensor_adc_to_voltage(uint16_t raw_adc) {
    return calc_adc_to_voltage(raw_adc, ADC_VREF, ADC_MAX_VALUE);
}

float fuel_sensor_voltage_to_resistance(float voltage) {
    return calc_voltage_to_resistance(voltage, ADC_VREF, VOLTAGE_DIVIDER_R_REF);
}

float fuel_sensor_resistance_to_percent(float resistance) {
    return calc_resistance_to_percent(resistance, 
                                       SENDER_RESISTANCE_EMPTY, 
                                       SENDER_RESISTANCE_FULL);
}

bool fuel_sensor_is_valid_resistance(float resistance) {
    // Allow some tolerance outside nominal range
    float tolerance = 10.0f; // 10 ohms tolerance
    return (resistance >= (SENDER_RESISTANCE_FULL - tolerance) &&
            resistance <= (SENDER_RESISTANCE_EMPTY + tolerance));
}

FuelReading fuel_sensor_read_averaged(int tank_number, int num_samples) {
    if (num_samples < 1) num_samples = 1;
    if (num_samples > 100) num_samples = 100;
    
    uint32_t sum = 0;
    
    for (int i = 0; i < num_samples; i++) {
        sum += fuel_sensor_read_raw(tank_number);
        #ifndef NATIVE_BUILD
        delayMicroseconds(ADC_SAMPLE_DELAY_US);
        #endif
    }
    
    // Create reading from averaged ADC value
    FuelReading reading;
    reading.raw_adc = sum / num_samples;
    reading.voltage = fuel_sensor_adc_to_voltage(reading.raw_adc);
    reading.resistance = fuel_sensor_voltage_to_resistance(reading.voltage);
    reading.percent = fuel_sensor_resistance_to_percent(reading.resistance);
    reading.valid = fuel_sensor_is_valid_resistance(reading.resistance);
    
    return reading;
}

FuelReading fuel_sensor_read_damped(int tank_number, int num_samples) {
    // Get averaged reading first
    FuelReading reading = fuel_sensor_read_averaged(tank_number, num_samples);
    
    // Apply EMA damping to the percentage
    if (tank_number == 1) {
        reading.percent = apply_ema(reading.percent, &ema_tank1_percent, &ema_initialized[0]);
    } else {
        reading.percent = apply_ema(reading.percent, &ema_tank2_percent, &ema_initialized[1]);
    }
    
    return reading;
}
