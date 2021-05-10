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

int Collector::getLastData(Batch& batch)
{
    // TODO: change to a circular buffer
    auto i = (idx % BUFSIZE == 0) ? BUFSIZE - 1 : idx % BUFSIZE - 1;
    batch.ppg_red = redBuffer[i];
    batch.ppg_ir = irBuffer[i];

    return 0;
}
