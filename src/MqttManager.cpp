#include <MqttManager.h>

#include <ArduinoJson.h>
#include <WiFi.h>
#include <ArduinoHA.h>

WiFiClient espClient;
HADevice device;
HAMqtt mqtt(espClient, device, 18);

HALight brightness("BRI", HALight::BrightnessFeature);

HASwitch led1("LED1");
HASwitch led2("LED2");
HASwitch led3("LED3");
HASwitch led4("LED4");
HASwitch led5("LED5");
HASwitch led6("LED6");
HASwitch led7("LED7");
HASwitch led8("LED8");

HASensor btn1("BTN1");
HASensor btn2("BTN2");
HASensor btn3("BTN3");
HASensor btn4("BTN4");
HASensor btn5("BTN5");
HASensor btn6("BTN6");
HASensor btn7("BTN7");
HASensor btn8("BTN8");

// The getter for the instantiated singleton instance
MqttManager_ &MqttManager_::getInstance()
{
    static MqttManager_ instance;
    return instance;
}

// Initialize the global shared instance
MqttManager_ &MqttManager = MqttManager.getInstance();

void onBrightnessCommand(uint8_t brightness, HALight *sender)
{
    SystemManager.setBrightness(brightness);
    sender->setBrightness(brightness);
}

void onStateCommand(bool state, HALight *sender)
{
    SystemManager.BrightnessOnOff(state);
    sender->setState(state);
}

void onSwitchCommand(bool state, HASwitch *sender)
{
    if (sender == &led1)
    {
        ButtonManager.setButtonState(0, state);
    }
    else if (sender == &led2)
    {
        ButtonManager.setButtonState(1, state);
    }
    else if (sender == &led3)
    {
        ButtonManager.setButtonState(2, state);
    }
    else if (sender == &led4)
    {
        ButtonManager.setButtonState(3, state);
    }
    else if (sender == &led5)
    {
        ButtonManager.setButtonState(4, state);
    }
    else if (sender == &led6)
    {
        ButtonManager.setButtonState(5, state);
    }
    else if (sender == &led7)
    {
        ButtonManager.setButtonState(6, state);
    }
    else if (sender == &led8)
    {
        ButtonManager.setButtonState(7, state);
    }
    sender->setState(state); // report state back to the Home Assistant
}

void onMqttMessage(const char *topic, const uint8_t *payload, uint16_t length)
{
    String strTopic = String(topic);
    String strPayload = (const char*)payload;

    Serial.println(strTopic);
    Serial.println(strPayload);

    if (strTopic == SystemManager.mqttprefix + String("/brightness"))
    {
        SystemManager.setBrightness(atoi(strPayload.c_str()));
        return;
    }
    if (strTopic == SystemManager.mqttprefix + String("/message"))
    {

        SystemManager.ShowMessage(strPayload);
        return;
    }
    if (strTopic == SystemManager.mqttprefix + String("/image"))
    {
        SystemManager.ShowImage(strPayload);
        return;
    }
    for (int i = 0; i < 8; i++)
    {
        if (strTopic == SystemManager.mqttprefix + String("/button" + String(i + 1) + "/state"))
        {
            ButtonManager.setButtonState(i, atoi(strPayload.c_str()));
            break;
        }
    }
}

long lastReconnectAttempt = 0;

void onMqttConnected()
{
    String prefix = SystemManager.mqttprefix;
    for (int i = 1; i <= 8; i++)
    {
        String topic = prefix + "/button" + String(i) + "/state";
        mqtt.subscribe(topic.c_str());
    }
    mqtt.subscribe((SystemManager.mqttprefix + String("/brightness")).c_str());
    mqtt.subscribe((SystemManager.mqttprefix + String("/message")).c_str());
    mqtt.subscribe((SystemManager.mqttprefix + String("/image")).c_str());

    for (int i = 1; i < 9; i++)
    {
        MqttManager.publish(("button" + String(i) + "/click").c_str(), "false");
        MqttManager.publish(("button" + String(i) + "/double_click").c_str(), "false");
        MqttManager.publish(("button" + String(i) + "/long_click").c_str(), "false");
        MqttManager.publish(("button" + String(i) + "/push").c_str(), "false");
    }

    Serial.println("MQTT Connected");
}

