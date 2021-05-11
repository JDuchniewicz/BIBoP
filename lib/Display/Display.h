#pragma once

#include "Defines.h"
#include <ACROBOTIC_SSD1306.h>

class Display
{
public:
    Display();
    ~Display();

    int init();
    int update(const Batch& batch);
private:
    uint8_t mode;
};
