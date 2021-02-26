//
// Created by Tom Seago on 6/4/20.
//

#pragma once

#include "brain_common.h"

#include "OneWire.h"
#include "DallasTemp.h"
#include "DHTSensor.h"

class Probe : public DHTSensorListener {
public:
    Probe();

    void start(TaskDef taskDef);

    void _task();

    float getTemp();

    virtual void dhtSensorReadData(DHTSensor& sensor);

private:
    uint32_t m_notifCount;

    OneWire m_oneWire;
    DallasTemp m_dallasTemp;
    DHTSensor m_dhtSensor;
};