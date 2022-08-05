#include <MqttManager.h>
#include <PubSubClient.h>
#include <config.h>
#include <ArduinoJson.h>
#include <WiFi.h>
//#include "TinyMqtt.h"

// MqttBroker broker(PORT);
WiFiClient espClient;
PubSubClient client(espClient);

byte *buffer;
boolean Rflag = false;
int r_len;

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
    payload[length] = '\0';
    String strTopic = String(topic);
    String strPayload = String((char *)payload);
    Serial.println(strTopic);
    Serial.println(strPayload);
    if (strTopic == SystemManager.getValue("mqttprefix") + String("/brightness"))
    {
        SystemManager.setBrightness(atoi(strPayload.c_str()));
    }
}

long lastReconnectAttempt = 0;

boolean reconnect()
{
    if (client.connect(SystemManager.getValue("mqttprefix"), SystemManager.getValue("mqttuser"), SystemManager.getValue("mqttpwd")))
    {
        client.subscribe((SystemManager.getValue("mqttprefix") + String("/brightness")).c_str());
        Serial.println("MQTT Connected");

        for (int i = 1; i < 9; i++)
        {
            MqttManager.publish(("button" + String(i) + "/click").c_str(), "false");
            MqttManager.publish(("button" + String(i) + "/double_click").c_str(), "false");
            MqttManager.publish(("button" + String(i) + "/long_click").c_str(), "false");
            MqttManager.publish(("button" + String(i) + "/push").c_str(), "false");
        }
    }

    return client.connected();
}

void MqttManager_::setup()
{
    uint16_t port = SystemManager.getInt("mqttport");
    client.setServer(SystemManager.getValue("mqttbroker"), port);
    client.setCallback(callback);
    lastReconnectAttempt = 0;
}

void MqttManager_::tick()
{
    // if (menuInternalBroker.isActive()) broker.loop();
    if (WiFi.isConnected())
    {
        if (!client.connected())
        {
            long now = millis();
            if (now - lastReconnectAttempt > 5000)
            {
                lastReconnectAttempt = now;
                Serial.println("Attempt to connect to MQTT");
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
}

void MqttManager_::publish(const char *topic, const char *payload)
{
    char result[100]; // array to hold the result.
    strcpy(result, SystemManager.getValue("mqttprefix"));
    strcat(result, "/");   // copy string one into the result.
    strcat(result, topic); // append string two to the result.
    client.publish(result, payload);
}