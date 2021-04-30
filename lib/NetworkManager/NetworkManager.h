#pragma once

#include "WiFiNINA.h"

class NetworkManager
{
public:
    NetworkManager();
    ~NetworkManager();

    int init(const char* ssid, const char* pass);

private:
    void printWifiData();
    void printCurrentNet();

    int status;
};
