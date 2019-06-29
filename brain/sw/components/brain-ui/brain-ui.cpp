#include "brain-ui-priv.h"
#include "brain-ui.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

BrainUI::BrainUI() :
    green(LEDC_CHANNEL_0, 4),
    blue(LEDC_CHANNEL_1, 16),
    rgbR(LEDC_CHANNEL_2, 34),
    rgbG(LEDC_CHANNEL_3, 33),
    rgbB(LEDC_CHANNEL_4, 14)
{

}

#define TASK_BRAINUI_STACK_SIZE 2048

// Lower numbers are less important.
#define TASK_BRAINUI_PRIORITY 0

void static task_brainui(void* pvParameters) {
    ((BrainUI*)pvParameters)->_task();
}


void
BrainUI::_task() {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(250);

    uint8_t val = 0;
    while(1) {
        vTaskDelayUntil( &xLastWakeTime, xFrequency );

        val += 4;
        if (val > 255) val = 0;

        ESP_LOGI(TAG, "Update LEDs to %d", val);
        green.setValue(val);
        blue.setValue((uint8_t)255-val);

        rgbR.setValue(val);
        rgbG.setValue(val);
        rgbB.setValue((uint8_t)255-val);
    }

    // Just in case we ever exit, we're supposed to do this.
    // This seems to _work_ more or less, but sure doesn't seem like
    // the safest thing because like, there are callbacks bro!
    vTaskDelete(nullptr);
}

void
BrainUI::start() {
    // Start all the LEDs first
    green.start();
    blue.start();
    rgbR.start();
    rgbG.start();
    rgbB.start();

    // Then we can do stuff
    green.setValue(0);
    blue.setValue(255);


    TaskHandle_t tHandle = NULL;

    BaseType_t tcResult;

    ESP_LOGI(TAG, "Starting brainui task...");
    // vTaskDelay(5 * xPortGetTickRateHz());

    tcResult  = xTaskCreate(task_brainui, "brainui", TASK_BRAINUI_STACK_SIZE,
                            this, TASK_BRAINUI_PRIORITY, &tHandle);

    if (tcResult != pdPASS) {
        ESP_LOGE(TAG, "Failed to create brainui task = %d", tcResult);
    } else {
        ESP_LOGI(TAG, "BrainUI task started");
    }
}