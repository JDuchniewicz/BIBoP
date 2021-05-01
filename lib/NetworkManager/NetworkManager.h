#pragma once

#include "Defines.h"
#include <WiFiNINA.h>
#include <ArduinoBearSSL.h>
#include <ArduinoECCX08.h>

class NetworkManager
{
public:
    NetworkManager(BearSSLClient& sslLambda);
    ~NetworkManager();

    int init(const char* ssid, const char* pass, const char* lambda_serv, const char* certificate);
    int postWiFi(const char* buffer);
    void readWiFi();
    bool serverDisconnectedWiFi();

private:
    void printWifiData();
    void printCurrentNet();
    void connectWiFi();
    static unsigned long getTime();

    BearSSLClient& sslLambda;
    const char* m_lambda_serv;
    const char* m_ssid;
    const char* m_pass;
    int status;
};
