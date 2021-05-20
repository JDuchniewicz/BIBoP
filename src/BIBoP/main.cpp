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
#include <ArduinoMqttClient.h>
#include <ACROBOTIC_SSD1306.h>

Batch batch; // for now this is a static container (could be a ring of data)
Config config(ssid, pass, certificate, broker, incomingTopic, outgoingTopic);

Collector collector;
Display display;
WiFiClient lambda;
BearSSLClient sslLambda(lambda);
MqttClient mqttClient(sslLambda);
NetworkManager networkManager(sslLambda, mqttClient, config);

//long lastTime = 0
volatile long debounce_time = 0;
volatile long current_time = 0;

// C, eh?
void displayLoop();
void buttonIrq();

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
    Serial.begin(115200); // this causes the stall of the program if there is no Serial connected
    while (!Serial);
    delay(200);

    if (collector.init() != 0)
        while(1);

    if (networkManager.init() != 0)
        while(1);

    //if (display.init() != 0)
    //    while(1);

    //// Start screen update loop
    //Scheduler.startLoop(displayLoop);

    //// attach the wake-up interrupt from a button
    //pinMode(2, INPUT_PULLUP);
    //attachInterrupt(2, buttonIrq, FALLING);
}

void displayLoop()
{
    display.update(batch);
    delay(500);
}

void buttonIrq()
{
    current_time = millis();
    if ((current_time - debounce_time) > 300)
    {
        Serial.println("IRQ!");
    }
    debounce_time = current_time;
}

// TODO: if time becomes a hindrance -> need to stop doing data processing in the collector
void loop()
{
    networkManager.postWiFi(request_body);
    //Serial.println("REGULAR!");
    //long d = millis() - lastTime;
    //lastTime = millis();
    //Serial.print("DELAY ");
    //Serial.println(d);
    //Serial.println("looping...");
    networkManager.readWiFi();
    if (networkManager.serverDisconnectedWiFi())
    {
        Serial.println("Disconnected from the server. Client stopped.");
        while (1);
    }
    // collect the data
    //collector.getData();
    //collector.getLastData(batch);
    // perform the inference if needed
    //printLastData();
    yield();
}

