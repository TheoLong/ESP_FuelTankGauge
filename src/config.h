#ifndef CONFIG_H
#define CONFIG_H

/*******************************************************************************
 * ESP_FuelTankGauge Configuration
 * 
 * All user-configurable parameters are defined here.
 * Modify these values to customize the fuel gauge behavior.
 * 
 * See doc/CONFIG_REFERENCE.md for detailed documentation.
 ******************************************************************************/

//==============================================================================
// OPERATING MODE
//==============================================================================
// Select ONE operating mode by setting it to 1, others to 0
// - MODE_NORMAL: Real ADC sensors, standard gauge display
// - MODE_DEMO:   Simulated cycling without hardware
// - MODE_DEBUG:  Real sensors + diagnostic overlay

// Default startup mode (can be changed at runtime via BOOT button)
// 0 = Normal, 1 = Demo, 2 = Debug
#define DEFAULT_MODE          0       // Start in Normal mode

// Legacy compile-time mode defines (for backward compatibility)
#define MODE_NORMAL           0       // Normal operation with real ADC sensors
#define MODE_DEMO             1       // Demo mode: cycles gauge without ADC
#define MODE_DEBUG            2       // Debug mode: shows gauge + ADC diagnostics

// Demo mode settings (only used when MODE_DEMO = 1)
#define DEMO_CYCLE_SPEED_MS   150     // Milliseconds per 1% change (slower for observation)
#define DEMO_TANK2_OFFSET     50      // Tank 2 offset from Tank 1 (0-100)

// Debug mode settings (only used when MODE_DEBUG = 1)
#define DEBUG_UPDATE_RATE_MS  200     // How often to update debug info
#define DEBUG_SHOW_RAW_ADC    1       // Show raw ADC value (0-4095)
#define DEBUG_SHOW_VOLTAGE    1       // Show calculated voltage
#define DEBUG_SHOW_RESISTANCE 1       // Show calculated resistance
#define DEBUG_SHOW_PIN        1       // Show GPIO pin number

//==============================================================================
// HARDWARE PINS - LCD (Fixed by Waveshare hardware, do not change)
//==============================================================================
#define PIN_LCD_MOSI          4       // SPI MOSI
#define PIN_LCD_CLK           5       // SPI Clock
#define PIN_LCD_DC            6       // Data/Command
#define PIN_LCD_CS            7       // Chip Select
#define PIN_LCD_RST           14      // Reset
#define PIN_LCD_BL            15      // Backlight (PWM capable)

//==============================================================================
// HARDWARE PINS - BOOT BUTTON
//==============================================================================
#define PIN_BOOT_BUTTON       9       // BOOT button (GPIO9) - used for mode switching
#define BUTTON_DEBOUNCE_MS    50      // Debounce time in milliseconds

//==============================================================================
// HARDWARE PINS - FUEL SENSORS (ADC)
//==============================================================================
#define PIN_TANK1_ADC         0       // GPIO0 = ADC1_CH0
#define PIN_TANK2_ADC         1       // GPIO1 = ADC1_CH1

//==============================================================================
// VOLTAGE DIVIDER CIRCUIT
//==============================================================================
// Circuit: Vref -> R_reference -> ADC_PIN -> R_sender -> GND
//
//          Vref (DIVIDER_VREF)
//           |
//          [R_ref] (DIVIDER_R_REFERENCE)
//           |
//          ADC -----> To GPIO pin
//           |
//          [R_sender] (fuel tank sender, 33-240 ohms)
//           |
//          GND

#define DIVIDER_VREF          3.3f    // Voltage applied to top of divider (V)
#define DIVIDER_R_REFERENCE   100.0f  // Reference resistor value (ohms)

//==============================================================================
// TANK CAPACITY
//==============================================================================
// Configure the tank capacity for gallon display

#define TANK_CAPACITY_GALLONS 50      // Tank capacity in gallons (0-50 range)

//==============================================================================
// FUEL SENDER SPECIFICATIONS
//==============================================================================
// Standard automotive fuel sender: 33 ohms (full) to 240 ohms (empty)
// Some senders are reversed - swap these values if your gauge reads backwards

#define SENDER_R_FULL         33.0f   // Resistance when tank is FULL (ohms)
#define SENDER_R_EMPTY        240.0f  // Resistance when tank is EMPTY (ohms)

//==============================================================================
// ADC CONFIGURATION
//==============================================================================
#define ADC_RESOLUTION        12      // ADC resolution in bits (ESP32-C6 = 12-bit)
#define ADC_MAX_VALUE         4095    // Maximum ADC value (2^12 - 1)
#define ADC_VREF              3.3f    // ADC reference voltage
#define ADC_SAMPLES           10      // Number of samples to average per reading

