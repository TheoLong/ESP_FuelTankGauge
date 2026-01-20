#include <unity.h>
#include "../src/sensor/fuel_sensor.h"
#include "../src/display/gauge.h"

// ============================================================================
// Test: ADC to Voltage Conversion
// ============================================================================

void test_adc_to_voltage_zero() {
    float voltage = calc_adc_to_voltage(0, 3.3f, 4095);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, voltage);
}

void test_adc_to_voltage_max() {
    float voltage = calc_adc_to_voltage(4095, 3.3f, 4095);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 3.3f, voltage);
}

void test_adc_to_voltage_midpoint() {
    float voltage = calc_adc_to_voltage(2048, 3.3f, 4095);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 1.65f, voltage);
}

// ============================================================================
// Test: Voltage to Resistance (Voltage Divider)
// ============================================================================

void test_voltage_to_resistance_full_tank() {
    // At 33 ohms (full tank), with 100 ohm reference:
    // V_adc = 3.3 * 33 / (100 + 33) = 0.819V
    float resistance = calc_voltage_to_resistance(0.819f, 3.3f, 100.0f);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 33.0f, resistance);
}

void test_voltage_to_resistance_empty_tank() {
    // At 240 ohms (empty tank), with 100 ohm reference:
    // V_adc = 3.3 * 240 / (100 + 240) = 2.329V
    float resistance = calc_voltage_to_resistance(2.329f, 3.3f, 100.0f);
    TEST_ASSERT_FLOAT_WITHIN(2.0f, 240.0f, resistance);
}

void test_voltage_to_resistance_midpoint() {
    // At ~136.5 ohms (50% tank), with 100 ohm reference:
    // V_adc = 3.3 * 136.5 / (100 + 136.5) = 1.904V
    float resistance = calc_voltage_to_resistance(1.904f, 3.3f, 100.0f);
    TEST_ASSERT_FLOAT_WITHIN(2.0f, 136.5f, resistance);
}

void test_voltage_to_resistance_invalid_zero() {
    // Zero voltage should return invalid
    float resistance = calc_voltage_to_resistance(0.0f, 3.3f, 100.0f);
    TEST_ASSERT_LESS_THAN(0.0f, resistance);
}

void test_voltage_to_resistance_invalid_at_vref() {
    // Voltage at Vref means infinite resistance (open circuit)
    float resistance = calc_voltage_to_resistance(3.3f, 3.3f, 100.0f);
    TEST_ASSERT_LESS_THAN(0.0f, resistance);
}

// ============================================================================
// Test: Resistance to Percentage
// ============================================================================

void test_resistance_to_percent_full() {
    // 33 ohms = 100% (full tank)
    float percent = calc_resistance_to_percent(33.0f, 240.0f, 33.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 100.0f, percent);
}

void test_resistance_to_percent_empty() {
    // 240 ohms = 0% (empty tank)
    float percent = calc_resistance_to_percent(240.0f, 240.0f, 33.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 0.0f, percent);
}

void test_resistance_to_percent_half() {
    // 136.5 ohms = 50%
    float midpoint = (240.0f + 33.0f) / 2.0f;  // 136.5 ohms
    float percent = calc_resistance_to_percent(midpoint, 240.0f, 33.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.5f, 50.0f, percent);
}

void test_resistance_to_percent_quarter() {
    // 25% tank = 75% of resistance range from full
    // R = 33 + 0.75 * (240 - 33) = 188.25 ohms
    float percent = calc_resistance_to_percent(188.25f, 240.0f, 33.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.5f, 25.0f, percent);
}

void test_resistance_to_percent_clamp_below() {
    // Below full resistance should clamp to 100%
    float percent = calc_resistance_to_percent(20.0f, 240.0f, 33.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 100.0f, percent);
}

void test_resistance_to_percent_clamp_above() {
    // Above empty resistance should clamp to 0%
    float percent = calc_resistance_to_percent(300.0f, 240.0f, 33.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 0.0f, percent);
}

// ============================================================================
// Test: Color Selection Based on Percentage
// ============================================================================

void test_color_red_zone() {
    // 0-20% should be red
    uint16_t color = gauge_get_color_for_percent(10.0f);
    TEST_ASSERT_EQUAL_UINT16(UI_COLOR_RED, color);
    
    color = gauge_get_color_for_percent(0.0f);
    TEST_ASSERT_EQUAL_UINT16(UI_COLOR_RED, color);
    
    color = gauge_get_color_for_percent(20.0f);
    TEST_ASSERT_EQUAL_UINT16(UI_COLOR_RED, color);
}

void test_color_yellow_zone() {
    // 21-40% should be yellow
    uint16_t color = gauge_get_color_for_percent(21.0f);
    TEST_ASSERT_EQUAL_UINT16(UI_COLOR_YELLOW, color);
    
    color = gauge_get_color_for_percent(30.0f);
    TEST_ASSERT_EQUAL_UINT16(UI_COLOR_YELLOW, color);
    
    color = gauge_get_color_for_percent(40.0f);
    TEST_ASSERT_EQUAL_UINT16(UI_COLOR_YELLOW, color);
}

