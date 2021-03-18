//
// Created by Tom Seago on 2/26/21.
//


#include "screen_driver_ssd1306.h"
#include "brain_common.h"
#include <string.h>

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
    config.master.clk_speed = 700000; // 10000;
    code = i2c_param_config(PORT, &config);
    if (code != ESP_OK) {
        ESP_LOGE(TAG, "Couldn't config I2C: %s", esp_err_to_name(code));
        return;
    }

//    ESP_LOGD(TAG, "SSD1306 driver start. Calling reset() and pattern()");
//    reset();
//     pattern();
    ESP_LOGD(TAG, "SSD1306 driver started.");
}


void
ScreenDriverSSD1306::handleReset() {
    ESP_LOGD(TAG, "handleReset() start");
    cmdDisplayOn(false);
    cmdClockDivideRatioOscFreq(0, 8); // Defaults
    cmdMuxRatio(0x3F); // Equal to height - 1
    cmdDisplayOffset(0);
    cmdDisplayStartLine(0);
    cmdChargeBump(true);
    cmdMemAddrMode(horizontal);
    cmdSegmentRemap(true); // eh??
    cmdComOutScanDirection(true); // remap no, means inc
    cmdComPinsHwConfig(false,true); // This was 0x12
    cmdContrast(0xCF); // derp, ok...

    cmdPrechargePeriod(1, 0xF);
    cmdVcomhDeselect(0x40); // 0x40 is default, to lower contrast use 0

    // If we don't follow ram then I really don't really know where it gets
    // things from but they don't work!
    cmdEntireDisplayOn(true);
    cmdInvertDisplay(false);
    cmdScrollDeactivate();
    cmdDisplayOn(true);

    handleClear();
    handleBlit();
    ESP_LOGD(TAG, "handleReset() end");
}

void
ScreenDriverSSD1306::handleClear() {
    memset(m_buffer, 0, m_width * (m_height / 8));
}

//void
//ScreenDriverSSD1306::handlePattern() {
//    ESP_LOGW(TAG, "Sending test one data.........");
////    cmdMemAddrMode(horizontal);
////    cmdColAddr(0, 127);
////    cmdPageAddr(0, 7);
//
//    for (uint8_t y = 0; y<8; y++) {
//        cmdPageStart(y);
//        cmdPageColAddrLow(0x02); // Why always 2???
//        cmdPageColAddrHigh(0);
//        for (uint8_t x=0; x<8; x++) {
//            i2c_cmd_handle_t link = i2c_cmd_link_create();
//            i2c_master_start(link);
//            i2c_master_write_byte(link, ADDR_WRITE, true);
//            i2c_master_write_byte(link, 0x40, true); // Data bytes
//
//            // Write 16 of them
//            for (uint8_t k = 0; k < 16; k++) {
//                uint8_t toWrite = (((y * 8) + x) % 2) ? 0xF0 : 0x0F;
//                //uint8_t toWrite = 0x05;
//                i2c_master_write_byte(link, toWrite, true);
//            }
//            i2c_master_stop(link);
//
//            auto code = i2c_master_cmd_begin(PORT, link, pdMS_TO_TICKS(1000));
//            if (code != ESP_OK) {
//                ESP_LOGE(TAG, "Error sending i2c command: %s", esp_err_to_name(code));
//            } else {
//                ESP_LOGI(TAG, "Sent i2c command ok...");
//            }
//
//            i2c_cmd_link_delete(link);
//        }
//    }
//}

