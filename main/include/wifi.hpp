#ifndef MAIN_WIFI_H_
#define MAIN_WIFI_H_
//-------------------------------------------------------------
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_wifi.h"
//-------------------------------------------------------------
void wifi_init_sta(void);
//-------------------------------------------------------------
#endif /* MAIN_WIFI_H_ */
