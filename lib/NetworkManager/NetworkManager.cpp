#include "NetworkManager.h"

NetworkManager::NetworkManager() : status(WL_IDLE_STATUS)
{

}

NetworkManager::~NetworkManager()
{

}

int NetworkManager::init(const char* ssid, const char* pass)
{
    if (WiFi.status() == WL_NO_MODULE)
    {
        Serial.println("Communication with WiFi module failed!");
        return -1;
    }

    while (status != WL_CONNECTED)
    {
        Serial.print("Attempting to connect to WPA SSID: ");
        Serial.println(ssid);
        status = WiFi.begin(ssid, pass);
        delay(10000);
    }
    Serial.print("Success!");
    printWifiData();

    return 0;
}

//TODO: extend this printing?
void NetworkManager::printWifiData()
{
   IPAddress ip = WiFi.localIP();
   Serial.print("IPAddress:");
   Serial.println(ip);
   Serial.println(ip);
}

void NetworkManager::printCurrentNet()
{
    Serial.print("SSID:");
    Serial.println(WiFi.SSID());
}
