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
    if (strTopic == SystemManager.getValue("mqttprefix") + String("/button1/state"))
    {
        ButtonManager.setButtonState(0, atoi(strPayload.c_str()));
    }
    if (strTopic == SystemManager.getValue("mqttprefix") + String("/button2/state"))
    {
        ButtonManager.setButtonState(1, atoi(strPayload.c_str()));
    }
    if (strTopic == SystemManager.getValue("mqttprefix") + String("/button3/state"))
    {
        ButtonManager.setButtonState(2, atoi(strPayload.c_str()));
    }
    if (strTopic == SystemManager.getValue("mqttprefix") + String("/button4/state"))
    {
        ButtonManager.setButtonState(3, atoi(strPayload.c_str()));
    }
    if (strTopic == SystemManager.getValue("mqttprefix") + String("/button5/state"))
    {
        ButtonManager.setButtonState(4, atoi(strPayload.c_str()));
    }
    if (strTopic == SystemManager.getValue("mqttprefix") + String("/button6/state"))
    {
        ButtonManager.setButtonState(5, atoi(strPayload.c_str()));
    }
    if (strTopic == SystemManager.getValue("mqttprefix") + String("/button7/state"))
    {
        ButtonManager.setButtonState(6, atoi(strPayload.c_str()));
    }
    if (strTopic == SystemManager.getValue("mqttprefix") + String("/button8/state"))
    {
        ButtonManager.setButtonState(7, atoi(strPayload.c_str()));
    }
    if (strTopic == SystemManager.getValue("mqttprefix") + String("/message"))
    {
        SystemManager.ShowMessage(strPayload);
    }
}

long lastReconnectAttempt = 0;

boolean reconnect()
{
    if (client.connect(SystemManager.getValue("mqttprefix"), SystemManager.getValue("mqttuser"), SystemManager.getValue("mqttpwd")))
    {
        client.subscribe((SystemManager.getValue("mqttprefix") + String("/brightness")).c_str());
        client.subscribe((SystemManager.getValue("mqttprefix") + String("/button1/state")).c_str());
        client.subscribe((SystemManager.getValue("mqttprefix") + String("/button2/state")).c_str());
        client.subscribe((SystemManager.getValue("mqttprefix") + String("/button3/state")).c_str());
        client.subscribe((SystemManager.getValue("mqttprefix") + String("/button4/state")).c_str());
        client.subscribe((SystemManager.getValue("mqttprefix") + String("/button5/state")).c_str());
        client.subscribe((SystemManager.getValue("mqttprefix") + String("/button6/state")).c_str());
        client.subscribe((SystemManager.getValue("mqttprefix") + String("/button7/state")).c_str());
        client.subscribe((SystemManager.getValue("mqttprefix") + String("/button8/state")).c_str());
        client.subscribe((SystemManager.getValue("mqttprefix") + String("/message")).c_str());

        for (int i = 1; i < 9; i++)
        {
            MqttManager.publish(("button" + String(i) + "/click").c_str(), "false");
            MqttManager.publish(("button" + String(i) + "/double_click").c_str(), "false");
            MqttManager.publish(("button" + String(i) + "/long_click").c_str(), "false");
            MqttManager.publish(("button" + String(i) + "/push").c_str(), "false");
        }

        Serial.println("MQTT Connected");
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
    char result[100];
    strcpy(result, SystemManager.getValue("mqttprefix"));
    strcat(result, "/");
    strcat(result, topic);
    client.publish(result, payload);
}