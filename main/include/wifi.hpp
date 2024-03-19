#pragma once

#include "esp_wifi.h"
#include "esp_event.h"
#include "mqtt.hpp"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "netif.hpp"

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

class Wifi
{
private:
    static Wifi *p_instance;
    wifi_config_t wifi_config;
    Wifi();
    ~Wifi();
    Wifi(const Wifi &) {}
    Wifi &operator=(Wifi &) = default;

public:
    static Wifi &getInstance();
    static void init();

    static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                                   int32_t event_id, void *event_data);
};