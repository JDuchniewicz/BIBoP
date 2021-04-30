#include "NetworkManager.h"

NetworkManager::NetworkManager()
{

}

NetworkManager::~NetworkManager()
{

}

int NetworkManager::init(const char* ssid, const char* pass, const char* lambda_serv)
{
    int status = WL_IDLE_STATUS;
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

    lambda_serv_ip = lambda_serv;
    return 0;
}

int NetworkManager::postWiFi(const char* buffer)
{
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("WiFi is disconected! Run init first!");
        return -1;
    }

    if (!lambda.connect(lambda_serv_ip, 80))
    {
        Serial.println("WiFi is disconected! Run init first!");
        return -1;
    }
    // success - send the data
    lambda.println("POST /Prod/classify HTTP/1.1");
    lambda.print("Host: ");
    lambda.println(lambda_serv_ip);
    lambda.println("content-type: application/json");
    lambda.println("accept: */*");
    lambda.print("content-length: ");
    lambda.println("1658");
    //lambda.println("connection: close");
    lambda.println();
    lambda.println(buffer);
}

void NetworkManager::readWiFi()
{
    while (lambda.available())
    {
        Serial.write(lambda.read());
    }
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
