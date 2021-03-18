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

protected:
    void handleReset() override;
    void handleClear() override;
    void handlePattern(enum ScreenDriverCommand::Pattern pattern) override;
    void handleBlit() override;
    void handleWrite(ScreenDriverCommand::WriteData& data) override;

private:
    /**
     * This buffer is laid out in the order we are going to be sending things
     * to the display so that it can be copied directly without juggling
     * data on every display blit.
     *
     * That means each byte is 8 rows of pixels and each character line is
     * 128 pixels wide. This is called a "page" and there are 8 pages.
     *
     * Given bytes a, b, c, d with a0 being the LSB of a and a7 being the MSB of a
     * the pixels are laid out as follows:
     *
     *      x axis --->
     *
     *   y   a0 b0 c0 d0 .....
     *   |   a1 b1 c1 d1 .....
     *   |   a2 b2 c2 d2 .....
     *   V   a3 b3 c3 d3 .....
     *       a4 b4 c4 d4 .....
     *       a5 b5 c5 d5 .....
     *       a6 b6 c6 d6 .....
     *       a7 b7 c7 d7 .....
     *
     * That patterns follows for a single "page" which is 128 bytes long. If the
     * first page is p0, the next p1, p2, p3, etc. where each page contains bytes
     * a, b, c, d, and so on from above the pages themselves (each of
     * which is 8 rows of pixels high) are then laid out as expected:
     *
     *      x axis --->
     *
     *   y   p0: a b c d .....
     *   |   p1: a b c d .....
     *   |   p2: a b c d .....
     *   V   p3: a b c d .....
     *       p4: a b c d .....
     *       p5: a b c d .....
     *       p6: a b c d .....
     *       p7: a b c d .....
     *
     * Eight pages with eight rows in each page adds up to 64 rows of pixels. The
     * pages are stored sequentially in the buffer.
     *
     * Since the driver IC itself doesn't have double buffered memory it doesn't really
     * make much sense to try and double buffer on the MCU side so we don't do that. We
     * have a display buffer that can be draw into and then this buffer will be "blitted"
     * to the device at appropriate times.
     */
    uint8_t m_buffer[8][128];


    static const i2c_port_t PORT = 1;

    /**
     * It's unclear from the datasheet if communication via the I2C interface can
     * read from the driver or not. The way I read it seems to indicate that it can
     * but it's probably not super interesting to do so this value is more for completeness
     * than anything.
     */
    static const uint8_t ADDR_READ  = 0b01111001;

    /**
     * The full address for writing to the I2C device. There are alternate addresses
     * where bit1 is true which would allow multiple displays on a single serial
     * line which might be somewhat interesting in some application or another.
     */
    static const uint8_t ADDR_WRITE = 0b01111000;

    // The alternate version of the addresses
