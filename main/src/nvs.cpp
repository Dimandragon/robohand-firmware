#include "nvs.hpp"

#include "esp_log.h"

#include <stdexcept>

static const char *TAG = "NVS";

/**
 * @brief NVS instance
 */
Nvs *Nvs::p_instance = 0;

/**
 * @brief Construct a new Nvs
 */
Nvs::Nvs()
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        err = nvs_flash_erase();
        if (err != ESP_OK)
            ESP_LOGE(TAG, "Error erase NVS: %x", err);

        err = nvs_flash_init();
        if (err != ESP_OK)
            ESP_LOGE(TAG, "Error init NVS after erase: %x", err);
    }

    if (err != ESP_OK)
        ESP_LOGE(TAG, "Error init NVS: %x", err);
}

/**
 * @brief destructor for Nvs
 */
Nvs::~Nvs()
{
    delete p_instance;
}

/**
 * @brief Get NVS instance
 *
 * @return Nvs& NVS instance
 */
Nvs &Nvs::getInstance()
{
    return *p_instance;

}

/**
 * @brief Init NVS singleton
 */
void Nvs::init()
{
    if (!p_instance)
    {
        p_instance = new Nvs();
    }
    else
    {
        Nvs &&new_nvs_instance = Nvs();
        *p_instance = new_nvs_instance;
    }
}
