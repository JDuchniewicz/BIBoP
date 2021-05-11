#pragma once

#include <stdint.h>
// all the defines will be stored here
constexpr auto SAMPLING_HZ = 125;
constexpr auto BUFSIZE = SAMPLING_HZ * 1;

constexpr auto HR_AVG_WIN_SIZE = 4;

struct Batch
{
    uint32_t ppg_red;
    uint32_t ppg_ir; // TODO: for now just return a single most recent value

    uint16_t beatAverage;
    float beatsPerMinute;
    uint16_t spO2;
    uint16_t temperature;
    bool deviceOk;
};

