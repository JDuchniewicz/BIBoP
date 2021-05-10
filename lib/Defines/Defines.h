#pragma once

#include <stdint.h>
// all the defines will be stored here
constexpr int SAMPLING_HZ = 125;
constexpr int BUFSIZE = SAMPLING_HZ * 1; // TODO: tweak the buffer size and dont add delays

struct Batch
{
    uint16_t ppg_red;
    uint16_t ppg_ir; // TODO: for now just return a single most recent value
};
