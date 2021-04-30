#include "Collector.h"

Collector::Collector() : idx(0)
{

}

Collector::~Collector()
{

}

int Collector::init()
{
    if (!pulseoximiter.begin())
    {
        Serial.println("No sensor connected!)");
        return -1;
    }

    pulseoximiter.setup();
    pulseoximiter.setSampleRate(125); // set 125 Hz
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
    // TODO: change to a circular buffer
    if (idx % BUFSIZE == 0)
        return Pair(redBuffer[BUFSIZE - 1], irBuffer[BUFSIZE - 1]);

    return Pair(redBuffer[idx % BUFSIZE - 1], irBuffer[idx % BUFSIZE - 1]);
}
