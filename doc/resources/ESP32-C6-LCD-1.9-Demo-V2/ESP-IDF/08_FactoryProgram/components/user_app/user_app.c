#include <stdio.h>
#include "user_app.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "adc_bsp.h"
#include "esp_timer.h"
#include "gui_guider.h"
#include "lcd_bl_pwm_bsp.h"
#include "esp_timer.h"
#include "button_bsp.h"
#include "qmi8658c.h"
#include "user_config.h"
#include "ble_scan_bsp.h"
#include "esp_wifi_bsp.h"
#include "esp_log.h"

static const char *TAG = "user_app";

lv_ui guider_ui;
ClockModule clock_iniput;
TaskHandle_t pxBleTask = NULL;
TaskHandle_t pxWifiTask = NULL;
#if 1
void example_wsled_port_task(void *arg);
void example_rgb_task(void *arg);
void example_user_task(void *arg);
void example_key_task(void *arg);
void example_gpio_task(void *arg);
void example_wifiscan_task(void *arg);
void example_blescan_task(void *arg);
void clock_init(void);
void lv_clear_list(lv_obj_t *obj,uint8_t value);
static void screen_btn_event_handler (lv_event_t *e);
void example_scan_task(void *arg);
void user_Click_Event_Init(lv_ui *ui)
{
  lv_obj_add_event_cb(ui->screen_btn_5, screen_btn_event_handler, LV_EVENT_ALL, ui);    //事件
}
void user_Init(void)
{
  setup_ui(&guider_ui);
  gpio_init();
  setUpdutySubdivide(LCD_GPIO_MODE,LCD_GPIO_MODE_ON);
  user_Click_Event_Init(&guider_ui);
  xTaskCreate(example_rgb_task, "example_rgb_task", 4 * 1024, &guider_ui, 2, NULL);          // RGB 颜色检测
  xTaskCreate(example_user_task, "example_user_task", 4 * 1024, &guider_ui, 2, NULL);        // user 综合代码
  xTaskCreate(example_key_task, "example_key_task", 4 * 1024, &guider_ui, 2, NULL);          // key
  xTaskCreate(example_wsled_port_task, "example_wsled_port_task", 4 * 1024, NULL, 2, NULL); 
  xTaskCreate(example_scan_task, "example_scan_task", 3 * 1024, &guider_ui, 2, NULL);        //快速扫描WIFI BLE 事件
  clock_init();
}
//wifi :1 ble:0
void esp_wifi_ble_setscan(uint8_t mode)
{
  static uint8_t wifi_ble_flag = 0;
  if(mode != wifi_ble_flag)
  {
    if(mode) //wifi 需要释放ble
    {
      ble_scan_Deinit();
      espwifi_Init();
    }
    else
    {
      espwifi_Deinit();
      ble_scan_Init();
    }
    wifi_ble_flag = mode;
  }
}
void example_scan_task(void *arg)
{
  lv_ui *ui = (lv_ui *)arg;
  nvs_flash_Init();
  ble_scan_class_init();
  ble_scan_Init();
  ble_scan_setconf();
  uint8_t mac[6];
  uint8_t connt = 0;
  char lv_buf_set[30] = {0};
  //espwifi_Init();
  lv_label_set_text(ui->screen_label_12, ".....");
  lv_label_set_text(ui->screen_label_14, ".....");
  for(;xQueueReceive(ble_Queue,mac,3500) == pdTRUE;)
  {
    ESP_LOGI(TAG, "%d",connt);
    connt++;
    vTaskDelay(pdMS_TO_TICKS(20));
  }
  sprintf(lv_buf_set,"%d,T",connt);
  lv_label_set_text(ui->screen_label_12, lv_buf_set);
  connt = 0;
  esp_wifi_ble_setscan(1);
  ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_scan_start(NULL,true));               //扫描可用AP
  ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_scan_get_ap_num(&connt));
  vTaskDelay(pdMS_TO_TICKS(5000));
  sprintf(lv_buf_set,"%d,T",connt);
  lv_label_set_text(ui->screen_label_14, lv_buf_set);
  connt = 0;
  espwifi_Deinit();
  vTaskDelete(NULL);
}
static void screen_btn_event_handler (lv_event_t *e)
{
  lv_event_code_t code = lv_event_get_code(e);
  lv_ui *ui = (lv_ui *)e->user_data;
  lv_obj_t * module = e->current_target;
  switch (code)
  {
    case LV_EVENT_CLICKED:
    {
      if(module == ui->screen_btn_5)
      {
        char sd_read_buff[30] = {""};
        s_example_write_file("/sd_card/test.txt","I like China.");
        s_example_read_file("/sd_card/test.txt",sd_read_buff,NULL);
        int8_t err = strcmp(sd_read_buff,"I like China.");
        if(err == 0)
        {
          lv_label_set_text(ui->screen_btn_5_label, "pass-test");
        }
        else
        {
          lv_label_set_text(ui->screen_btn_5_label, "failed-test");
        }
      }
      break;
    }
    default:
      break;
  }
}
void lv_clear_list(lv_obj_t *obj,uint8_t value) 
{
	for(signed char i = value-1; i>=0; i--)
	{
		lv_obj_t *imte = lv_obj_get_child(obj,i);
		lv_obj_add_flag(imte,LV_OBJ_FLAG_HIDDEN);
		vTaskDelay(pdMS_TO_TICKS(20));
	}
  vTaskDelay(pdMS_TO_TICKS(20));
  lv_obj_invalidate(obj);       //下个周期重新绘画
}
void example_key_task(void *arg)
{
  lv_ui *ui = (lv_ui *)arg;
  uint8_t flag = 0x01;
  uint8_t stemg = 0x00;
  uint8_t lsr = 0;
  button_Init();
  for(;;)
  {
    EventBits_t even = xEventGroupWaitBits(key_groups,(0x01<<1) | (0x01<<0) | (0x01<<2),pdTRUE,pdFALSE,1000);
    if( (even >> 0) & 0x01 )
    {
      switch (stemg)
      {
        case 0x00:
        lv_obj_scroll_by(ui->screen_carousel_1,-170,0,LV_ANIM_OFF);
        stemg = 1;
          break;
        case 0x01:
        stemg = 0;
        lv_obj_scroll_by(ui->screen_carousel_1,170,0,LV_ANIM_OFF);
          break;
        default:
          break;
      }
    }
    else if( (even >> 1) & 0x01 )
    {
      if((flag>>0) & 0x01)
      {
        flag &= ~(0x01<<0);
        setUpdutySubdivide(LCD_GPIO_MODE,LCD_GPIO_MODE_OFF);
      }
      else
      {
        flag |= (0x01<<0);
        setUpdutySubdivide(LCD_GPIO_MODE,LCD_GPIO_MODE_ON);
      }
    }
    else if( (even >> 2) & 0x01 )
    {
      lv_event_send(ui->screen_btn_5, LV_EVENT_CLICKED, NULL);
      printf("ok3\n");
    }
    else
    {
      lsr++;
      //lv_label_set_text(ui->screen_label_14, "NONO");
    }
    if(lsr == 2)
    {
      lsr = 0;
      lv_label_set_text(ui->screen_btn_5_label, "SD card read and write Test");
    }
  }
}

