idf_component_register(
  SRCS "user_app.c"
  PRIV_REQUIRES adc_bsp ui_bsp lcd_bl_pwm_bsp button_bsp qmi8658c ble_scan_bsp esp_wifi_bsp main
  INCLUDE_DIRS "./")
