#include "NetworkManager.h"

NetworkManager::NetworkManager(BearSSLClient& sslLambda) : sslLambda(sslLambda)
{

}

NetworkManager::~NetworkManager()
{

}

int NetworkManager::init(const char* ssid, const char* pass, const char* lambda_serv, const char* certificate)
{
    if (!ECCX08.begin())
    {
        Serial.println("Could not initialize ECCX08!");
        return -1;
    }

    ArduinoBearSSL.onGetTime(NetworkManager::getTime);
    sslLambda.setEccSlot(0, certificate);

    if (WiFi.status() == WL_NO_MODULE)
    {
        Serial.println("Communication with WiFi module failed!");
        return -1;
    }


    m_lambda_serv = lambda_serv;
    m_ssid = ssid;
    m_pass = pass;
    return 0;
}

int NetworkManager::postWiFi(const char* buffer)
{
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("WiFi is disconected! Reconnecting");
        connectWiFi();
    }

    if (!sslLambda.connect(m_lambda_serv, 443))
    {
        Serial.println("WiFi is disconected! Run init first!");
        return -1;
    }
    // success - send the data
    sslLambda.println("POST /Prod/classify HTTP/1.1");
    sslLambda.print("Host: ");
    sslLambda.println(m_lambda_serv);
    sslLambda.println("content-type: application/json");
    sslLambda.println("accept: */*");
    sslLambda.print("content-length: ");
    sslLambda.println("1658");
    //lambda.println("connection: close");
    sslLambda.println();
    sslLambda.println(buffer);
    Serial.println("Posted request");
    return 0;
}

void NetworkManager::readWiFi()
{
    while (sslLambda.available())
    {
        Serial.write(sslLambda.read());
    }
}

bool NetworkManager::serverDisconnectedWiFi()
{
    if (!sslLambda.connected())
    {
        sslLambda.stop();
        return true;
    }
    return false;
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

void NetworkManager::connectWiFi()
{
    int status = WL_IDLE_STATUS;

    while (status != WL_CONNECTED)
    {
        Serial.print("Attempting to connect to WPA SSID: ");
        Serial.println(m_ssid);
        status = WiFi.begin(m_ssid, m_pass);
        delay(7000);
    }
    Serial.print("Success!");
    printWifiData();
}

unsigned long NetworkManager::getTime()
{
    return WiFi.getTime();
}
