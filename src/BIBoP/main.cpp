#include <Arduino.h>
#include <Wire.h>
#include <Scheduler.h>

#include "Collector.h"
#include "NetworkManager.h"
#include "Display.h"
// because of how arduino makefile builds dependencies, this finds proper files in the libs/ dir - they are otherwise unused here
#include "Defines.h"
#include "Secrets.h"
#include <MAX30105.h>
#include <WiFiNINA.h>
// in order to make WiFiNINA build, I had to remove one UDP.h in cores/arduino and move the IPAdress raw_addr to public
#include <SPI.h> // required to build wifinina
#include <ArduinoECCX08.h>
#include <ArduinoBearSSL.h>
#include <ACROBOTIC_SSD1306.h>

Collector collector;
Display display;
WiFiClient lambda;
BearSSLClient sslLambda(lambda);
NetworkManager networkManager(sslLambda);

Batch batch; // for now this is a static container (could be a ring of data)
//long lastTime = 0;

// C, eh?
void displayLoop();

void printLastData()
{
    Serial.print(" R: ");
    Serial.print(batch.ppg_red);
    Serial.print(" IR: ");
    Serial.print(batch.ppg_ir);
    Serial.println();
}

void setup()
{
    //Serial.begin(115200); // this causes the stall of the program if there is no Serial connected
    //while (!Serial);
    delay(200);

    if (collector.init() != 0)
        while(1);

    //if (networkManager.init(ssid, pass, lambda_serv, certificate) != 0)
    //    while(1);

    // TODO add SSL/TLS and retry posting this to the endpoint
    /*
    int status = -1;
    while (status != 0)
    {

    }
    */
    //Serial.println("Attempt sending the payload.");
    //networkManager.postWiFi(request_body);

    if (display.init() != 0)
        while(1);

    // Start screen update loop
    Scheduler.startLoop(displayLoop);
}

void displayLoop()
{
    display.update(batch);
    delay(500);
}

// TODO: if time becomes a hindrance -> need to stop doing data processing in the collector
void loop()
{
    //long d = millis() - lastTime;
    //lastTime = millis();
    //Serial.print("DELAY ");
    //Serial.println(d);
    //Serial.println("looping...");
    //networkManager.readWiFi();
    //if (networkManager.serverDisconnectedWiFi())
    //{
    //    Serial.println("Disconnected from the server. Client stopped.");
    //    while (1);
    //}
    // collect the data
    collector.getData();
    collector.getLastData(batch);
    // perform the inference if needed
    printLastData();
    //networkManager.postWiFi()
    yield();
}

