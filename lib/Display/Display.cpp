#include "Display.h"

Display::Display()
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
    oled.setTextXY(0, 0);
    oled.putString("Red: ");
    oled.putNumber(batch.ppg_red);
    oled.setTextXY(1, 0);
    oled.putString("IR: ");
    oled.putNumber(batch.ppg_ir);
    return 0;
}