void
ScreenDriverSSD1306::handlePattern(enum ScreenDriverCommand::Pattern pattern) {
    ESP_LOGI(TAG, "handlePattern %d", pattern);
    switch (pattern) {
        case ScreenDriverCommand::Diagonals:
            // One Page at a time, top to bottom
            for (uint8_t y = 0; y<8; y++) {
                // Two 8x8 cells at a time with a pyramid in each cell
                for (uint8_t cell=0; cell < 16; cell++) {

                    // Cell width of 8
                    for (uint8_t x = 0; x < 8; x++) {
                        uint8_t colData = 0x11 << x;

//                if (y > 0 || cell > 7) {
//                    colData = 0;
//                }
                        m_buffer[y][(cell * 8) + x] = colData;
                    }
                }
            }
            break;

        case ScreenDriverCommand::Checkerboard:
            for (uint8_t y = 0; y<8; y++) {
                for (uint8_t x = 0; x < 128; x++) {
                    uint8_t val = (((x / 8) + (y % 2)) % 2) ? 0xFF : 0x00;
                    //uint8_t val = 0x0f;
                    m_buffer[y][x] = val;
                }
            }
            break;

        case ScreenDriverCommand::CheckerboardAlt:
            for (uint8_t y = 0; y<8; y++) {
                for (uint8_t x = 0; x < 128; x++) {
                    uint8_t val = (((x / 8) + (y % 2)) % 2) ? 0x00 : 0xFF;
//                    uint8_t val = 0xf0;
                    m_buffer[y][x] = val;
                }
            }
            break;
    }
}
//
//void
//ScreenDriverSSD1306::handleBlit() {
//    ESP_LOGW(TAG, "Blitting buffer to device");
//    cmdMemAddrMode(horizontal);
//    cmdColAddr(0, 127);
//    cmdPageAddr(0, 7);
//
//    uint8_t *cursor = &m_buffer[0][0];
//    for (uint8_t y = 0; y<8; y++) {
//        for (uint8_t x=0; x<2; x++) {
//            i2c_cmd_handle_t link = i2c_cmd_link_create();
//            i2c_master_start(link);
//            i2c_master_write_byte(link, ADDR_WRITE, true);
//            i2c_master_write_byte(link, 0x40, true); // Data bytes
//
//            // Write 16 bytes per command. I'm not sure if this is a holdover from
//            // arduino world or a limitation of the ssd1306 memory
//            for (uint8_t k = 0; k < 64; k++) {
//                //uint8_t toWrite = (((y * 8) + x) % 2) ? 0xF0 : 0x0F;
//                //uint8_t toWrite = 0x05;
//                uint8_t toWrite = *(cursor++);
//                i2c_master_write_byte(link, toWrite, false);
//            }
//            i2c_master_stop(link);
//
//            auto code = i2c_master_cmd_begin(PORT, link, pdMS_TO_TICKS(1000));
//            if (code != ESP_OK) {
//                ESP_LOGE(TAG, "Error blitting: %s", esp_err_to_name(code));
//            } else {
//                // ESP_LOGI(TAG, "Blit'ed 16 bytes");
//            }
//
//            i2c_cmd_link_delete(link);
//        }
//    }
//}

void
ScreenDriverSSD1306::handleBlit() {
    ESP_LOGW(TAG, "Blitting buffer to device");
//    cmdMemAddrMode(horizontal);
    cmdColAddr(0, 127);
    cmdPageAddr(0, 7);

    uint8_t *cursor = &m_buffer[0][0];
    i2c_cmd_handle_t link = i2c_cmd_link_create();
    i2c_master_start(link);
    i2c_master_write_byte(link, ADDR_WRITE, true);
    i2c_master_write_byte(link, 0x40, true); // Data bytes

    i2c_master_write(link, cursor, sizeof(m_buffer), true);
    i2c_master_stop(link);

    auto code = i2c_master_cmd_begin(PORT, link, pdMS_TO_TICKS(1000));
    if (code != ESP_OK) {
        ESP_LOGE(TAG, "Error blitting: %s", esp_err_to_name(code));
    } else {
        // ESP_LOGI(TAG, "Blit'ed 16 bytes");
    }

    i2c_cmd_link_delete(link);
}

void
ScreenDriverSSD1306::handleWrite(ScreenDriverCommand::WriteData& data) {

}






void
ScreenDriverSSD1306::sendCmd(uint8_t val) {
    i2c_cmd_handle_t link = i2c_cmd_link_create();
    i2c_master_start(link);
    i2c_master_write_byte(link, ADDR_WRITE, true);
    i2c_master_write_byte(link, 0x80, true);
    i2c_master_write_byte(link, val, true);
    i2c_master_stop(link);

    auto code = i2c_master_cmd_begin(PORT, link, pdMS_TO_TICKS(1000));
    if (code != ESP_OK) {
        ESP_LOGE(TAG, "Error sending i2c command: %s", esp_err_to_name(code));
    } else {
        ESP_LOGI(TAG, "i2c cmd: %02x", val);
    }

    i2c_cmd_link_delete(link);
}



