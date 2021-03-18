//
// Created by Tom Seago on 2/26/21.
//

#pragma once

#include "brain_common.h"


struct ScreenDriverCommand {
    enum Kind {
        Reset,
        Clear,
        Pattern,
        Blit,
        Write,
        Rectangle,
        Line,
        Pixmap,
    };

    Kind kind;

    enum Pattern {
        Diagonals,
        Checkerboard,
        CheckerboardAlt,
    };

    struct WriteData {
        uint8_t *data;
        uint32_t len;
    };

    struct RectangleData {
        uint8_t left;
        uint8_t top;
        uint8_t width;
        uint8_t height;
    };

    union CommandData {
        WriteData writeData;
        RectangleData rectangleData;
        enum Pattern patternData;
    } data;
};

/**
 * Handles internal screen buffers and implements drawing commands.
 */
class ScreenDriver {
public:
    ScreenDriver(uint16_t width, uint16_t height);
    virtual ~ScreenDriver();

    virtual void start() = 0;

    uint16_t width() const { return m_width; }
    uint16_t height() const { return m_height; }


    virtual void doCommand(ScreenDriverCommand& cmd);

protected:
    uint16_t m_width;
    uint16_t m_height;

    virtual void handleReset() = 0;
    virtual void handleClear() = 0;
    virtual void handlePattern(enum ScreenDriverCommand::Pattern pattern) = 0;
    virtual void handleBlit() = 0;
    virtual void handleWrite(ScreenDriverCommand::WriteData& data) = 0;
};