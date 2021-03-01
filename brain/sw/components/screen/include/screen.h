//
// Created by Tom Seago on 2019-07-10.
//

#pragma once

#include "brain_common.h"

#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/portable.h>
#include <freertos/timers.h>

#include "screen_driver.h"

class Screen {
public:
    Screen(ScreenDriver& driver);

    void start(TaskDef taskDef);

    // Semi-private for ESP32 c world
    void _task();
private:
    ScreenDriver& m_driver;
};