#include "Collector.h"
#include "heartRate.h"
#include "spo2_algorithm.h"

Collector::Collector() : idx(0), m_hrIdx(0)
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
    pulseoximiter.setSampleRate(SAMPLING_HZ); // set 125 Hz
    pulseoximiter.enableDIETEMPRDY();
    return 0;
}

int Collector::getData()
{
    uint16_t red = pulseoximiter.getRed();
    uint16_t ir = pulseoximiter.getIR();

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

    batch.deviceOk = false;

    // calculate the heart rate and spO2 (should be split to a new class??)
    if (irBuffer[i] > 25000)
    {
        batch.deviceOk = true;

        if (checkForBeat(irBuffer[i]))
        {
            long delta = millis() - m_lastBeat;
            m_lastBeat = delta;

            uint8_t beatsPerMinute = (uint8_t)(60 / (delta / 1000.0));
            if (beatsPerMinute < 255 && beatsPerMinute < 20)
            {
                m_heartRates[m_hrIdx++] = beatsPerMinute;
                m_hrIdx %= HR_AVG_WIN_SIZE;

                uint16_t beatAvg = 0;
                for (uint8_t k = 0; k < HR_AVG_WIN_SIZE; ++k)
                    beatAvg += m_heartRates[k];

                batch.beatAverage = beatAvg / HR_AVG_WIN_SIZE;
            }
        }
        batch.temperature = pulseoximiter.readTemperature(); // TODO: should it be here?
    }

    return 0;
}
