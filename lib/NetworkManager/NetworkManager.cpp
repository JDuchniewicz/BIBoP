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

void NetworkManager::reconnectWiFi()
{
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("WiFi is disconected! Reconnecting");
        connectWiFi();
    }
}

int NetworkManager::postWiFi(Batch& batch)
{
    prepareMessage(batch);
    publishMessage(MESSAGE_BUFFER);
    return 0;
}

void NetworkManager::readWiFi()
{
    if (!mqttClient.connected())
    {
        Serial.println(mqttClient.connectError());
        connectMqtt();
    }
    mqttClient.poll();
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

void NetworkManager::prepareMessage(Batch& batch)
{
    Serial.println("preparing message");
    // we could create it using a library, let's do it by hand
    // write beginning
    char* buffer_pos = MESSAGE_BUFFER;
    uint8_t len = 0;

    len = sprintf(buffer_pos, "%s", JSON_BEGIN);
    buffer_pos += len;

    // write the array body
    for (uint8_t i = 0; i < INFERENCE_BUFSIZE; ++i)
    {
        len = sprintf(buffer_pos, "%lu", batch.ppg_red[batch.start_idx + i]);
        buffer_pos += len;
        len = sprintf(buffer_pos, "%s", ", ");
        buffer_pos += len;
    }
    buffer_pos -= 2; // remove last ", "

    // end the array and add string termination
    len = sprintf(buffer_pos, "%s", JSON_END);
    buffer_pos += len;
    sprintf(buffer_pos, "%s", "\0");

    Serial.println(MESSAGE_BUFFER);
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