void test_color_green_zone() {
    // 41-100% should be green
    uint16_t color = gauge_get_color_for_percent(41.0f);
    TEST_ASSERT_EQUAL_UINT16(UI_COLOR_GREEN, color);
    
    color = gauge_get_color_for_percent(50.0f);
    TEST_ASSERT_EQUAL_UINT16(UI_COLOR_GREEN, color);
    
    color = gauge_get_color_for_percent(100.0f);
    TEST_ASSERT_EQUAL_UINT16(UI_COLOR_GREEN, color);
}

// ============================================================================
// Test: Segment Calculation
// ============================================================================

void test_segments_empty() {
    int segments = gauge_get_filled_segments(0.0f);
    TEST_ASSERT_EQUAL_INT(0, segments);
}

void test_segments_full() {
    int segments = gauge_get_filled_segments(100.0f);
    TEST_ASSERT_EQUAL_INT(GAUGE_SEGMENT_COUNT, segments);
}

void test_segments_half() {
    int segments = gauge_get_filled_segments(50.0f);
    TEST_ASSERT_EQUAL_INT(GAUGE_SEGMENT_COUNT / 2, segments);
}

void test_segments_one_segment() {
    // One segment = 100/20 = 5%
    int segments = gauge_get_filled_segments(5.0f);
    TEST_ASSERT_EQUAL_INT(1, segments);
}

void test_segments_clamp_negative() {
    int segments = gauge_get_filled_segments(-10.0f);
    TEST_ASSERT_EQUAL_INT(0, segments);
}

void test_segments_clamp_over_100() {
    int segments = gauge_get_filled_segments(150.0f);
    TEST_ASSERT_EQUAL_INT(GAUGE_SEGMENT_COUNT, segments);
}

// ============================================================================
// Test: End-to-End ADC to Percentage
// ============================================================================

void test_full_chain_full_tank() {
    // Simulate full tank (33 ohms)
    // V_adc = 3.3 * 33 / (100 + 33) = 0.819V
    // ADC = 0.819 / 3.3 * 4095 = 1016
    uint16_t adc = 1016;
    
    float voltage = calc_adc_to_voltage(adc, 3.3f, 4095);
    float resistance = calc_voltage_to_resistance(voltage, 3.3f, 100.0f);
    float percent = calc_resistance_to_percent(resistance, 240.0f, 33.0f);
    
    TEST_ASSERT_FLOAT_WITHIN(5.0f, 100.0f, percent);
}

void test_full_chain_empty_tank() {
    // Simulate empty tank (240 ohms)
    // V_adc = 3.3 * 240 / (100 + 240) = 2.329V
    // ADC = 2.329 / 3.3 * 4095 = 2890
    uint16_t adc = 2890;
    
    float voltage = calc_adc_to_voltage(adc, 3.3f, 4095);
    float resistance = calc_voltage_to_resistance(voltage, 3.3f, 100.0f);
    float percent = calc_resistance_to_percent(resistance, 240.0f, 33.0f);
    
    TEST_ASSERT_FLOAT_WITHIN(5.0f, 0.0f, percent);
}

void test_full_chain_half_tank() {
    // Simulate half tank (136.5 ohms)
    // V_adc = 3.3 * 136.5 / (100 + 136.5) = 1.904V
    // ADC = 1.904 / 3.3 * 4095 = 2363
    uint16_t adc = 2363;
    
    float voltage = calc_adc_to_voltage(adc, 3.3f, 4095);
    float resistance = calc_voltage_to_resistance(voltage, 3.3f, 100.0f);
    float percent = calc_resistance_to_percent(resistance, 240.0f, 33.0f);
    
    TEST_ASSERT_FLOAT_WITHIN(5.0f, 50.0f, percent);
}

// ============================================================================
// Test Runner
// ============================================================================

void setUp(void) {
    // Called before each test
}

void tearDown(void) {
    // Called after each test
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    
    // ADC to Voltage tests
    RUN_TEST(test_adc_to_voltage_zero);
    RUN_TEST(test_adc_to_voltage_max);
    RUN_TEST(test_adc_to_voltage_midpoint);
    
    // Voltage to Resistance tests
    RUN_TEST(test_voltage_to_resistance_full_tank);
    RUN_TEST(test_voltage_to_resistance_empty_tank);
    RUN_TEST(test_voltage_to_resistance_midpoint);
    RUN_TEST(test_voltage_to_resistance_invalid_zero);
    RUN_TEST(test_voltage_to_resistance_invalid_at_vref);
    
    // Resistance to Percentage tests
    RUN_TEST(test_resistance_to_percent_full);
    RUN_TEST(test_resistance_to_percent_empty);
    RUN_TEST(test_resistance_to_percent_half);
    RUN_TEST(test_resistance_to_percent_quarter);
    RUN_TEST(test_resistance_to_percent_clamp_below);
    RUN_TEST(test_resistance_to_percent_clamp_above);
    
    // Color selection tests
    RUN_TEST(test_color_red_zone);
    RUN_TEST(test_color_yellow_zone);
    RUN_TEST(test_color_green_zone);
    
    // Segment calculation tests
    RUN_TEST(test_segments_empty);
    RUN_TEST(test_segments_full);
    RUN_TEST(test_segments_half);
    RUN_TEST(test_segments_one_segment);
    RUN_TEST(test_segments_clamp_negative);
    RUN_TEST(test_segments_clamp_over_100);
    
    // End-to-end tests
    RUN_TEST(test_full_chain_full_tank);
    RUN_TEST(test_full_chain_empty_tank);
    RUN_TEST(test_full_chain_half_tank);
    
    return UNITY_END();
}
