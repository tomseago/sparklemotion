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
    TaskHandle_t curTask = xTaskGetCurrentTaskHandle();

//    ESP_LOGW(TAG, "Posting to queue: %p <- &%p  (task %p)", msg, &msg, curTask);
    xQueueSend(m_qSend, &msg, 0);
}

void
ScreenDriver::handleQueue() {
    TaskHandle_t curTask = xTaskGetCurrentTaskHandle();
    void* pMsg;

    if (xQueueReceive(m_qSend, &pMsg, pdMS_TO_TICKS(500))) {
//        ESP_LOGW(TAG, "Received from queue: %p <- &%p (task %p)", pMsg, *(void**)pMsg, curTask);
        handleMsg(pMsg);
        ESP_LOGI(TAG, "Message handling complete: %p, (task %p)", pMsg, curTask);
    } else {
//        ESP_LOGI(TAG, "No screen message to handle (task %p)", curTask);
    }
}