//==============================================================================
// SIGNAL FILTERING / SMOOTHING
//==============================================================================
// Exponential Moving Average (EMA) smoothing
// Lower alpha = smoother but slower response
// Higher alpha = faster response but more noise
// Range: 0.0 to 1.0

#define SMOOTHING_ALPHA       0.15f   // EMA smoothing factor
#define MIN_CHANGE_PERCENT    1       // Minimum % change to trigger display update

//==============================================================================
// DISPLAY CONFIGURATION
//==============================================================================
#define SCREEN_WIDTH          170     // LCD width in pixels
#define SCREEN_HEIGHT         320     // LCD height in pixels
#define SCREEN_ROTATION       0       // 0=Portrait, 1=Landscape, 2=Portrait180, 3=Landscape180

// Backlight
#define BACKLIGHT_DEFAULT     255     // Default brightness (0-255, not used - on/off only)
#define BACKLIGHT_ACTIVE_LOW  1       // 1 = LOW turns on backlight, 0 = HIGH turns on

//==============================================================================
// GAUGE LAYOUT - POSITIONING
//==============================================================================
// Screen is divided into two halves for Tank 1 (left) and Tank 2 (right)

#define LAYOUT_PCT_TEXT_Y     20      // Y position of percentage text
#define LAYOUT_PCT_TEXT_H     30      // Height of percentage text area
#define LAYOUT_BAR_TOP        55      // Y position where bar starts
#define LAYOUT_BAR_BOTTOM     290     // Y position where bar ends
#define LAYOUT_LABEL_Y        298     // Y position of tank labels (T1/T2)

//==============================================================================
// GAUGE APPEARANCE
//==============================================================================
#define GAUGE_BAR_WIDTH       60      // Width of each bar in pixels
#define GAUGE_BORDER_WIDTH    2       // Border thickness around bars
#define GAUGE_SEGMENTS        20      // Number of visual segment lines
#define GAUGE_SEGMENT_LINE_W  1       // Segment divider line width (pixels)

//==============================================================================
// COLOR THRESHOLDS (Percentage)
//==============================================================================
// Define at what fuel level colors change
// 0% to THRESHOLD_RED_MAX = RED (danger)
// THRESHOLD_RED_MAX to THRESHOLD_YELLOW_MAX = YELLOW (warning)  
// Above THRESHOLD_YELLOW_MAX = GREEN (good)

#define THRESHOLD_RED_MAX     20      // 0-20% = Red zone
#define THRESHOLD_YELLOW_MAX  40      // 20-40% = Yellow zone
                                      // 40-100% = Green zone

//==============================================================================
// COLORS (RGB565 Format)
//==============================================================================
// RGB565: 5 bits red, 6 bits green, 5 bits blue

// Background and borders
#define COLOR_BACKGROUND      0x0000  // Black
#define COLOR_BORDER          0xFFFF  // White
#define COLOR_SEGMENT_LINE    0x39E7  // Medium gray

// Fuel level zone colors
#define COLOR_ZONE_RED        0xF800  // Pure red (RGB: 255, 0, 0)
#define COLOR_ZONE_YELLOW     0xFFE0  // Pure yellow (RGB: 255, 255, 0)
#define COLOR_ZONE_GREEN      0x07E0  // Pure green (RGB: 0, 255, 0)

// Debug mode colors
#define COLOR_DEBUG_BG        0x0000  // Black background
#define COLOR_DEBUG_BORDER    0x07FF  // Cyan border
#define COLOR_DEBUG_LABEL     0x07FF  // Cyan labels
#define COLOR_DEBUG_VALUE     0xFFE0  // Yellow values

//==============================================================================
// TEXT SIZES
//==============================================================================
// Arduino_GFX text sizes: 1=6x8, 2=12x16, 3=18x24, 4=24x32

#define TEXT_SIZE_PERCENT     3       // Size for percentage display (e.g., "75%")
#define TEXT_SIZE_LABEL       2       // Size for tank labels ("T1", "T2")
#define TEXT_SIZE_DEBUG       1       // Size for debug information

//==============================================================================
// TIMING / REFRESH RATES
//==============================================================================
#define MAIN_LOOP_INTERVAL_MS 50      // Main loop update interval
#define DISPLAY_REFRESH_MS    50      // Display update interval
#define SENSOR_READ_MS        50      // Sensor reading interval

