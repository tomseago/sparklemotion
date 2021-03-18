//
// Created by Tom Seago on 2/26/21.
//

#include "screen_driver.h"
#include "brain_common.h"

static const char* TAG = TAG_SCREEN;

ScreenDriver::ScreenDriver(uint16_t width, uint16_t height) :
    m_width(width), m_height(height)
{
}

ScreenDriver::~ScreenDriver() {
}

void
ScreenDriver::doCommand(ScreenDriverCommand& cmd) {
    switch(cmd.kind) {
        case ScreenDriverCommand::Reset:
            handleReset();
            break;

        case ScreenDriverCommand::Clear:
            handleClear();
            break;

        case ScreenDriverCommand::Pattern:
            handlePattern(cmd.data.patternData);
            break;

        case ScreenDriverCommand::Blit:
            handleBlit();
            break;

        case ScreenDriverCommand::Write:
            handleWrite(cmd.data.writeData);
            break;

        case ScreenDriverCommand::Rectangle:
            break;
        case ScreenDriverCommand::Line:
            break;
        case ScreenDriverCommand::Pixmap:
            break;
    }
}