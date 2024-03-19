#pragma once

#include <atomic>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

class SmallMutex
{
private:
    std::atomic<bool> flag;

public:
    /**
     * @brief Construct a new Small Mutex object
     */
    SmallMutex()
    {
        flag.store(false, std::memory_order_relaxed);
    }
    SmallMutex(SmallMutex &t) {}

    /**
     * @brief Lock Small Mutex
     */
    void lock()
    {
        while (flag.exchange(true, std::memory_order_acquire))
        {
            if (xPortInIsrContext())
            {
                portYIELD_FROM_ISR();
            }
            else
            {
                taskYIELD();
            }
        }
    }

    /**
     * @brief Unlock Small Mutex
     */
    void unlock()
    {
        flag.store(false, std::memory_order_release);
    }

    /**
     * @brief Get Small Mutex state
     *
     * @return true - mutex locked
     * @return false - mutex unlocked
     */
    bool isLocked()
    {
        return flag.load(std::memory_order_acquire);
    }
};