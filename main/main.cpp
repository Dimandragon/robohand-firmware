#include "esp_log.h"

#include <stdio.h>
#include "wifi.hpp"
#include "mqtt.hpp"
#include "netif.hpp"

void app_main(void)
{
    esp_log_level_set(FW_TAG, ESP_LOG_VERBOSE);
    esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
    esp_log_level_set("MQTT", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_TCP", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);

    ESP_LOGI(FW_TAG, "heap size: %ld", esp_get_free_heap_size());

    Wifi::init();
    MqttClient::init();

}