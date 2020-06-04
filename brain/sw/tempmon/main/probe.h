//
// Created by Tom Seago on 6/4/20.
//

#pragma once

#include "brain_common.h"

#include "OneWire.h"
#include "DallasTemp.h"

class Probe {
public:
    Probe();

    void start(TaskDef taskDef);

    void _task();

    float getTemp();

private:

    OneWire m_oneWire;
    DallasTemp m_dallasTemp;

};