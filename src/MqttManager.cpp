#include <MqttManager.h>
#include <PubSubClient.h>
#include <config.h>
#include <ArduinoJson.h>

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
    if (strTopic == MQTT_PREFIX + String("/brightness"))
    {
        SystemManager.setBrightness(atoi(strPayload.c_str()));
    }
}

long lastReconnectAttempt = 0;

boolean reconnect()
{
    if (client.connect(MQTT_PREFIX, MQTT_USER, MQTT_PASS))
    {
        client.subscribe((MQTT_PREFIX + String("/brightness")).c_str());
        Serial.println("MQTT Connected");

        String json;
        StaticJsonDocument<200> doc;

        
    }

    return client.connected();
}

void MqttManager_::setup()
{
    client.setServer(MQTT_HOST, MQTT_PORT);
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
    strcpy(result, MQTT_PREFIX);
    strcat(result, "/");   // copy string one into the result.
    strcat(result, topic); // append string two to the result.
    client.publish(result, payload);
}