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
    int postWiFi(Batch& batch);
    void readWiFi();
    void reconnectWiFi();
    bool serverDisconnectedWiFi();


private:
    void printWifiData();
    void printCurrentNet();
    void connectWiFi();
    void connectMqtt();
    void publishMessage(const char* buffer);
    void onMqttMessage(int messageLength);

    void prepareMessage(Batch& batch);

    static unsigned long getTime();
    static void onMqttMessageTrampoline(void* context, int messageLength);

    BearSSLClient& sslLambda;
    MqttClient& mqttClient;
    Config& m_config;

    char MESSAGE_BUFFER[MESSAGE_BUF_SIZE];
};
