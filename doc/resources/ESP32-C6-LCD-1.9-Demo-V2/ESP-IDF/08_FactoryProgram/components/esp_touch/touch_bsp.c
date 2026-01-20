#include <stdio.h>
#include "touch_bsp.h"
#include "i2c_bsp.h"
#define I2C_ADDR_CST78X 0x15
uint8_t touch_en = 0;
void touch_Init(void)
{
  uint8_t data = 0x00;
  for(uint8_t i = 0; i<10; i++)
  {
    uint8_t err = I2C_writr_buff(I2C_ADDR_CST78X,0x00,&data,1); //切换正常模式
    if(err == ESP_OK)
    {
      touch_en = 1;
      break;
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}
uint8_t getTouch(uint16_t *x,uint16_t *y)
{
  uint8_t _num;
  uint8_t tp_temp[7];
  //ESP_ERROR_CHECK_WITHOUT_ABORT(I2C_read_buff(I2C_ADDR_CST78X,0x00,tp_temp,7));
  I2C_read_buff(I2C_ADDR_CST78X,0x00,tp_temp,7);
  _num = tp_temp[2];
  if(_num)
  {
    *x = ((uint16_t)(tp_temp[3] & 0x0f)<<8) + (uint16_t)tp_temp[4];
    *y = ((uint16_t)(tp_temp[5] & 0x0f)<<8) + (uint16_t)tp_temp[6];
    return 1;
  }
  return 0;
}