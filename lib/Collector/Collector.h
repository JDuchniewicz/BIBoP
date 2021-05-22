#pragma once

#include "Defines.h"

#include "MAX30105.h"

class Collector
{
public:
    Collector();
    ~Collector();

    int init(TwoWire& i2c);

    int getData();
    int readData(Batch& batch);

    int getLastData(Batch& batch);

    void collectorOff();
    void collectorOn();
private:
    uint32_t redBuffer[BUFSIZE];
    uint32_t irBuffer[BUFSIZE];
    int idx;
    MAX30105 pulseoximiter;
    // other classes
    long m_lastBeat;
    uint8_t m_heartRates[HR_AVG_WIN_SIZE];
    uint8_t m_hrIdx;
};
