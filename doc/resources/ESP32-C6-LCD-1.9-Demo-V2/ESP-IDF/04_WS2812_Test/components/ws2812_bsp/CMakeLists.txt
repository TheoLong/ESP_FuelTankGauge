idf_component_register(
  SRCS "ws2812_bsp.c"
  PRIV_REQUIRES driver espressif__led_strip                 
  INCLUDE_DIRS "./")
