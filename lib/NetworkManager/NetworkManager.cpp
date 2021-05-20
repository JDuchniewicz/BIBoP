#include "NetworkManager.h"

NetworkManager::NetworkManager(BearSSLClient& sslLambda, MqttClient& mqttClient, Config& config) : sslLambda(sslLambda), mqttClient(mqttClient), m_config(config), m_lastMillis(0), m_pollMillis(0)
{

}

NetworkManager::~NetworkManager()
{

}

int NetworkManager::init()
{
    if (!ECCX08.begin())
    {
        Serial.println("Could not initialize ECCX08!");
        return -1;
    }

    ArduinoBearSSL.onGetTime(NetworkManager::getTime);
    sslLambda.setEccSlot(0, m_config.certificate);

    if (WiFi.status() == WL_NO_MODULE)
    {
        Serial.println("Communication with WiFi module failed!");
        return -1;
    }
    mqttClient.setId("BIBOP0");

    mqttClient.onMessage(NetworkManager::onMqttMessageTrampoline);
    mqttClient.registerOwner(this);
    mqttClient.setConnectionTimeout(50 * 1000L); // connection timeout?
    mqttClient.setCleanSession(false);
    return 0;
}

int NetworkManager::postWiFi(const char* buffer)
{
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("WiFi is disconected! Reconnecting");
        connectWiFi();
    }

    // set up conditions for posting
    if (millis() - m_lastMillis > 30000)
    {
        m_lastMillis = millis();
        publishMessage(buffer);
    }

    return 0;
}

void NetworkManager::readWiFi()
{
    if (millis() - m_pollMillis > 1000) // TODO: move timing to outer loop? define constants
    {
        //Serial.println(m_pollMillis);
        //Serial.println("Time to poll");
        m_pollMillis = millis();

        if (!mqttClient.connected())
        {
            Serial.println(mqttClient.connectError());
            connectMqtt();
        }
        mqttClient.poll();
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
        Serial.println(m_config.ssid);
        status = WiFi.begin(m_config.ssid, m_config.pass);
        delay(7000);
    }
    Serial.print("Success!");
    printWifiData();
}

void NetworkManager::connectMqtt()
{
	Serial.print("Attempting connection to MQTT broker: ");
	Serial.print(m_config.broker);
	Serial.println(" ");

	while (!mqttClient.connect(m_config.broker, 8883))
	{
		// failed, retry
		Serial.print(".");
		delay(5000);
	}
	Serial.println();

	Serial.println("You're connected to the MQTT broker");
	Serial.println();

	// subscribe to a topic
	mqttClient.subscribe(m_config.incomingTopic);
}

void NetworkManager::publishMessage(const char* buffer)
{
	Serial.println("Publishing message");
    Serial.println(m_config.outgoingTopic);
    //Serial.println(buffer);

	// send message, the Print interface can be used to set the message contents
	mqttClient.beginMessage(m_config.outgoingTopic, false, 1);
	//mqttClient.print("{\"data\": \"hello world\"}");
	mqttClient.print(buffer);
	int ret = mqttClient.endMessage();
	Serial.println(ret);
}

unsigned long NetworkManager::getTime()
{
    return WiFi.getTime();
}

void NetworkManager::onMqttMessageTrampoline(void* context, int messageLength)
{
    return reinterpret_cast<NetworkManager*>(context)->onMqttMessage(messageLength);
}

void NetworkManager::onMqttMessage(int messageLength)
{
	// we received a message, print out the topic and contents
	Serial.print("Received a message with topic '");
	Serial.print(mqttClient.messageTopic());
	Serial.print("', length ");
	Serial.print(messageLength);
	Serial.println(" bytes:");

	// use the Stream interface to print the contents
	while (mqttClient.available())
	{
		Serial.print((char)mqttClient.read());
	}

	Serial.println();
}