//==============================================================================
// TANK LABELS
//==============================================================================
#define TANK1_LABEL           "T1"    // Label for Tank 1
#define TANK2_LABEL           "T2"    // Label for Tank 2

//==============================================================================
// DEBUG MODE LAYOUT
//==============================================================================
// In debug mode, the screen shows gauges and diagnostic info overlay

#define DEBUG_INFO_Y          140     // Y position for debug overlay (inside bar)
#define DEBUG_LINE_HEIGHT     10      // Pixels between debug text lines
#define DEBUG_BOX_PADDING     2       // Padding inside debug box

//==============================================================================
// CALCULATED VALUES (Do not modify)
//==============================================================================
#define GAUGE_BAR_HEIGHT      (LAYOUT_BAR_BOTTOM - LAYOUT_BAR_TOP)
#define ZONE_WIDTH            (SCREEN_WIDTH / 2)
#define BAR1_X                ((ZONE_WIDTH - GAUGE_BAR_WIDTH) / 2)
#define BAR2_X                (ZONE_WIDTH + (ZONE_WIDTH - GAUGE_BAR_WIDTH) / 2)
#define SEGMENT_HEIGHT_F      ((float)GAUGE_BAR_HEIGHT / (float)GAUGE_SEGMENTS)

// Note: Mode is now runtime-switchable via BOOT button
// MODE_NORMAL, MODE_DEMO, MODE_DEBUG are used as enum values (0, 1, 2)
// DEFAULT_MODE sets the startup mode

//==============================================================================
// COMPATIBILITY ALIASES (Used by internal modules)
//==============================================================================
// LCD Pins
#define LCD_PIN_MOSI          PIN_LCD_MOSI
#define LCD_PIN_CLK           PIN_LCD_CLK
#define LCD_PIN_DC            PIN_LCD_DC
#define LCD_PIN_CS            PIN_LCD_CS
#define LCD_PIN_RST           PIN_LCD_RST
#define LCD_PIN_BL            PIN_LCD_BL

// ADC Pins
#define ADC_PIN_TANK1         PIN_TANK1_ADC
#define ADC_PIN_TANK2         PIN_TANK2_ADC

// Display dimensions
#define LCD_WIDTH             SCREEN_WIDTH
#define LCD_HEIGHT            SCREEN_HEIGHT
#define LCD_ROTATION          SCREEN_ROTATION
#define LCD_COL_OFFSET        35
#define LCD_ROW_OFFSET        0

// Voltage divider
#define VOLTAGE_DIVIDER_R_REF DIVIDER_R_REFERENCE

// Sender resistance
#define SENDER_RESISTANCE_FULL  SENDER_R_FULL
#define SENDER_RESISTANCE_EMPTY SENDER_R_EMPTY

// Fuel thresholds
#define FUEL_THRESHOLD_RED    (float)THRESHOLD_RED_MAX
#define FUEL_THRESHOLD_YELLOW (float)THRESHOLD_YELLOW_MAX

// UI Colors (RGB565)
#define UI_COLOR_BACKGROUND   COLOR_BACKGROUND
#define UI_COLOR_BORDER       COLOR_BORDER
#define UI_COLOR_TEXT         0xFFFF
#define UI_COLOR_RED          COLOR_ZONE_RED
#define UI_COLOR_YELLOW       COLOR_ZONE_YELLOW
#define UI_COLOR_GREEN        COLOR_ZONE_GREEN
#define UI_COLOR_EMPTY        0x2104
#define UI_COLOR_DEBUG        COLOR_DEBUG_LABEL

// Gauge dimensions
#define GAUGE_WIDTH           GAUGE_BAR_WIDTH
#define GAUGE_SEGMENT_COUNT   GAUGE_SEGMENTS
#define GAUGE_SEGMENT_HEIGHT  13
#define GAUGE_SEGMENT_GAP     1

// Text sizes
#define UI_FONT_SIZE_LABEL    TEXT_SIZE_LABEL
#define UI_FONT_SIZE_PERCENT  TEXT_SIZE_PERCENT

// Layout offsets
#define UI_LABEL_OFFSET       25
#define UI_PERCENT_OFFSET     10

// Timing
#define UPDATE_INTERVAL_MS    MAIN_LOOP_INTERVAL_MS
#define ADC_SAMPLE_DELAY_US   100

// Demo mode settings
#define DEMO_CYCLE_INTERVAL_MS DEMO_CYCLE_SPEED_MS
#define DEMO_CYCLE_STEP       1.0f
#define DEMO_START_TANK1      50.0f
#define DEMO_START_TANK2      (50.0f + DEMO_TANK2_OFFSET)

#endif // CONFIG_H
