#pragma once

#include "Defines.h"
#include <ACROBOTIC_SSD1306.h>

class Display
{
public:
    Display();
    ~Display();

    int init(TwoWire& i2c);
    int update(const Batch& batch);

    void displayOff();
    void displayOn();
private:
    uint8_t mode;
};
