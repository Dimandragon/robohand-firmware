#include "utils.hpp"
#include <vector>
#include <cstdarg>
#include <string>

#include "esp_log.h"

static const char *TAG = "UTILS";

/**
 * @brief Format string
 *
 * @param fmt Format string
 * @param ... Variadic arguments
 * @return std::string Result
 */
std::string format(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    std::vector<char> v(10);

    while (true)
    {
        va_list args2;
        va_copy(args2, args);

        int res = vsnprintf(v.data(), v.size(), fmt, args2);

        if ((res >= 0) && (res < static_cast<int>(v.size())))
        {
            va_end(args);
            va_end(args2);
            return std::string(v.data());
        }

        size_t size;

        if (res < 0)
            size = v.size() * 2;
        else
            size = static_cast<size_t>(res) + 1;

        v.clear();
        v.resize(size);
        va_end(args2);
    }
}

/**
 * @brief Convert str mac to unit8_t array
 *
 * @param mac_str MAC addr
 * @return uint8_t* MAC addr array
 */
uint8_t *strMacToArray(const char *mac_str)
{
    static uint8_t mac[6];

    for (int i = 0; i < 6; i++)
    {
        std::string mac_sigment = {mac_str[i * 3], mac_str[i * 3 + 1]};
        mac[i] = stoi(mac_sigment, 0, 16);
    }

    return mac;
}