//
// Created by Tom Seago on 2/26/21.
//

#pragma once

#include "brain_common.h"
#include <freertos/queue.h>

class ScreenDriver {
public:
    ScreenDriver(uint16_t width, uint16_t height);
    virtual ~ScreenDriver();

    virtual void start() = 0;

    virtual void reset() = 0;
    virtual void clear() = 0;
    virtual void send() = 0;

    uint16_t width() const { return m_width; }
    uint16_t height() const { return m_height; }

    void handleQueue();

protected:
    uint16_t m_width;
    uint16_t m_height;

    uint8_t *m_bufFront;
    QueueHandle_t m_qSend;

    void postToQueue(void *msg);
    virtual void handleMsg(void *msg) = 0;
};