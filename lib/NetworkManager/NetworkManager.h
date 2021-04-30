#pragma once

#include "Defines.h"
#include "WiFiNINA.h"

class NetworkManager
{
public:
    NetworkManager();
    ~NetworkManager();

    int init(const char* ssid, const char* pass, const char* lambda_serv);
    int postWiFi(const char* buffer);
void     void readWiFi();

private:
    void printWifiData();
    void printCurrentNet();

    WiFiClient lambda;
    const char* lambda_serv_ip;
    int status;
};
