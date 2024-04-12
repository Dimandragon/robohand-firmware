#pragma once

#include "nvs_flash.h"

class Nvs
{
private:
    static Nvs *p_instance;
    Nvs();
    ~Nvs();
    Nvs(const Nvs &) {}
    Nvs &operator=(Nvs &) = default;

public:
    static Nvs &getInstance();
    static void init();
};