//    static const uint8_t ADDR_READ  = 0b01111011;
//    static const uint8_t ADDR_WRITE = 0b01111010;


    gpio_num_t m_pinSCL;
    gpio_num_t m_pinSDA;

    /**
     * From reading other people's code it seems that "commands" are sent in
     * a single I2C transaction per byte. This seems slow and bad and lame, but
     * at the moment it appears to be required. Perhaps someday with some experimentation
     * it can be figured out if the commands and their data can be strung together
     * in a single serial transaction which would reduce overhead on the serial
     * line dramatically.
     *
     * But for now, we have this. Once everything is better understood this should
     * probably revert back to being an inline function if plausible.
     *
     * @param val
     */
    void sendCmd(uint8_t val);

    // These constants are ported from another library and maybe will go away in
    // favor of having things exposed via good function names.
    static const uint8_t CHARGEPUMP = 0x8D;
    static const uint8_t COLUMNADDR = 0x21;
    static const uint8_t COMSCANDEC = 0xC8;
    static const uint8_t COMSCANINC = 0xC0;
    static const uint8_t DISPLAYALLON = 0xA5;
    static const uint8_t DISPLAYALLON_RESUME = 0xA4;
    static const uint8_t DISPLAYOFF = 0xAE;
    static const uint8_t DISPLAYON = 0xAF;
    static const uint8_t EXTERNALVCC = 0x1;
    static const uint8_t INVERTDISPLAY = 0xA7;
    static const uint8_t MEMORYMODE = 0x20;
    static const uint8_t NORMALDISPLAY = 0xA6;
    static const uint8_t PAGEADDR = 0x22;
    static const uint8_t SEGREMAP = 0xA0;
    static const uint8_t SETCOMPINS = 0xDA;
    static const uint8_t SETCONTRAST = 0x81;
    static const uint8_t SETDISPLAYCLOCKDIV = 0xD5;
    static const uint8_t SETDISPLAYOFFSET = 0xD3;
    static const uint8_t SETHIGHCOLUMN = 0x10;
    static const uint8_t SETLOWCOLUMN = 0x00;
    static const uint8_t SETMULTIPLEX = 0xA8;
    static const uint8_t SETPRECHARGE = 0xD9;
    static const uint8_t SETSEGMENTREMAP = 0xA1;
    static const uint8_t SETSTARTLINE = 0x40;
    static const uint8_t SETVCOMDETECT = 0xDB;
    static const uint8_t SWITCHCAPVCC = 0x02;

    /**
     * Contrast ranges for 1 to 256. Higher values mean increased contrast.
     *
     * This is the command from the datasheet, but it seems that the effective
     * procedure for varying the contrast is better described in setBrightness
     *
     * @param cmd
     * @param contrast
     */
    void cmdContrast(uint8_t contrast) {
        sendCmd(SETCONTRAST);
        sendCmd(contrast);
    }

    /**
     * Ported from another library this is the full procedure that cmdBrightness
     * actually uses. Understanding of the relevance here is kind of low at the
     * moment.
     *
     * @param contrast
     * @param precharge
     * @param comdetect
     */
    void cmdContrastFull(uint8_t contrast, uint8_t precharge, uint8_t comdetect) {
        sendCmd(SETPRECHARGE);
        sendCmd(precharge);

        sendCmd(SETCONTRAST);
        sendCmd(contrast);

        sendCmd(SETVCOMDETECT);
        sendCmd(comdetect);

        sendCmd(DISPLAYALLON_RESUME);
        sendCmd(NORMALDISPLAY);
        sendCmd(DISPLAYON);
    }

    /**
     * Uses some internal magic to directly translate from "brightness" to
     * contrast settings.
     * @param brightness value from 0 to 255 (maybe 1 to 255)
     */
    void cmdBrightness(uint8_t brightness) {
        uint8_t contrast = brightness;
        if (brightness < 128) {
            // Magic
            contrast = brightness * 1.171;
        } else {
            contrast = brightness * 1.172 - 43;
        }

        uint8_t precharge = 241;
        if (!brightness) {
            precharge = 0;
        }
        uint8_t comdetect = brightness / 4;
        cmdContrastFull(contrast, precharge, comdetect);
    }

    /**
     * This allows you to just turn all pixels on the display to the ON state instead
     * of "following what's in RAM" - which seems pointless. Anyway it's good during
     * initialization to tell the driver heh, yeah, use what's in RAM there okay
     * buddy?
     *
     * @param followRam - true to do as one would expect. false to just turn on all the
     *        pixels for god knows what reason.
     */
    void cmdEntireDisplayOn(bool followRam) {
        sendCmd(followRam ? DISPLAYALLON_RESUME : DISPLAYALLON);
    }

    /**
     * Inverts the display in terms of color. With normal mode a 0 bit in memory
     * will be an "OFF" pixel, usually black, whereas a 1 bit will be an "ON" pixel.
     * If inverted is true the opposite will be the case. A 0 will be ON and a 1
     * will be OFF.
     *
     * When post pixels on the display are ON I think the current needs of the display
     * go up and you may get some flicker if using the charge bump method of getting the
     * voltage up to the required level versus supplying a high VCC separately from
     * the VDD interface rail.
     *
     * @param inverted
     */
    void cmdInvertDisplay(bool inverted) {
        if (inverted) {
            sendCmd(INVERTDISPLAY);
        } else {
            sendCmd(NORMALDISPLAY);
        }
    }

    void cmdDisplayOn(bool on) {
        if (on) {
            sendCmd(DISPLAYON);
        } else {
            sendCmd(DISPLAYOFF);
        }
    }

    void cmdScrollDeactivate() {
//        i2c_master_write_byte(cmd, 0x00, true);
//        i2c_master_write_byte(cmd, 0x2e, true);
        sendCmd(0x2E);
    }

    void cmdScrollActivate() {
//        i2c_master_write_byte(cmd, 0x00, true);
//        i2c_master_write_byte(cmd, 0x2f, true);
        sendCmd(0x2F);
    }

    // Addressing Setting commands
    
    // For page addressing mode
    void cmdPageColAddrLow(uint8_t addr) {
//        i2c_master_write_byte(cmd, 0x00, true);
//        i2c_master_write_byte(cmd, addr & 0x0f, true);
        sendCmd(addr & 0x0f);
    }

    // For page addressing mode
    void cmdPageColAddrHigh(uint8_t addr) {
//        i2c_master_write_byte(cmd, 0x00, true);
//        i2c_master_write_byte(cmd, 0x10 | (addr & 0x0f), true);
        sendCmd(0x10 | (addr & 0x0F));
    }

    // page 0-7, for page addressing mode
    void cmdPageStart(uint8_t page) {
//        i2c_master_write_byte(cmd, 0x00, true);
//        i2c_master_write_byte(cmd, 0xb0 | (page & 0x07), true);
        sendCmd(0xB0 | (page & 0x07));
    }

    typedef uint8_t memAddrMode_t;
    static const memAddrMode_t horizontal = 0;
    static const memAddrMode_t vertical = 1;
    static const memAddrMode_t page = 2;
    void cmdMemAddrMode(memAddrMode_t mode) {
//        i2c_master_write_byte(cmd, 0x00, true);
//        i2c_master_write_byte(cmd, 0x20, true);
//
//        i2c_master_write_byte(cmd, 0x00, true);
//        i2c_master_write_byte(cmd, mode, true);
        sendCmd(0x20);
        sendCmd(mode);
    }

    void cmdColAddr(uint8_t start, uint8_t end) {
//        i2c_master_write_byte(cmd, 0x00, true);
//        i2c_master_write_byte(cmd, 0x21, true);
//
//        i2c_master_write_byte(cmd, 0x00, true);
//        i2c_master_write_byte(cmd, start, true);
//
//        i2c_master_write_byte(cmd, 0x00, true);
//        i2c_master_write_byte(cmd, end, true);
        sendCmd(0x21);
        sendCmd(start);
        sendCmd(end);
    }

    void cmdPageAddr(uint8_t start, uint8_t end) {
//        i2c_master_write_byte(cmd, 0x00, true);
//        i2c_master_write_byte(cmd, 0x22, true);
//
//        i2c_master_write_byte(cmd, 0x00, true);
//        i2c_master_write_byte(cmd, start, true);
//
//        i2c_master_write_byte(cmd, 0x00, true);
//        i2c_master_write_byte(cmd, end, true);
        sendCmd(0x22);
        sendCmd(start);
        sendCmd(end);
    }
    
    
    // Hardware Configuration
    void cmdDisplayStartLine(uint8_t line) {
//        i2c_master_write_byte(cmd, 0x00, true);
//        // TODO: Verify this
//        i2c_master_write_byte(cmd, 0x40 | line, true);
        sendCmd(0x40 | line);
    }

    void cmdSegmentRemap(bool remap) {
//        i2c_master_write_byte(cmd, 0x00, true);
//        i2c_master_write_byte(cmd, 0xA0 | remap, true);
        sendCmd(0xA0 | remap);
    }

    void cmdMuxRatio(uint8_t ratio) {
//        i2c_master_write_byte(cmd, 0x00, true);
//        i2c_master_write_byte(cmd, 0xa8, true);
//
//        i2c_master_write_byte(cmd, 0x00, true);
//        i2c_master_write_byte(cmd, ratio, true);
        sendCmd(0xA8);
        sendCmd(ratio);
    }

    /**
     * This inverts/remaps the scan direction from COM0 to COM63 into COM63 to COM0.
     * Effectively what this means is a top to bottom inversion of the rows of the
     * display.
     *
     * This setting interacts with the COM pins hw config values to produce 8 different
     * ways in which the oled panel can be connected to the output of the driver IC.
     *
     * @param remap
     */
    void cmdComOutScanDirection(bool remap) {
//        i2c_master_write_byte(cmd, 0x00, true);
//        i2c_master_write_byte(cmd, remap ? 0xC8 : 0xC0, true);
        sendCmd(remap ? 0xC8 : 0xC0);
    }

    void cmdDisplayOffset(uint8_t offset) {
//        i2c_master_write_byte(cmd, 0x00, true);
//        i2c_master_write_byte(cmd, 0xd3, true);
//
//        i2c_master_write_byte(cmd, 0x00, true);
//        i2c_master_write_byte(cmd, offset, true);
        sendCmd(0xD3);
        sendCmd(offset);
    }

    /**
     * The "COM" pins can be connected from the driver IC to the actual oled screen
     * in multiple ways to allow different PCB layouts. Full screen module assemblies
     * that contain both the driver IC and the screen (the things you can typically
     * by from Amazon, Adafruit, etc) from different manufacturers may require
     * different settings for these two values.
     *
     * From the datasheet these two values interact with the COM out scan direction
     * setting to produce 8 different ways to connect the COM pins to the oled hardware.
     *
     * @param enableLeftRightRemap - the left to right remapping here refers to sides
     *        of the SSD1306 IC itself. An incorrect setting will likely cause an
     *        interlacing artifact or issue. This does NOT swap the data stream to
     *        pixel mapping from left to right on the display.
     * @param useAlternateComPinConfig - using the standard config (false) will map
     *        the first 32 COM pins to the first 32 rows on the screen and the second
     *        block of 32 to the second block of rows. The alternate config (true)
     *        will interlace the two sets of 32. Try one value here and then the other
     *        to see if you are getting interlaced rows or not.
     */
    void cmdComPinsHwConfig(bool enableLeftRightRemap, bool useAlternateComPinConfig) {
        // TODO: Make this booleans for data A5, A4
//        i2c_master_write_byte(cmd, 0x00, true);
//        i2c_master_write_byte(cmd, 0xda, true);
//
//        i2c_master_write_byte(cmd, 0x00, true);
//        i2c_master_write_byte(cmd, val, true);
        sendCmd(0xDA);
        uint8_t val = 0x02 | (enableLeftRightRemap << 5) | (useAlternateComPinConfig << 4);
        sendCmd(val);
    }

    // Timing & Driving Scheme
    
    // Reset would be 0, 8
    void cmdClockDivideRatioOscFreq(uint8_t divideRatio = 0, uint8_t oscFreq = 8) {
//        i2c_master_write_byte(cmd, 0x00, true);
//        i2c_master_write_byte(cmd, 0xd5, true);
//
//        i2c_master_write_byte(cmd, 0x00, true);
        uint8_t val = (oscFreq << 4) | (divideRatio & 0x0F);
//        i2c_master_write_byte(cmd, val, true);
        sendCmd(0xD5);
        sendCmd(val);
    }

    // Reset would be 2 and 2
    void cmdPrechargePeriod(uint8_t phase1, uint8_t phase2) {
//        i2c_master_write_byte(cmd, 0x00, true);
//        i2c_master_write_byte(cmd, 0xd9, true);
//
//        i2c_master_write_byte(cmd, 0x00, true);
        uint8_t val = (phase2 << 4) | (phase1 & 0x0F);
//        i2c_master_write_byte(cmd, val, true);
        sendCmd(0xD9);
        sendCmd(val);
    }

    // Valid values are 0, 2, and 3. Reset is 2
    void cmdVcomhDeselect(uint8_t level) {
//        i2c_master_write_byte(cmd, 0x00, true);
//        i2c_master_write_byte(cmd, 0xdb, true);
//
//        i2c_master_write_byte(cmd, 0x00, true);
        //uint8_t val = level << 4;
//        i2c_master_write_byte(cmd, val, true);
        sendCmd(0xDB);

        // Should val be shifted left by 4? Not sure. Use it raw for now
        sendCmd(level);
    }

    void cmdChargeBump(bool enable) {
//        i2c_master_write_byte(cmd, 0x00, true);
//        i2c_master_write_byte(cmd, 0x8d, true);
//
//        i2c_master_write_byte(cmd, 0x00, true);
//        i2c_master_write_byte(cmd, enable ? 0x14 : 0x10, true);
        sendCmd(0x8D);
        sendCmd(enable ? 0x14 : 0x10);
    }

    
};