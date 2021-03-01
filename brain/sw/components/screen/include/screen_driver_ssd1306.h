//
// Created by Tom Seago on 2/26/21.
//

#pragma once

#include "screen_driver.h"
#include <driver/i2c.h>


class ScreenDriverSSD1306 : public ScreenDriver {
public:
    ScreenDriverSSD1306(uint16_t width, uint16_t height,
                        gpio_num_t pinSCL, gpio_num_t pinSDA);
    virtual ~ScreenDriverSSD1306();

    virtual void start();

    virtual void reset();
    virtual void clear();
    virtual void send();

private:
    static const i2c_port_t PORT = 1;

    static const uint8_t ADDR_READ  = 0b01111001;
    static const uint8_t ADDR_WRITE = 0b01111000;

    // The alternate version of the addresses
//    static const uint8_t ADDR_READ  = 0b01111011;
//    static const uint8_t ADDR_WRITE = 0b01111010;


    gpio_num_t m_pinSCL;
    gpio_num_t m_pinSDA;

    virtual void handleMsg(void *msg);
    
    void testOne();
    

    void cmdStart(i2c_cmd_handle_t cmd) {
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, ADDR_WRITE, true);
    }

    void cmdStop(i2c_cmd_handle_t cmd) {
        i2c_master_stop(cmd);
    }
    
    /**
     * Contrast ranges for 1 to 256. Higher values mean increased contrast.
     * @param cmd
     * @param contrast
     */
    void cmdContrast(i2c_cmd_handle_t cmd, uint8_t contrast, bool isLast = true) {
        i2c_master_write_byte(cmd, 0x00, true);
        i2c_master_write_byte(cmd, 0x81, true);

        i2c_master_write_byte(cmd, 0x00, true);
        i2c_master_write_byte(cmd, contrast, true);
    }

    void cmdEntireDisplayOn(i2c_cmd_handle_t cmd, bool followRam, bool isLast = true) {
        i2c_master_write_byte(cmd, 0x00, true);
        i2c_master_write_byte(cmd, followRam ? 0xA4 : 0xA5, true);
    }

    void cmdInvertDisplay(i2c_cmd_handle_t cmd, bool inverted, bool isLast = true) {
        i2c_master_write_byte(cmd, 0x00, true);
        i2c_master_write_byte(cmd, inverted ? 0xA7 : 0xA6, true);
    }

    void cmdDisplayOn(i2c_cmd_handle_t cmd, bool on, bool isLast = true) {
        i2c_master_write_byte(cmd, 0x00, true);
        i2c_master_write_byte(cmd, on ? 0xAF : 0xAE, true);
    }

    // TODO: Add Scrolling commands
    void cmdScrollDeactivate(i2c_cmd_handle_t cmd, bool isLast = true) {
        i2c_master_write_byte(cmd, 0x00, true);
        i2c_master_write_byte(cmd, 0x2e, true);
    }

    void cmdScrollActivate(i2c_cmd_handle_t cmd, bool isLast = true) {
        i2c_master_write_byte(cmd, 0x00, true);
        i2c_master_write_byte(cmd, 0x2f, true);
    }

    // Addressing Setting commands
    
    // For page addressing mode
    void cmdPageColAddrLow(i2c_cmd_handle_t cmd, uint8_t addr, bool isLast = true) {
        i2c_master_write_byte(cmd, 0x00, true);
        i2c_master_write_byte(cmd, addr & 0x0f, true);
    }

    // For page addressing mode
    void cmdPageColAddrHigh(i2c_cmd_handle_t cmd, uint8_t addr, bool isLast = true) {
        i2c_master_write_byte(cmd, 0x00, true);
        i2c_master_write_byte(cmd, 0x10 | (addr & 0x0f), true);
    }

    // page 0-7, for page addressing mode
    void cmdPageStart(i2c_cmd_handle_t cmd, uint8_t page, bool isLast = true) {
        i2c_master_write_byte(cmd, 0x00, true);
        i2c_master_write_byte(cmd, 0xb0 | (page & 0x07), true);
    }

    typedef uint8_t memAddrMode_t;
    static const memAddrMode_t horizontal = 0;
    static const memAddrMode_t vertical = 1;
    static const memAddrMode_t page = 2;
    void cmdMemAddrMode(i2c_cmd_handle_t cmd, memAddrMode_t mode, bool isLast = true) {
        i2c_master_write_byte(cmd, 0x00, true);
        i2c_master_write_byte(cmd, 0x20, true);

        i2c_master_write_byte(cmd, 0x00, true);
        i2c_master_write_byte(cmd, mode, true);
    }

    void cmdColAddr(i2c_cmd_handle_t cmd, uint8_t start, uint8_t end, bool isLast = true) {
        i2c_master_write_byte(cmd, 0x00, true);
        i2c_master_write_byte(cmd, 0x21, true);

        i2c_master_write_byte(cmd, 0x00, true);
        i2c_master_write_byte(cmd, start, true);

        i2c_master_write_byte(cmd, 0x00, true);
        i2c_master_write_byte(cmd, end, true);
    }

    void cmdPageAddr(i2c_cmd_handle_t cmd, uint8_t start, uint8_t end, bool isLast = true) {
        i2c_master_write_byte(cmd, 0x00, true);
        i2c_master_write_byte(cmd, 0x22, true);

        i2c_master_write_byte(cmd, 0x00, true);
        i2c_master_write_byte(cmd, start, true);

        i2c_master_write_byte(cmd, 0x00, true);
        i2c_master_write_byte(cmd, end, true);
    }
    
    
    // Hardware Configuration
    void cmdDisplayStartLine(i2c_cmd_handle_t cmd, uint8_t line, bool isLast = true) {
        i2c_master_write_byte(cmd, 0x00, true);
        // TODO: Verify this
        i2c_master_write_byte(cmd, 0x40 | line, true);
    }

    void cmdSegmentRemap(i2c_cmd_handle_t cmd, bool remap, bool isLast = true) {
        i2c_master_write_byte(cmd, 0x00, true);
        i2c_master_write_byte(cmd, 0xA0 | remap, true);
    }

    void cmdMuxRatio(i2c_cmd_handle_t cmd, uint8_t ratio, bool isLast = true) {
        i2c_master_write_byte(cmd, 0x00, true);
        i2c_master_write_byte(cmd, 0xa8, true);

        i2c_master_write_byte(cmd, 0x00, true);
        i2c_master_write_byte(cmd, ratio, true);
    }

    void cmdComOutScanDirection(i2c_cmd_handle_t cmd, bool remap, bool isLast = true) {
        i2c_master_write_byte(cmd, 0x00, true);
        i2c_master_write_byte(cmd, remap ? 0xC8 : 0xC0, true);
    }

    void cmdDisplayOffset(i2c_cmd_handle_t cmd, uint8_t offset, bool isLast = true) {
        i2c_master_write_byte(cmd, 0x00, true);
        i2c_master_write_byte(cmd, 0xd3, true);

        i2c_master_write_byte(cmd, 0x00, true);
        i2c_master_write_byte(cmd, offset, true);
    }

    void cmdComPinsHwConfig(i2c_cmd_handle_t cmd, uint8_t val, bool isLast = true) {
        // TODO: Make this booleans for data A5, A4
        i2c_master_write_byte(cmd, 0x00, true);
        i2c_master_write_byte(cmd, 0xda, true);

        i2c_master_write_byte(cmd, 0x00, true);
        i2c_master_write_byte(cmd, val, true);
    }

    // Timing & Driving Scheme
    
    // Reset would be 0, 8
    void cmdClockDivideRatioOscFreq(i2c_cmd_handle_t cmd, uint8_t divideRatio, uint8_t oscFreq, bool isLast = true) {
        i2c_master_write_byte(cmd, 0x00, true);
        i2c_master_write_byte(cmd, 0xd5, true);

        i2c_master_write_byte(cmd, 0x00, true);
        uint8_t val = (oscFreq << 4) | (divideRatio & 0x0F);
        i2c_master_write_byte(cmd, val, true);
    }

    // Reset would be 2 and 2
    void cmdPrechargePeriod(i2c_cmd_handle_t cmd, uint8_t phase1, uint8_t phase2, bool isLast = true) {
        i2c_master_write_byte(cmd, 0x00, true);
        i2c_master_write_byte(cmd, 0xd9, true);

        i2c_master_write_byte(cmd, 0x00, true);
        uint8_t val = (phase2 << 4) | (phase1 & 0x0F);
        i2c_master_write_byte(cmd, val, true);
    }

    // Valid values are 0, 2, and 3. Reset is 2
    void cmdVcomhDeselect(i2c_cmd_handle_t cmd, uint8_t level, bool isLast = true) {
        i2c_master_write_byte(cmd, 0x00, true);
        i2c_master_write_byte(cmd, 0xdb, true);

        i2c_master_write_byte(cmd, 0x00, true);
        uint8_t val = level << 4;
        i2c_master_write_byte(cmd, val, true);
    }

    void cmdChargeBump(i2c_cmd_handle_t cmd, bool enable, bool isLast = true) {
        i2c_master_write_byte(cmd, 0x00, true);
        i2c_master_write_byte(cmd, 0x8d, true);

        i2c_master_write_byte(cmd, 0x00, true);
        i2c_master_write_byte(cmd, enable ? 0x14 : 0x10, true);
    }

    
};