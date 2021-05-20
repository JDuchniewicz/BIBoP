#pragma once

#include "Defines.h"
#include <WiFiNINA.h>
#include <ArduinoBearSSL.h>
#include <ArduinoECCX08.h>
#include <ArduinoMqttClient.h>

class NetworkManager
{
public:
    NetworkManager(BearSSLClient& sslLambda, MqttClient& mqttClient, Config& config);
    ~NetworkManager();

    int init();
    int postWiFi(const char* buffer);
    void readWiFi();
    bool serverDisconnectedWiFi();

private:
    void printWifiData();
    void printCurrentNet();
    void connectWiFi();
    void connectMqtt();
    void publishMessage(const char* buffer);
    void onMqttMessage(int messageLength);

    static unsigned long getTime();
    static void onMqttMessageTrampoline(void* context, int messageLength);

    BearSSLClient& sslLambda;
    MqttClient& mqttClient;
    Config m_config;
    unsigned long m_lastMillis; //should be long?
    unsigned long m_pollMillis;

    int status;
};
