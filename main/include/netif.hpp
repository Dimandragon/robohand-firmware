#pragma once

#include "esp_netif.h"

typedef esp_netif_t *NetIf;

class Netif
{
private:
    static NetIf *p_instance;

public:
    static NetIf &getInstance();
    static void init(NetIf &&new_instance);
};

