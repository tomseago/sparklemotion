//
// Created by Tom Seago on 2/26/21.
//

#include "screen_driver.h"
#include "brain_common.h"

static const char* TAG = TAG_SCREEN;

ScreenDriver::ScreenDriver(uint16_t width, uint16_t height) :
    m_width(width), m_height(height)
{
    m_bufFront = (uint8_t*)malloc(m_width * m_height);
    if (!m_bufFront) {
        ESP_LOGE(TAG, "OOM: Unable to create screen buffer");
    }

    m_qSend = xQueueCreate(20, sizeof(void*));
    if (!m_qSend) {
        ESP_LOGE(TAG, "OOM: Unable to create send queue");
    }
}

ScreenDriver::~ScreenDriver() {
    if (m_bufFront) {
        free(m_bufFront);
        m_bufFront = NULL;
    }
}

void
ScreenDriver::postToQueue(void *msg) {
    xQueueSend(m_qSend, msg, 0);
}

void
ScreenDriver::handleQueue() {
    void* pMsg;

    if (xQueueReceive(m_qSend, &pMsg, pdMS_TO_TICKS(500))) {
        handleMsg(pMsg);
    }
}