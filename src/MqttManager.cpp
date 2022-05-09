#include <MqttManager.h>
#include <PubSubClient.h>
#include <config.h>

//#include "TinyMqtt.h"

// MqttBroker broker(PORT);
WiFiClient espClient;
PubSubClient client(espClient);

// The getter for the instantiated singleton instance
MqttManager_ &MqttManager_::getInstance()
{
    static MqttManager_ instance;
    return instance;
}

// Initialize the global shared instance
MqttManager_ &MqttManager = MqttManager.getInstance();

void callback(char *topic, byte *payload, unsigned int length)
{
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++)
    {
        Serial.print((char)payload[i]);
    }
    Serial.println();

    if ((char)payload[0] == '1')
    {
        ButtonManager.setButtonState(0, true);
    }
    else
    {
        ButtonManager.setButtonState(0, false);
    }
}

long lastReconnectAttempt = 0;

boolean reconnect()
{
    if (client.connect(MQTTprefix, MQTTuser, MQTTpass))
    {
        client.subscribe(MQTTprefix);
    }
    return client.connected();
}

void MqttManager_::setup()
{
    client.setServer(MQTThost, MQTTport);
    client.setCallback(callback);
    lastReconnectAttempt = 0;
}

void MqttManager_::tick()
{
    // if (menuInternalBroker.isActive()) broker.loop();

    if (!client.connected())
    {
        long now = millis();
        if (now - lastReconnectAttempt > 5000)
        {
            lastReconnectAttempt = now;
            // Attempt to reconnect
            if (reconnect())
            {
                lastReconnectAttempt = 0;
            }
        }
    }
    else
    {
        // Client connected
        client.loop();
    }
}

void MqttManager_::publish(const char *topic, const char *payload)
{
    char result[100]; // array to hold the result.
    strcpy(result, MQTTprefix);
    strcat(result, "/");   // copy string one into the result.
    strcat(result, topic); // append string two to the result.
    client.publish(result, payload);
}