void connect()
{
    mqtt.onMessage(onMqttMessage);
    mqtt.onConnected(onMqttConnected);
    if (SystemManager.mqttuser == "" || SystemManager.mqttpass == "")
    {

        mqtt.begin(SystemManager.mqtthost.c_str(), SystemManager.mqttport, nullptr, nullptr);
    }
    else
    {
        mqtt.begin(SystemManager.mqtthost.c_str(), SystemManager.mqttport, SystemManager.mqttuser.c_str(), SystemManager.mqttpass.c_str());
    }
}

void MqttManager_::setup()
{

    byte mac[6];
    WiFi.macAddress(mac);
    device.setUniqueId(mac, sizeof(mac));
    device.setName("SmartPusher");
    device.setSoftwareVersion(SystemManager.VERSION);
    device.setManufacturer("Blueforcer");
    device.setModel("8 Button Array");

    brightness.setIcon("mdi:lightbulb");
    brightness.setName("Brightness");
    brightness.onStateCommand(onStateCommand);
    brightness.onBrightnessCommand(onBrightnessCommand); // optional
    brightness.setCurrentState(true);

    led1.onCommand(onSwitchCommand);
    led1.setIcon("mdi:led-on");
    led1.setName("LED 1");

    led2.onCommand(onSwitchCommand);
    led2.setIcon("mdi:led-on");
    led2.setName("LED 2");

    led3.onCommand(onSwitchCommand);
    led3.setIcon("mdi:led-on");
    led3.setName("LED 3");

    led4.onCommand(onSwitchCommand);
    led4.setIcon("mdi:led-on");
    led4.setName("LED 4");

    led5.onCommand(onSwitchCommand);
    led5.setIcon("mdi:led-on");
    led5.setName("LED 5");

    led6.onCommand(onSwitchCommand);
    led6.setIcon("mdi:led-on");
    led6.setName("LED 6");

    led7.onCommand(onSwitchCommand);
    led7.setIcon("mdi:led-on");
    led7.setName("LED 7");

    led8.onCommand(onSwitchCommand);
    led8.setIcon("mdi:led-on");
    led8.setName("LED 8");

    led8.onCommand(onSwitchCommand);
    led8.setIcon("mdi:led-on");
    led8.setName("LED 8");

    btn1.setIcon("mdi:radiobox-marked");
    btn1.setName("Button 1");
    btn1.setValue("-");

    btn2.setIcon("mdi:radiobox-marked");
    btn2.setName("Button 2");
    btn2.setValue("-");

    btn3.setIcon("mdi:radiobox-marked");
    btn3.setName("Button 3");
    btn3.setValue("-");

    btn4.setIcon("mdi:radiobox-marked");
    btn4.setName("Button 4");
    btn4.setValue("-");

    btn5.setIcon("mdi:radiobox-marked");
    btn5.setName("Button 5");
    btn5.setValue("-");

    btn6.setIcon("mdi:radiobox-marked");
    btn6.setName("Button 6");
    btn6.setValue("-");

    btn7.setIcon("mdi:radiobox-marked");
    btn7.setName("Button 7");
    btn7.setValue("-");

    btn8.setIcon("mdi:radiobox-marked");
    btn8.setName("Button 8");
    btn8.setValue("-");

    lastReconnectAttempt = 0;
    connect();
}

void MqttManager_::tick()
{
    if (SystemManager.mqtthost != "")
    {
        mqtt.loop();
    }
}

void MqttManager_::publish(const char *topic, const char *payload)
{
    char result[100];
    strcpy(result, SystemManager.mqttprefix.c_str());
    strcat(result, "/");
    strcat(result, topic);
    mqtt.publish(result, payload, false);
}

void MqttManager_::HAState(uint8_t btn, const char *state)
{
    switch (btn)
    {
    case 0:
        btn1.setValue(state);
        break;
    case 1:
        btn2.setValue(state);
        break;
    case 2:
        btn3.setValue(state);
        break;
    case 3:
        btn4.setValue(state);
        break;
    case 4:
        btn5.setValue(state);
        break;
    case 5:
        btn6.setValue(state);
        break;
    case 6:
        btn7.setValue(state);
        break;
    case 7:
        btn8.setValue(state);
        break;
    default:
        break;
    }
}
