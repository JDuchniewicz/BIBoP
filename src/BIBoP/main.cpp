#include <Arduino.h>
#include <Wire.h>
#include "Collector.h"
#include "NetworkManager.h"
// because of how arduino makefile builds dependencies, this finds proper files in the libs/ dir - they are otherwise unused here
#include "Defines.h"
#include "Secrets.h"
#include <MAX30105.h>
#include <WiFiNINA.h>
// in order to make WiFiNINA build, I had to remove one UDP.h in cores/arduino and move the IPAdress raw_addr to public
#include <SPI.h> // required to build wifinina
#include <ArduinoECCX08.h>
#include <ArduinoBearSSL.h>

// For now just implement it here, move out to classes and libraries when more complex stuff is going on

Collector collector;
WiFiClient lambda;
BearSSLClient sslLambda(lambda);
NetworkManager networkManager(sslLambda);

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
    while (!Serial);

    if (collector.init() != 0)
        while(1);

    if (networkManager.init(ssid, pass, lambda_serv, certificate) != 0)
        while(1);

    // TODO add SSL/TLS and retry posting this to the endpoint
    /*
    int status = -1;
    while (status != 0)
    {

    }
    */
    Serial.println("Attempt sending the payload.");
    networkManager.postWiFi(request_body);
}

void loop()
{
    Serial.println("looping...");
    networkManager.readWiFi();
    if (networkManager.serverDisconnectedWiFi())
    {
        Serial.println("Disconnected from the server. Client stopped.");
        while (1);
    }
    // collect the data
    collector.getData();
    // perform the inference if needed
    //printLastData();
    //networkManager.postWiFi()
}

