//
// Created by Tom Seago on 2/26/21.
//


#include "screen_driver_ssd1306.h"
#include "brain_common.h"


static const char* TAG = TAG_SCREEN;

ScreenDriverSSD1306::ScreenDriverSSD1306(uint16_t width, uint16_t height,
                                         gpio_num_t pinSCL, gpio_num_t pinSDA) :
        ScreenDriver(width, height),
        m_pinSCL(pinSCL), m_pinSDA(pinSDA)
{
}

ScreenDriverSSD1306::~ScreenDriverSSD1306() {
}

void
ScreenDriverSSD1306::start() {
    ESP_LOGD(TAG, "SSD1306 driver start. Configuring i2c");

    // Install first
    esp_err_t code;
    code = i2c_driver_install(PORT, I2C_MODE_MASTER, 0, 0, 0);
    if (code != ESP_OK) {
        ESP_LOGE(TAG, "Couldn't install I2C driver: %s", esp_err_to_name(code));
        return;
    }

    // Configure the I2C driver second
    i2c_config_t config;
    config.mode = I2C_MODE_MASTER;
    config.sda_io_num = m_pinSDA;
    config.sda_pullup_en = GPIO_PULLUP_ENABLE;
    config.scl_io_num = m_pinSCL;
    config.scl_pullup_en = GPIO_PULLUP_ENABLE;
    config.master.clk_speed = 10000;
    code = i2c_param_config(PORT, &config);
    if (code != ESP_OK) {
        ESP_LOGE(TAG, "Couldn't config I2C: %s", esp_err_to_name(code));
        return;
    }

    ESP_LOGD(TAG, "SSD1306 driver start. Calling reset() and testOne()");
    reset();
     testOne();
    ESP_LOGD(TAG, "SSD1306 driver done.");
}

void
ScreenDriverSSD1306::reset() {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    cmdStart(cmd);

    cmdMuxRatio(cmd,0x3f);
    cmdDisplayOffset(cmd, 0x00);
    cmdDisplayStartLine(cmd, 0x00);
    // set mem mode 0?
    cmdSegmentRemap(cmd, true);
    cmdComOutScanDirection(cmd, false);
    cmdComPinsHwConfig(cmd, 0x12);
    cmdContrast(cmd, 0x7f);
    cmdEntireDisplayOn(cmd, false);
    cmdInvertDisplay(cmd, false);
    cmdClockDivideRatioOscFreq(cmd, 0x0, 0x8);
    cmdChargeBump(cmd, true);
    cmdDisplayOn(cmd, true, true);
    cmdStop(cmd);

    postToQueue(cmd);
}

void
ScreenDriverSSD1306::clear() {
}

void
ScreenDriverSSD1306::send() {
}

void
ScreenDriverSSD1306::testOne() {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    cmdStart(cmd);

    // Setup the addressing so that we go horizontally through all columns
    // in all pages.
    cmdMemAddrMode(cmd, horizontal);
    cmdColAddr(cmd, 0, 127);
    cmdPageAddr(cmd, 0, 7);

    // Co = 0, only data until stop
    // D/C# = 1, data is for GDDRAM
    i2c_master_write_byte(cmd, 0x40, true);

    for(uint8_t row = 0; row < 8; row++) {
        for (uint8_t charPos = 0; charPos < 16; charPos++) {
            uint8_t toWrite = (((row * 8) + charPos) % 2) ? 0xFF : 0x0F;
            for (uint8_t charCol = 0; charCol < 8; charCol++) {
                i2c_master_write_byte(cmd, toWrite, true);
            }
        }
    }

    cmdStop(cmd);

    postToQueue(cmd);
}

void
ScreenDriverSSD1306::handleMsg(void *msg) {
    i2c_cmd_handle_t cmd = msg;
    ESP_LOGI(TAG, "Handling message: %p", cmd);
    auto code = i2c_master_cmd_begin(PORT, cmd, pdMS_TO_TICKS(1000));
    if (code != ESP_OK) {
        ESP_LOGE(TAG, "Error sending i2c command: %s", esp_err_to_name(code));
    } else {
        ESP_LOGI(TAG, "Sent message ok...");
    }

    i2c_cmd_link_delete(msg);
    ESP_LOGI(TAG, "Deletion done");
}
