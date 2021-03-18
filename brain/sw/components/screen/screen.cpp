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
    m_queue = xQueueCreate(100, sizeof(ScreenDriverCommand));
    if (!m_queue) {
        ESP_LOGE(TAG, "OOM: Unable to create send queue");
    }
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

[[noreturn]] void
Screen::_task() {
    ESP_LOGD(TAG, "Screen._task()");
    m_driver.start();

    // Queue up our first commands!
    reset();
    setPattern(ScreenDriverCommand::Checkerboard);
    blit();

    TickType_t intervalLength = pdMS_TO_TICKS(1000);
    TickType_t nextFrameAt = xTaskGetTickCount() + intervalLength;
    enum ScreenDriverCommand::Pattern nextPattern = ScreenDriverCommand::CheckerboardAlt;

    while(true) {
//        ESP_LOGD(TAG, "Screen._task() handleQueue()");
        // TODO: Flatten this function into here
        handleQueue();

        TickType_t now = xTaskGetTickCount();
        if (now > nextFrameAt) {
            setPattern(nextPattern);
            blit();

            nextPattern = (nextPattern == ScreenDriverCommand::Checkerboard) ?
                            ScreenDriverCommand::CheckerboardAlt :
                            ScreenDriverCommand::Checkerboard;
            nextFrameAt = now + intervalLength;
        }
    }
}


void
Screen::postToQueue(ScreenDriverCommand* cmd) {
    // TaskHandle_t curTask = xTaskGetCurrentTaskHandle();

//    ESP_LOGW(TAG, "Posting to queue: %p <- &%p  (task %p)", msg, &msg, curTask);
    if (xQueueSend(m_queue, (void*)cmd, pdMS_TO_TICKS(500)) != pdTRUE) {
        ESP_LOGE(TAG, "Failed to enqueue a screen command!!!!");
    }
}

void
Screen::handleQueue() {
    // TaskHandle_t curTask = xTaskGetCurrentTaskHandle();

    // Might as well put this on the static heap instead of the task stack
    static ScreenDriverCommand cmd;

    if (xQueueReceive(m_queue, &cmd, pdMS_TO_TICKS(500))) {
//        ESP_LOGW(TAG, "Received from queue: %p <- &%p (task %p)", pMsg, *(void**)pMsg, curTask);
        m_driver.doCommand(cmd);
        // ESP_LOGI(TAG, "Message handling complete: %p, (task %p)", pMsg, curTask);
    } else {
//        ESP_LOGI(TAG, "No screen message to handle (task %p)", curTask);
    }
}

void
Screen::reset() {
    ScreenDriverCommand cmd;
    cmd.kind = ScreenDriverCommand::Reset;

    postToQueue(&cmd);
}

void
Screen::clear() {

}

void
Screen::setPattern(enum ScreenDriverCommand::Pattern patternArg) {
    ESP_LOGI(TAG, "setPattern %d", patternArg);

    ScreenDriverCommand cmd;
    cmd.kind = ScreenDriverCommand::Pattern;
    cmd.data.patternData = patternArg;

    postToQueue(&cmd);
}

void
Screen::blit() {
    ScreenDriverCommand cmd;
    cmd.kind = ScreenDriverCommand::Blit;

    postToQueue(&cmd);
}

void
Screen::write(uint8_t* txt, uint8_t len) {

}

void
Screen::rectangle(uint8_t left, uint8_t top, uint8_t width, uint8_t height) {

}
