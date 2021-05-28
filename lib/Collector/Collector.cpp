#include "Collector.h"
#include "heartRate.h"
#include "spo2_algorithm.h"

Collector::Collector() : idx(0), m_lastBeat(0), m_heartRates({0, 0, 0, 0}), m_hrIdx(0)
{

}

Collector::~Collector()
{

}

int Collector::init(TwoWire& i2c)
{
    if (!pulseoximiter.begin(i2c, I2C_SPEED_FAST))
    {
        print("No sensor connected!\n");
        return -1;
    }

    pulseoximiter.setup();
    pulseoximiter.setSampleRate(SAMPLING_HZ); // set 125 Hz
    pulseoximiter.setPulseAmplitudeRed(0x0A);
    pulseoximiter.enableDIETEMPRDY();
    return 0;
}

int Collector::getData() // it is synchroneously driven, could be via interrupts
{
    uint32_t red = pulseoximiter.getRed();
    uint32_t ir = pulseoximiter.getIR();
    print("R: %lu IR: %lu", red, ir);

    redBuffer[idx % INFERENCE_BUFSIZE] = red;
    irBuffer[idx % INFERENCE_BUFSIZE] = ir;
    ++idx;

    return 0;
}

int Collector::readData(Batch& batch)
{
    // read either lower or upper half of the buffer IMPLEMENTATION EASE (or should I say lazyness?)
    // it does not make a big difference anyway
    print("Reading time: idx: %d\n", idx);

    if (idx < INFERENCE_BUFSIZE)
    {
        batch.ppg_red = redBuffer;
        batch.ppg_ir = irBuffer;
        batch.start_idx = 0;
    } else
    {
        batch.ppg_red = &redBuffer[INFERENCE_BUFSIZE - 1];
        batch.ppg_ir = &irBuffer[INFERENCE_BUFSIZE - 1];
        batch.start_idx = INFERENCE_BUFSIZE - 1;
    }
    return 0;
}

// TODO: this probably needs a big rework, getting spo2 and IR threshold + dynamic peak detection
int Collector::getLastData(Batch& batch)
{
    // TODO: change to a circular buffer
    auto i = (idx % INFERENCE_BUFSIZE == 0) ? INFERENCE_BUFSIZE - 1 : idx % INFERENCE_BUFSIZE - 1;

    batch.deviceOk = false;
    print("Batch data: bpm %d, avg: %d, spo2: %d\n", batch.beatsPerMinute, batch.beatAverage, batch.spO2);
    // calculate the heart rate and spO2 (should be split to a new class??)
    if (irBuffer[i] > 25000)
    {
        batch.deviceOk = true;

        if (checkForBeat(irBuffer[i]))
        {
            //print("FOOUND BEAT");
            //print(irBuffer[i]);
            long delta = millis() - m_lastBeat;
            m_lastBeat = millis();

            float beatsPerMinute = (60 / (delta / 1000.0));
            //print(beatsPerMinute);

            batch.beatsPerMinute = beatsPerMinute;

            if (beatsPerMinute < 255 && beatsPerMinute > 20)
            {
                m_heartRates[m_hrIdx++] = (uint8_t)beatsPerMinute;
                m_hrIdx %= HR_AVG_WIN_SIZE;

                uint32_t beatAvg = 0;
                for (uint8_t k = 0; k < HR_AVG_WIN_SIZE; ++k)
                    beatAvg += m_heartRates[k];

                batch.beatAverage = beatAvg / HR_AVG_WIN_SIZE;
            }
        }
        batch.temperature = pulseoximiter.readTemperature(); // TODO: should it be here?
    }

    return 0;
}

void Collector::collectorOn()
{
    pulseoximiter.wakeUp();
}

void Collector::collectorOff()
{
    pulseoximiter.shutDown();
}
