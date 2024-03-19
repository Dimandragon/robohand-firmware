#include "netif.hpp"

#include "esp_log.h"

#include <stdexcept>

/**
 * @brief Netif instance
 */
NetIf *Netif::p_instance = 0;

/**
 * @brief Get Netif instance
 *
 * @return NetIf& Netif instance
 */
NetIf &Netif::getInstance()
{
    if (p_instance)
        return *p_instance;

    throw std::runtime_error("NetIf not created");
}

/**
 * @brief Init Netif singleton
 *
 * @param new_instance New NetIf instance
 */
void Netif::init(NetIf &&new_instance)
{
    if (!p_instance)
    {
        p_instance = new NetIf();
    }
    *p_instance = new_instance;
}