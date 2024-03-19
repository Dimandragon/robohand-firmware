#include "utils.hpp"
#include "wifi.hpp"
#include "mqtt.hpp"
#include "esp_log.h"
#include "esp_mac.h"
#include "freertos/FreeRTOS.h"
#include "netif.hpp"

#include <string>

/**
 * @brief Wifi instance
 */
Wifi *Wifi::p_instance = 0;

auto FW_TAG = CONFIG_WIFI_SSID;

/**
 * @brief WIFI events handler
 *
 * @param arg Handler argument
 * @param event_base Event type
 * @param event_id Event ID
 * @param event_data Event data
 */
void Wifi::wifi_event_handler(void *arg, esp_event_base_t event_base,
                              int32_t event_id, void *event_data)
{
    esp_err_t err;

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
        ESP_LOGI(FW_TAG, "in wifi heap size: %ld", esp_get_free_heap_size());
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        vTaskDelay(pdMS_TO_TICKS());
        err = esp_wifi_connect();
        if (err != ESP_OK)
        {
            ESP_LOGE(FW_TAG, "connect to the AP fail: %s", esp_err_to_name(err));
        }
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = static_cast<ip_event_got_ip_t *>(event_data);
        ESP_LOGD(FW_TAG, "Connected to SSID: %s", CONFIG_WIFI_SSID);
        ESP_LOGD(FW_TAG, "ip: " IPSTR, IP2STR(&event->ip_info.ip));
    }
}

/**
 * @brief Construct a new Wifi
 */
Wifi::Wifi()
{
    // Set mac addr
        const uint8_t *mac = (const uint8_t *)CONFIG_MAC;
        esp_err_t err = esp_base_mac_addr_set(mac);
        if (err == ESP_OK)
            ESP_LOGD(FW_TAG, "MAC address successfully set to %02x:%02x:%02x:%02x:%02x:%02x",
                     mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        else
            ESP_LOGD(FW_TAG, "Failed to set MAC address");
    

    ESP_LOGI(FW_TAG, "wifi_init start");
    ESP_ERROR_CHECK(esp_netif_init());
    Netif::init(esp_netif_create_default_wifi_sta());

    // Init wifi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;

    // Setup wifi and IP events
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                                        &this->wifi_event_handler,
                                                        static_cast<void *>(this),
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                                        &this->wifi_event_handler,
                                                        static_cast<void *>(this),
                                                        &instance_got_ip));

    // WIFI creds (SSID, PASS, AUTH)
    wifi_config = wifi_config_t{
        .sta = {
            .threshold = {
                .authmode = (wifi_auth_mode_t)CONFIG_WIFI_SECURITY_STANDART,
            },
        },
    };

    // Set WIFI SSID
    std::string ssid = CONFIG_WIFI_SSID;
    std::copy(ssid.begin(), ssid.end(), std::begin(wifi_config.sta.ssid));

    // Set WIFI pass
    std::string pass = CONFIG_WIFI_PASSWORD;
    std::copy(pass.begin(), pass.end(), std::begin(wifi_config.sta.password));

    // Setup wifi config and start
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(FW_TAG, "wifi_init finished");
}

/**
 * @brief Destroy the Wifi
 */
Wifi::~Wifi()
{
    esp_wifi_stop();
};

/**
 * @brief Get Wifi instance
 *
 * @return Wifi& Wifi instance
 */
Wifi &Wifi::getInstance()
{
    if (p_instance)
        return *p_instance;
}

/**
 * @brief Init Wifi singleton
 */
void Wifi::init()
{
    if (!p_instance)
    {
        p_instance = new Wifi();
    }
    else
    {
        Wifi &&new_wifi_instance = Wifi();
        *p_instance = new_wifi_instance;
    }
}