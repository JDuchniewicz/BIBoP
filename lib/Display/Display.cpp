#include "Display.h"

Display::Display() : mode(255)
{

}

Display::~Display()
{

}

int Display::init()
{
    oled.init();
    //oled.setFont(font5x7);
    oled.clearDisplay();
    oled.setTextXY(0, 0);
    oled.putString("Welcome!");

    return 0; // is int required here?
}

int Display::update(const Batch& batch)
{
    if (!batch.deviceOk)
    {
        if (mode != 0)
            oled.clearDisplay();

        mode = 0;

        oled.setTextXY(2, 0);
        oled.putString("INVALID SIGNAL");
        oled.setTextXY(3, 0);
        oled.putString("PLEASE MOVE BAND");
        oled.setTextXY(4, 0);
        oled.putString("UNTIL THIS DISSAPEARS!");
    }
    else
    {
        if (mode != 1)
            oled.clearDisplay();

        mode = 1;
        // this will be too small... need probably a different oled library
        oled.setTextXY(1, 0);
        oled.putString("BPM: ");
        oled.putNumber(batch.beatAverage);
        oled.setTextXY(3, 0);
        oled.putString("SpO2: ");
        oled.putNumber(batch.spO2);
        oled.setTextXY(4, 0);
        oled.putString("Temperature: ");
        oled.putNumber(batch.temperature); // add celcius degrees
        // add hrv
    }
    return 0;
}
