//
// Created by Tom Seago on 2019-07-10.
//

#include "screen.h"

static const char* TAG = TAG_SCREEN;



void glue_task(void *pArg) {
    ((Screen*)pArg)->_task();
}

Screen::Screen(ScreenDriver& driver) :
    m_driver(driver)
{

}

void
Screen::start(TaskDef taskDef) {
    auto tcResult = taskDef.createTask(glue_task, this, nullptr);

    if (tcResult != pdPASS) {
        ESP_LOGE(TAG, "Failed to create screen task = %d", tcResult);
    } else {
        ESP_LOGI(TAG, "Screen task started");
    }
    ESP_LOGD(TAG, "Screen.start() end");
}

void
Screen::_task() {
    ESP_LOGD(TAG, "Screen._task()");
    m_driver.start();

    while(true) {
//        ESP_LOGD(TAG, "Screen._task() handleQueue()");
        m_driver.handleQueue();
    }
}