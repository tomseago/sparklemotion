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
        ESP_LOGE(TAG, "Failed to create probe task = %d", tcResult);
    } else {
        ESP_LOGI(TAG, "Probe task started");
    }
}

void
Screen::_task() {
    m_driver.start();

    while(true) {
        m_driver.handleQueue();
    }
}