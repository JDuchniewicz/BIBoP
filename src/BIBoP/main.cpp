#include <Arduino.h>
#include <Wire.h>
#include "Collector.h"
// because of how arduino makefile builds dependencies, this finds proper files in the libs/ dir - they are otherwise unused here
#include "Defines.h"
#include "MAX30105.h"

// For now just implement it here, move out to classes and libraries when more complex stuff is going on

Collector collector;

void printLastData()
{
    auto samples = collector.getLastData();
    Serial.print(" R: ");
    Serial.print(samples.first);
    Serial.print(" IR: ");
    Serial.print(samples.second);
    Serial.println();
}

void setup()
{
    Serial.begin(115200);
    if (collector.init() != 0)
        while(1);

}

void loop()
{
    // collect the data
    collector.getData();
    // perform the inference if needed
    printLastData();
}

