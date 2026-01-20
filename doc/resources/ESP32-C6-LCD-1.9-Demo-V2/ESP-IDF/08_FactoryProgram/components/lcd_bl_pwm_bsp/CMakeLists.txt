idf_component_register(
  SRCS "lcd_bl_pwm_bsp.c"
  PRIV_REQUIRES driver espressif__led_strip                 
  INCLUDE_DIRS "./")
