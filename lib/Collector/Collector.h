#pragma once

#include "Defines.h"

#include "MAX30105.h"

class Collector
{
public:
    Collector();
    ~Collector();

    int init();

    int getData();
    int readData();

    int getLastData(Batch& batch);
private:
    uint16_t redBuffer[BUFSIZE];
    uint16_t irBuffer[BUFSIZE];
    int idx;
    MAX30105 pulseoximiter;
    // other classes
};
