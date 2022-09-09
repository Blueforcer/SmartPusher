#include <MqttManager.h>
#include "EspMQTTClient.h"
#include <config.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include "HAMqttDevice.h"
//#include "TinyMqtt.h"

// MqttBroker broker(PORT);
WiFiClient espClient;

HAMqttDevice button1("button1", HAMqttDevice::BINARY_SENSOR);
HAMqttDevice light("buttonLight1", HAMqttDevice::LIGHT);

// MQTT client setup
EspMQTTClient client(
    SystemManager.getValue("mqttbroker"), // MQTT broker ip
    SystemManager.getValue("mqttport"),   // MQTT Client name
    SystemManager.getValue("mqttuser"),   // MQTT username
    SystemManager.getValue("mqttpwd"),    // MQTT password
    SystemManager.getValue("mqttprefix"));

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

void onConnectionEstablished()
{

    client.subscribe((SystemManager.getValue("mqttprefix") + String("/brightness")).c_str(), [](const String &payload)
                     { SystemManager.setBrightness(atoi(payload.c_str())); });

    client.subscribe((SystemManager.getValue("mqttprefix") + String("/button1/state")).c_str(), [](const String &payload)
                     { ButtonManager.setButtonState(0, atoi(payload.c_str())); });

    client.subscribe((SystemManager.getValue("mqttprefix") + String("/button2/state")).c_str(), [](const String &payload)
                     { ButtonManager.setButtonState(1, atoi(payload.c_str())); });

    client.subscribe((SystemManager.getValue("mqttprefix") + String("/button3/state")).c_str(), [](const String &payload)
                     { ButtonManager.setButtonState(2, atoi(payload.c_str())); });

    client.subscribe((SystemManager.getValue("mqttprefix") + String("/button4/state")).c_str(), [](const String &payload)
                     { ButtonManager.setButtonState(3, atoi(payload.c_str())); });

    client.subscribe((SystemManager.getValue("mqttprefix") + String("/button5/state")).c_str(), [](const String &payload)
                     { ButtonManager.setButtonState(4, atoi(payload.c_str())); });

    client.subscribe((SystemManager.getValue("mqttprefix") + String("/button6/state")).c_str(), [](const String &payload)
                     { ButtonManager.setButtonState(5, atoi(payload.c_str())); });

    client.subscribe((SystemManager.getValue("mqttprefix") + String("/button7/state")).c_str(), [](const String &payload)
                     { ButtonManager.setButtonState(0, atoi(payload.c_str())); });

    client.subscribe((SystemManager.getValue("mqttprefix") + String("/button8/state")).c_str(), [](const String &payload)
                     { ButtonManager.setButtonState(0, atoi(payload.c_str())); });

    client.subscribe((SystemManager.getValue("mqttprefix") + String("/message")).c_str(), [](const String &payload)
                     { SystemManager.ShowMessage(payload); });

    client.subscribe((SystemManager.getValue("mqttprefix") + String("/image")).c_str(), [](const String &payload)
                     { SystemManager.ShowImage(payload); });
}

void MqttManager_::setup()
{
client.enableDebuggingMessages();
    client.enableLastWillMessage("TestClient/lastwill", "I am going offline"); // You can activate the retain flag by setting the third parameter to true
}

void MqttManager_::tick()
{
    // if (menuInternalBroker.isActive()) broker.loop();
    if (WiFi.isConnected())
    {
        client.loop();
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