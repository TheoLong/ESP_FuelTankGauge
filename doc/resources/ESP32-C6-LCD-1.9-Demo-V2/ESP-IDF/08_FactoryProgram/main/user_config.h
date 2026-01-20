#ifndef USER_CONFIG_H
#define USER_CONFIG_H


#include "esp_err.h"

esp_err_t s_example_write_file(const char *path, char *data);
float sd_card_get_value(void);
esp_err_t s_example_read_file(const char *path,char *pxbuf,uint32_t *outLen);

#endif 