//
//void
//ScreenDriverSSD1306::reset() {
////    // My sequence from reading the data sheet
////    cmdDisplayOn(false); // test
////    cmdClockDivideRatioOscFreq(0x0, 0x8); // test
////    cmdMuxRatio(0x3f);
////    cmdDisplayOffset(0x00);
////    cmdDisplayStartLine(0x00);
////
////    // Bump here??
////    cmdChargeBump(true);
////
////    // set mem mode 0?
//////    cmdMemAddrMode(horizontal); // test
////
////    cmdSegmentRemap(true);
////    cmdComOutScanDirection(false);
////    cmdComPinsHwConfig(0x12);
////    cmdContrast(0x7f); // Maybe try 0xCF??
////    cmdEntireDisplayOn(false); // Maybe try true??
////    cmdInvertDisplay(false);
//////    cmdClockDivideRatioOscFreq(0x0, 0x8);
//////    cmdChargeBump(true);
////    cmdDisplayOn(true);
//
//    ///////////////////////
//    // Sequence from the OLEDDisplay library
//    cmdDisplayOn(false);
//    cmdClockDivideRatioOscFreq(0, 8); // Defaults
//    cmdMuxRatio(0x3F); // Equal to height - 1
//    cmdDisplayOffset(0);
//    cmdDisplayStartLine(0);
//    cmdChargeBump(true);
//    cmdMemAddrMode(horizontal);
//    cmdSegmentRemap(true); // eh??
//    cmdComOutScanDirection(false); // remap no, means inc
//    cmdComPinsHwConfig(0x12); // eh??
//    cmdContrast(0xCF); // derp, ok...
//
//    cmdPrechargePeriod(1, 0xF);
//    cmdVcomhDeselect(0x40); // 0x40 is default, to lower contrast use 0
//
//    // If we don't follow ram then I really don't really know where it gets
//    // things from but they don't work!
//    cmdEntireDisplayOn(true);
//    cmdInvertDisplay(false);
//    cmdScrollDeactivate();
//    cmdDisplayOn(true);
//}


//
//void
//ScreenDriverSSD1306::pattern() {
////    // Setup the addressing so that we go horizontally through all columns
////    // in all pages.
////    cmdMemAddrMode(horizontal);
////    cmdColAddr(0, 127);
////    cmdPageAddr(0, 7);
//
//    // Co = 0, only data until stop
//    // D/C# = 1, data is for GDDRAM
////    i2c_master_write_byte(cmd, 0x40, true);
////
////    for(uint8_t row = 0; row < 8; row++) {
////        for (uint8_t charPos = 0; charPos < 16; charPos++) {
////            uint8_t toWrite = (((row * 8) + charPos) % 2) ? 0xFF : 0x0F;
////            for (uint8_t charCol = 0; charCol < 8; charCol++) {
////                i2c_master_write_byte(cmd, toWrite, true);
////            }
////        }
////    }
////
////    cmdStop(cmd);
////
////    postToQueue(cmd);
//
//    ESP_LOGW(TAG, "Sending test one data.........");
////    cmdMemAddrMode(horizontal);
////    cmdColAddr(0, 127);
////    cmdPageAddr(0, 7);
//
//    for (uint8_t y = 0; y<8; y++) {
//        cmdPageStart(y);
//        cmdPageColAddrLow(0x02); // Why always 2???
//        cmdPageColAddrHigh(0);
//        for (uint8_t x=0; x<8; x++) {
//            i2c_cmd_handle_t link = i2c_cmd_link_create();
//            i2c_master_start(link);
//            i2c_master_write_byte(link, ADDR_WRITE, true);
//            i2c_master_write_byte(link, 0x40, true); // Data bytes
//
//            // Write 16 of them
//            for (uint8_t k = 0; k < 16; k++) {
////                uint8_t toWrite = (((y * 8) + x) % 2) ? 0xFF : 0x0F;
//                uint8_t toWrite = 0x05;
//                i2c_master_write_byte(link, toWrite, true);
//            }
//            i2c_master_stop(link);
//
//            postToQueue(link);
//        }
//    }
//}
//
//void
//ScreenDriverSSD1306::handleMsg(void *msg) {
//    i2c_cmd_handle_t cmd = msg;
//    ESP_LOGI(TAG, "Handling message: %p", cmd);
//    auto code = i2c_master_cmd_begin(PORT, cmd, pdMS_TO_TICKS(1000));
//    if (code != ESP_OK) {
//        ESP_LOGE(TAG, "Error sending i2c command: %s", esp_err_to_name(code));
//    } else {
//        ESP_LOGI(TAG, "Sent message ok...");
//    }
//
//    i2c_cmd_link_delete(msg);
//    ESP_LOGI(TAG, "Deletion done");
//}
