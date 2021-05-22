#pragma once

#include <stdint.h>
// all the defines will be stored here
constexpr auto SAMPLING_HZ = 125;
constexpr auto INFERENCE_BUFSIZE = SAMPLING_HZ;
constexpr auto BUFSIZE = INFERENCE_BUFSIZE * 2;

constexpr auto MESSAGE_BUF_SIZE = 4096;

constexpr auto HR_AVG_WIN_SIZE = 4;

constexpr auto JSON_BEGIN = "{\n    \"data\": [";
constexpr auto JSON_END = "]\n}";

struct Batch
{
    uint32_t* ppg_red; // pass only pointers to the array
    uint32_t* ppg_ir; // TODO: the buffer size is for now 125 samples, probably should be 10 times that for best inference
    uint8_t start_idx;

    uint16_t beatAverage;
    float beatsPerMinute;
    uint16_t spO2;
    uint16_t temperature;
    bool deviceOk;
};


struct Config
{
    Config(const char* s, const char* p, const char* c, const char* b, const char* i, const char* o) :
        ssid(s), pass(p), certificate(c), broker(b), incomingTopic(i), outgoingTopic(o) {}
    const char* ssid;
    const char* pass;
    const char* certificate;
    const char* broker;
    const char* incomingTopic;
    const char* outgoingTopic;
};
