#ifndef FUEL_SENSOR_H
#define FUEL_SENSOR_H

#include "config.h"
#include <stdint.h>

/**
 * @brief Fuel sensor reading structure
 */
typedef struct {
    uint16_t raw_adc;       // Raw ADC value (0-4095)
    float voltage;          // Calculated voltage at ADC pin
    float resistance;       // Calculated sender resistance
    float percent;          // Calculated fuel percentage (0-100)
    bool valid;             // True if reading is within expected range
} FuelReading;

/**
 * @brief Initialize the fuel sensor ADC pins
 */
void fuel_sensor_init();

/**
 * @brief Read raw ADC value from specified tank sensor
 * @param tank_number Tank identifier (1 or 2)
 * @return Raw ADC value (0-4095)
 */
uint16_t fuel_sensor_read_raw(int tank_number);

/**
 * @brief Convert raw ADC value to voltage
 * @param raw_adc Raw ADC value (0-4095)
 * @return Voltage at ADC pin
 */
float fuel_sensor_adc_to_voltage(uint16_t raw_adc);

/**
 * @brief Convert voltage to sender resistance using voltage divider formula
 * @param voltage Measured voltage at ADC pin
 * @return Sender resistance in ohms
 */
float fuel_sensor_voltage_to_resistance(float voltage);

/**
 * @brief Convert sender resistance to fuel percentage
 * @param resistance Sender resistance in ohms
 * @return Fuel percentage (0-100, clamped)
 */
float fuel_sensor_resistance_to_percent(float resistance);

/**
 * @brief Apply smoothing/averaging to readings
 * @param tank_number Tank identifier (1 or 2)
 * @param num_samples Number of samples to average
 * @return Averaged FuelReading
 */
FuelReading fuel_sensor_read_averaged(int tank_number, int num_samples);

/**
 * @brief Read with EMA damping applied (configurable via FUEL_DAMPING_ENABLE)
 * @param tank_number Tank identifier (1 or 2)
 * @param num_samples Number of samples to average before damping
 * @return Damped FuelReading with smoothed percentage
 */
FuelReading fuel_sensor_read_damped(int tank_number, int num_samples);

/**
 * @brief Check if a resistance value is within valid sender range
 * @param resistance Resistance in ohms
 * @return true if within SENDER_RESISTANCE_EMPTY to SENDER_RESISTANCE_FULL range
 */
bool fuel_sensor_is_valid_resistance(float resistance);

// ============================================================================
// Pure calculation functions (for unit testing without hardware)
// ============================================================================

/**
 * @brief Pure calculation: ADC to voltage
 */
float calc_adc_to_voltage(uint16_t raw_adc, float v_ref, int adc_max);

/**
 * @brief Pure calculation: Voltage to resistance (voltage divider)
 * Assumes sender is between ADC pin and ground, reference resistor to Vref
 */
float calc_voltage_to_resistance(float v_adc, float v_ref, float r_ref);

/**
 * @brief Pure calculation: Resistance to percentage
 * Uses linear interpolation between empty and full resistance values
 */
float calc_resistance_to_percent(float resistance, float r_empty, float r_full);

#endif // FUEL_SENSOR_H
