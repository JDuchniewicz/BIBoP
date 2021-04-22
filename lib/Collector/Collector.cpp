#include "Collector.h"

Collector::Collector() : idx(0)
{

}

Collector::~Collector()
{

}

// setup 125 Hz for collection speed
int Collector::init()
{
    if (!pulseoximiter.begin())
    {
        Serial.println("No sensor connected!)");
        return -1;
    }

    pulseoximiter.setup();
    return 0;
}

int Collector::getData()
{
    uint16_t red = pulseoximiter.getRed();
    uint16_t ir = pulseoximiter.getGreen();

    redBuffer[idx % BUFSIZE] = red;
    irBuffer[idx % BUFSIZE] = ir;
    ++idx;

    return 0;
}

int Collector::readData()
{
    return 0;
}

Pair<uint16_t, uint16_t> Collector::getLastData()
{
    return Pair(redBuffer[idx], irBuffer[idx]);
}
