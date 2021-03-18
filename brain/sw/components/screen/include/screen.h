//
// Created by Tom Seago on 2019-07-10.
//

#pragma once

#include "brain_common.h"

#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/portable.h>
#include <freertos/timers.h>
#include <freertos/queue.h>

#include "screen_driver.h"

/**
 * The Screen class is the standard interface point for drawing
 * related tasks. It exposes a public API that allows other tasks to
 * draw on the screen, but these drawing commands are handled by
 * the passed in driver subclass and they are called on a separate
 * task that is started for screen drawing.
 *
 * Thus all the methods here are thread safe whereas the ones on
 * ScreenDriver are not.
 */
class Screen {
public:
    Screen(ScreenDriver& driver);

    void start(TaskDef taskDef);

    // Screen Commands
    void reset();
    void clear();
    void setPattern(enum ScreenDriverCommand::Pattern patternArg);
    void blit();
    void write(uint8_t* txt, uint8_t len);
    void rectangle(uint8_t left, uint8_t top, uint8_t width, uint8_t height);
//    void line();
//    void pixmap();

    // Semi-private for ESP32 c world
    [[noreturn]] void _task();
private:
    ScreenDriver& m_driver;

    // A queue of ScreenDriverCommands
    QueueHandle_t m_queue;

    void postToQueue(ScreenDriverCommand* cmd);
    void handleQueue();
};