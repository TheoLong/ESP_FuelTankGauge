#ifndef LCD_BL_PWM_BSP_H
#define LCD_BL_PWM_BSP_H

#define PIN_NUM_BL  15
#define example_key 9
#define BLINK_GPIO  3

#define example_test_out1 2
#define example_test_in1  21

#define  LCD_GPIO_MODE 0
#define  LCD_GPIO_MODE_OFF 1
#define  LCD_GPIO_MODE_ON  0

#define  LCD_PWM_MODE 1
#define  LCD_PWM_MODE_0   (0xff-0)
#define  LCD_PWM_MODE_25  (0xff-25)
#define  LCD_PWM_MODE_50  (0xff-50)
#define  LCD_PWM_MODE_75  (0xff-75)
#define  LCD_PWM_MODE_100 (0xff-100)
#define  LCD_PWM_MODE_125 (0xff-125)
#define  LCD_PWM_MODE_150 (0xff-150)
#define  LCD_PWM_MODE_175 (0xff-175)
#define  LCD_PWM_MODE_200 (0xff-200)
#define  LCD_PWM_MODE_225 (0xff-225)
#define  LCD_PWM_MODE_255 (0xff-255)
void gpio_init(void);
void setUpdutySubdivide(uint8_t mode , uint16_t duty); //0: GPIO模式
void ws2812_Init(void);
void ws2812_set_pixel(uint8_t value);


void GPIO_SET(uint8_t pin,uint8_t mode);
uint8_t GPIO_GET(uint8_t pin);

#endif