void example_wsled_port_task(void *arg)
{
  ws2812_Init();
  vTaskDelay(pdMS_TO_TICKS(5000));

  ws2812_set_pixel(1);
  vTaskDelay(pdMS_TO_TICKS(2000));
  ws2812_set_pixel(2);
  vTaskDelay(pdMS_TO_TICKS(2000));
  ws2812_set_pixel(3);
  vTaskDelay(pdMS_TO_TICKS(2000));
  ws2812_set_pixel(4);
  vTaskDelay(pdMS_TO_TICKS(2000));
  ws2812_set_pixel(5);
  vTaskDelay(pdMS_TO_TICKS(2000));
  ws2812_set_pixel(6);
  vTaskDelay(pdMS_TO_TICKS(2000));

  vTaskDelete(NULL); //删除任务
}

void user_app_sd_read(lv_ui *obj)
{
  char sd_buf[15] = {0};
  float sd_value = sd_card_get_value();
  if(sd_value)
  {
    sprintf(sd_buf,"%.2fG",sd_value);
    lv_label_set_text(obj->screen_label_19, sd_buf);
  }
  else
  {
    lv_label_set_text(obj->screen_label_19, "NULL");
  }
}
void example_user_task(void *arg)
{
  lv_ui *ui = (lv_ui *)arg;
  uint32_t stimes = 0;
  uint32_t adc_test = 0;
  uint32_t qmi_test = 0;
  float adc_value;
  float acc[3] = {0};
  float gyro[3] = {0};
  char adc_buf[15] = {0};
  char qmi_buf[15] = {0};
  adc_bsp_init();
  qmi8658_init();
  user_app_sd_read(ui);
  for(;;)
  {
    if(stimes - adc_test > 1) //2s
    {
      adc_test = stimes;
      adc_get_value(&adc_value,NULL);
      if(adc_value)
      {
        sprintf(adc_buf,"%.2fV",adc_value);
        lv_label_set_text(ui->screen_label_2, adc_buf); 
      }
    }
    if(stimes - qmi_test > 0) //1s
    {
      qmi_test = stimes;
      qmi8658_read_xyz(acc,gyro);
      sprintf(qmi_buf,"x : %.2f",acc[0]);
      lv_label_set_text(ui->screen_label_8, qmi_buf);
      sprintf(qmi_buf,"y : %.2f",acc[1]);
      lv_label_set_text(ui->screen_label_9, qmi_buf);
      sprintf(qmi_buf,"z : %.2f",acc[2]);
      lv_label_set_text(ui->screen_label_10, qmi_buf);
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
    stimes++;
  }
}
void out_time(ClockModule * clock)
{
  clock->out_Hours = clock->Hours * 5;
  clock->out_minutes = clock->minutes;
  clock->out_seconds = clock->seconds;
  uint8_t bat = clock->out_minutes / 12;
  clock->out_Hours += bat;

  int16_t Hours_ars = clock_iniput.out_Hours * 6 - 90;
  int16_t Minutes_ars = clock_iniput.out_minutes * 6 - 90;
  int16_t Seconds_ars = clock_iniput.out_seconds * 6 - 90;
  lv_img_set_angle(guider_ui.screen_img_4, Hours_ars * 10);
  lv_img_set_angle(guider_ui.screen_img_5, Minutes_ars * 10);
  lv_img_set_angle(guider_ui.screen_img_6, Seconds_ars * 10);
}
static void clock_task_callback(void *arg)
{
  static uint8_t bat = 0;
  lv_ui *ui = (lv_ui *)arg;
  clock_iniput.out_seconds++;
  int16_t Seconds_ars = clock_iniput.out_seconds * 6 - 90;
  lv_img_set_angle(ui->screen_img_6, Seconds_ars * 10);
  if(clock_iniput.out_seconds == 60)
  {
    clock_iniput.out_seconds = 0;
    clock_iniput.out_minutes++;
    int16_t Minutes_ars = clock_iniput.out_minutes * 6 - 90;
    lv_img_set_angle(ui->screen_img_5, Minutes_ars * 10);
    if( (clock_iniput.out_minutes == 12) || (clock_iniput.out_minutes == 24) || (clock_iniput.out_minutes == 36) || (clock_iniput.out_minutes == 48) || (clock_iniput.out_minutes == 60))
    bat = 1;
    else
    bat = 0;
  }
  if(clock_iniput.out_minutes == 60)
  {
    clock_iniput.minutes = 0;
  }
  if( bat == 1 )
  {
    bat = 0;
    clock_iniput.out_Hours++;
    int16_t Hours_ars = clock_iniput.out_Hours * 6 - 90;
    lv_img_set_angle(ui->screen_img_4, Hours_ars * 10);
  }
  if(clock_iniput.out_Hours == 60)
  {
    clock_iniput.out_Hours = 0;
  }
}
void SetTheClock_start(lv_ui *ui)
{
  const esp_timer_create_args_t clock_tick_timer_args = 
  {
    .callback = &clock_task_callback,
    .name = "clock_task",
    .arg = ui,
  };
  esp_timer_handle_t clock_tick_timer = NULL;
  ESP_ERROR_CHECK(esp_timer_create(&clock_tick_timer_args, &clock_tick_timer));
  ESP_ERROR_CHECK(esp_timer_start_periodic(clock_tick_timer, 1000 * 1000));              //1s
}
void clock_init(void)
{
  clock_iniput.Hours = 6;
  clock_iniput.minutes = 0;
  clock_iniput.seconds = 0;
  out_time(&clock_iniput);
  SetTheClock_start(&guider_ui);    //启动时钟
}
void example_rgb_task(void *arg)
{
  lv_ui *ui = (lv_ui *)arg;
  lv_obj_clear_flag(ui->screen_carousel_1,LV_OBJ_FLAG_SCROLLABLE); //不可移动
  lv_obj_clear_flag(ui->screen_cont_1,LV_OBJ_FLAG_HIDDEN); //显示
  lv_obj_add_flag(ui->screen_cont_2, LV_OBJ_FLAG_HIDDEN);

  lv_obj_clear_flag(ui->screen_img_1,LV_OBJ_FLAG_HIDDEN); //显示
  lv_obj_add_flag(ui->screen_img_2, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(ui->screen_img_3, LV_OBJ_FLAG_HIDDEN);
  vTaskDelay(pdMS_TO_TICKS(1500));
  lv_obj_clear_flag(ui->screen_img_2,LV_OBJ_FLAG_HIDDEN); //显示
  lv_obj_add_flag(ui->screen_img_1, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(ui->screen_img_3, LV_OBJ_FLAG_HIDDEN);
  vTaskDelay(pdMS_TO_TICKS(1500));
  lv_obj_clear_flag(ui->screen_img_3,LV_OBJ_FLAG_HIDDEN); //显示
  lv_obj_add_flag(ui->screen_img_2, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(ui->screen_img_1, LV_OBJ_FLAG_HIDDEN);
  vTaskDelay(pdMS_TO_TICKS(1500));
  lv_obj_clear_flag(ui->screen_cont_2,LV_OBJ_FLAG_HIDDEN); //显示
  lv_obj_add_flag(ui->screen_cont_1, LV_OBJ_FLAG_HIDDEN);  
  lv_obj_add_flag(ui->screen_carousel_1,LV_OBJ_FLAG_SCROLLABLE); //可移动
  vTaskDelete(NULL); //删除任务
}
#endif