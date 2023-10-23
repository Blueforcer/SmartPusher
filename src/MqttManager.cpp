#include <MqttManager.h>
#include <WiFi.h>
#include <ArduinoHA.h>

WiFiClient espClient;
HADevice device;
HAMqtt mqtt(espClient, device, 20);

HALight *brightness = nullptr;

HASwitch *scroll = nullptr;

HASwitch *led1, *led2, *led3, *led4, *led5, *led6, *led7, *led8 = nullptr;
HASensor *btn1, *btn2, *btn3, *btn4, *btn5, *btn6, *btn7, *btn8 = nullptr;
// HASwitch led1("LED1");
// HASwitch led2("LED2");
// HASwitch led3("LED3");
// HASwitch led4("LED4");
// HASwitch led5("LED5");
// HASwitch led6("LED6");
// HASwitch led7("LED7");
// HASwitch led8("LED8");

// HASensor btn1("BTN1");
// HASensor btn2("BTN2");
// HASensor btn3("BTN3");
// HASensor btn4("BTN4");
// HASensor btn5("BTN5");
// HASensor btn6("BTN6");
// HASensor btn7("BTN7");
// HASensor btn8("BTN8");

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

void onScrollCommand(bool state, HASwitch *sender)
{
    SystemManager.scrolling(state);
    sender->setState(state);
}

void onSwitchCommand(bool state, HASwitch *sender)
{
    if (sender == led1)
    {
        ButtonManager.setButtonState(0, state);
    }
    else if (sender == led2)
    {
        ButtonManager.setButtonState(1, state);
    }
    else if (sender == led3)
    {
        ButtonManager.setButtonState(2, state);
    }
    else if (sender == led4)
    {
        ButtonManager.setButtonState(3, state);
    }
    else if (sender == led5)
    {
        ButtonManager.setButtonState(4, state);
    }
    else if (sender == led6)
    {
        ButtonManager.setButtonState(5, state);
    }
    else if (sender == led7)
    {
        ButtonManager.setButtonState(6, state);
    }
    else if (sender == led8)
    {
        ButtonManager.setButtonState(7, state);
    }
    sender->setState(state); // report state back to the Home Assistant
}

void onMqttMessage(const char *topic, const uint8_t *payload, uint16_t length)
{
    String strTopic = String(topic);
    String strPayload = String((const char *)payload).substring(0, length);

    if (strTopic == SystemManager.MQTT_PREFIX + String("/brightness"))
    {
        SystemManager.setBrightness(atoi(strPayload.c_str()));
        return;
    }
    if (strTopic == SystemManager.MQTT_PREFIX + String("/message"))
    {
        SystemManager.ShowMessage(strPayload);
        return;
    }
    if (strTopic == SystemManager.MQTT_PREFIX + String("/page"))
    {
        SystemManager.showPage(strPayload);
        return;
    }
    if (strTopic == SystemManager.MQTT_PREFIX + String("/scrolling"))
    {
        if (strPayload == "true")
        {
            SystemManager.scrolling(true);
        }
        else
        {
            SystemManager.scrolling(false);
        }
        return;
    }

    if (strTopic == SystemManager.MQTT_PREFIX + String("/image"))
    {
        SystemManager.ShowImage(strPayload);
        return;
    }

    if (strTopic.startsWith(SystemManager.MQTT_PREFIX + "/custompage/"))
    {
        int firstSlash = strTopic.indexOf("/custompage/") + 1;
        int secondSlash = strTopic.indexOf("/", firstSlash) + 1;
        String screenName = strTopic.substring(secondSlash, strTopic.indexOf("/", secondSlash));
        String variableName = strTopic.substring(strTopic.lastIndexOf("/") + 1);
        SystemManager.setCustomPageVariables(screenName, variableName, strPayload);
        return;
    }

    for (int i = 0; i < 8; i++)
    {
        if (strTopic == SystemManager.MQTT_PREFIX + String("/button" + String(i + 1) + "/state"))
        {
            ButtonManager.setButtonState(i, atoi(strPayload.c_str()));
            break;
        }
    }


    if (SystemManager.RGB_BUTTONS)
    {
    if (strTopic == SystemManager.MQTT_PREFIX + String("/button1/Blau"))
    {
       SystemManager.RGBControl(15, atoi(strPayload.c_str()));
       Serial.println("Button1 Blau: " + strPayload);
    }
     if (strTopic == SystemManager.MQTT_PREFIX + String("/button1/Gruen"))
    {
       SystemManager.RGBControl(14, atoi(strPayload.c_str()));
       Serial.println("Button1 Gruen: " + strPayload);
    }
     if (strTopic == SystemManager.MQTT_PREFIX + String("/button2/Blau"))
    {
       SystemManager.RGBControl(13, atoi(strPayload.c_str()));
    }
     if (strTopic == SystemManager.MQTT_PREFIX + String("/button2/Gruen"))
    {
       SystemManager.RGBControl(12, atoi(strPayload.c_str()));
    }

     if (strTopic == SystemManager.MQTT_PREFIX + String("/button3/Blau"))
    {
       SystemManager.RGBControl(11, atoi(strPayload.c_str()));
    }
     if (strTopic == SystemManager.MQTT_PREFIX + String("/button3/Gruen"))
    {
       SystemManager.RGBControl(10, atoi(strPayload.c_str()));
    }

     if (strTopic == SystemManager.MQTT_PREFIX + String("/button4/Blau"))
    {
       SystemManager.RGBControl(9, atoi(strPayload.c_str()));
    }
     if (strTopic == SystemManager.MQTT_PREFIX + String("/button4/Gruen"))
    {
       SystemManager.RGBControl(8, atoi(strPayload.c_str()));
    }

     if (strTopic == SystemManager.MQTT_PREFIX + String("/button5/Blau"))
    {
       SystemManager.RGBControl(7, atoi(strPayload.c_str()));
    }
     if (strTopic == SystemManager.MQTT_PREFIX + String("/button5/Gruen"))
    {
       SystemManager.RGBControl(6, atoi(strPayload.c_str()));
    }

     if (strTopic == SystemManager.MQTT_PREFIX + String("/button6/Blau"))
    {
       SystemManager.RGBControl(5, atoi(strPayload.c_str()));
    }
     if (strTopic == SystemManager.MQTT_PREFIX + String("/button6/Gruen"))
    {
       SystemManager.RGBControl(4, atoi(strPayload.c_str()));
    }

     if (strTopic == SystemManager.MQTT_PREFIX + String("/button7/Blau"))
    {
       SystemManager.RGBControl(3, atoi(strPayload.c_str()));
    }
     if (strTopic == SystemManager.MQTT_PREFIX + String("/button7/Gruen"))
    {
       SystemManager.RGBControl(2, atoi(strPayload.c_str()));
    }

     if (strTopic == SystemManager.MQTT_PREFIX + String("/button8/Blau"))
    {
       SystemManager.RGBControl(1, atoi(strPayload.c_str()));
    }
     if (strTopic == SystemManager.MQTT_PREFIX + String("/button8/Gruen"))
    {
       SystemManager.RGBControl(0, atoi(strPayload.c_str()));
    }

    } // END RGBBUTTON CHECK


}

void onMqttConnected()
{
    String prefix = SystemManager.MQTT_PREFIX;
    if (SystemManager.IO_BROKER)
    {
        for (int i = 1; i < 9; i++)
        {
            MqttManager.publish(("button" + String(i) + "/click").c_str(), "false");
            MqttManager.publish(("button" + String(i) + "/double_click").c_str(), "false");
            MqttManager.publish(("button" + String(i) + "/long_click").c_str(), "false");
            MqttManager.publish(("button" + String(i) + "/push").c_str(), "false");
        }

        MqttManager.publish("brightness", "255");
        MqttManager.publish("message", "Hello from SmartPusher");
        MqttManager.publish("image", "image");
        MqttManager.publish("scrolling", "true");
        MqttManager.publish("page", "time");
        SystemManager.sendCustomPageKeys();
    }
    for (int i = 1; i <= 8; i++)
    {
        String topic = prefix + "/button" + String(i) + "/state";
        mqtt.subscribe(topic.c_str());
    }

  mqtt.subscribe((SystemManager.MQTT_PREFIX + String("/button1/Blau")).c_str());
        mqtt.subscribe((SystemManager.MQTT_PREFIX + String("/button1/Gruen")).c_str());
        
        mqtt.subscribe((SystemManager.MQTT_PREFIX + String("/button2/Blau")).c_str());
        mqtt.subscribe((SystemManager.MQTT_PREFIX + String("/button2/Gruen")).c_str());
        
        mqtt.subscribe((SystemManager.MQTT_PREFIX + String("/button3/Blau")).c_str());
        mqtt.subscribe((SystemManager.MQTT_PREFIX + String("/button3/Gruen")).c_str());
        
        mqtt.subscribe((SystemManager.MQTT_PREFIX + String("/button4/Blau")).c_str());
        mqtt.subscribe((SystemManager.MQTT_PREFIX + String("/button4/Gruen")).c_str());
        
        mqtt.subscribe((SystemManager.MQTT_PREFIX + String("/button5/Blau")).c_str());
        mqtt.subscribe((SystemManager.MQTT_PREFIX + String("/button5/Gruen")).c_str());
        
        mqtt.subscribe((SystemManager.MQTT_PREFIX + String("/button6/Blau")).c_str());
        mqtt.subscribe((SystemManager.MQTT_PREFIX + String("/button6/Gruen")).c_str());
        
        mqtt.subscribe((SystemManager.MQTT_PREFIX + String("/button7/Blau")).c_str());
        mqtt.subscribe((SystemManager.MQTT_PREFIX + String("/button7/Gruen")).c_str());
        
        mqtt.subscribe((SystemManager.MQTT_PREFIX + String("/button8/Blau")).c_str());
        mqtt.subscribe((SystemManager.MQTT_PREFIX + String("/button8/Gruen")).c_str());

    mqtt.subscribe((prefix + String("/brightness")).c_str());
    mqtt.subscribe((prefix + String("/message")).c_str());
    mqtt.subscribe((prefix + String("/scrolling")).c_str());
    mqtt.subscribe((prefix + String("/image")).c_str());
    mqtt.subscribe((prefix + String("/page")).c_str());
    mqtt.subscribe((prefix + String("/custompage/#")).c_str());
    Serial.println("MQTT Connected");
}

void connect()
{

    mqtt.onMessage(onMqttMessage);
    mqtt.onConnected(onMqttConnected);
    if (SystemManager.MQTT_USER == "" || SystemManager.MQTT_PASS == "")
    {
        Serial.println("Connecting to MQTT");
        mqtt.begin(SystemManager.MQTT_HOST.c_str(), SystemManager.MQTT_PORT, nullptr, nullptr, SystemManager.MQTT_PREFIX.c_str());
    }
    else
    {
        Serial.println("Connecting to MQTT");
        mqtt.begin(SystemManager.MQTT_HOST.c_str(), SystemManager.MQTT_PORT, SystemManager.MQTT_USER.c_str(), SystemManager.MQTT_PASS.c_str(), SystemManager.MQTT_PREFIX.c_str());
    }
}

char led1ID[40], led2ID[40], led3ID[40], led4ID[40], led5ID[40], led6ID[40], led7ID[40], led8ID[40], btn1ID[40], btn2ID[40], btn3ID[40], btn4ID[40], btn5ID[40], btn6ID[40], btn7ID[40], btn8ID[40], BriID[40], ScrID[40];

void MqttManager_::setup()
{

    byte mac[6];
    WiFi.macAddress(mac);

    if (SystemManager.HA_DISCOVERY)
    {
        Serial.println("Starting Homeassistant discovery");
        Serial.println(SystemManager.uniqueID);

        uint8_t mac[6];
        WiFi.macAddress(mac);
        char *macStr = new char[18 + 1];
        snprintf(macStr, 24, "%02x%02x%02x", mac[3], mac[4], mac[5]);

        device.setUniqueId(mac, sizeof(mac));
        device.setName(SystemManager.uniqueID);
        device.setSoftwareVersion(SystemManager.VERSION);
        device.setManufacturer("Blueforcer");
        device.setModel("8 Button Array");
        device.setAvailability(true);
        device.enableSharedAvailability();
        device.enableLastWill();
        Serial.println("1");

        sprintf(BriID, "BRI", macStr);
        brightness = new HALight(BriID, HALight::BrightnessFeature);
        brightness->setIcon("mdi:lightbulb");
        brightness->setName("Brightness");
        brightness->onStateCommand(onStateCommand);
        brightness->onBrightnessCommand(onBrightnessCommand); // optional
        brightness->setCurrentState(true);
        brightness->setCurrentBrightness(255);

        sprintf(ScrID, "SCROLL", macStr);
        scroll = new HASwitch(ScrID);
        scroll->onCommand(onScrollCommand);
        scroll->setIcon("mdi:arrow-left-right");
        scroll->setCurrentState(true);
        scroll->setName("Scrolling");

        sprintf(led1ID, "LED1", macStr);
        led1 = new HASwitch(led1ID);
        led1->onCommand(onSwitchCommand);
        led1->setIcon("mdi:led-on");
        led1->setName("LED 1");

        sprintf(led2ID, "LED2", macStr);
        led2 = new HASwitch(led2ID);
        led2->onCommand(onSwitchCommand);
        led2->setIcon("mdi:led-on");
        led2->setName("LED 2");

        sprintf(led3ID, "LED3", macStr);
        led3 = new HASwitch(led3ID);
        led3->onCommand(onSwitchCommand);
        led3->setIcon("mdi:led-on");
        led3->setName("LED 3");

        sprintf(led4ID, "LED4", macStr);
        led4 = new HASwitch(led4ID);
        led4->onCommand(onSwitchCommand);
        led4->setIcon("mdi:led-on");
        led4->setName("LED 4");

        sprintf(led5ID, "LED5", macStr);
        led5 = new HASwitch(led5ID);
        led5->onCommand(onSwitchCommand);
        led5->setIcon("mdi:led-on");
        led5->setName("LED 5");

        sprintf(led6ID, "LED6", macStr);
        led6 = new HASwitch(led6ID);
        led6->onCommand(onSwitchCommand);
        led6->setIcon("mdi:led-on");
        led6->setName("LED 6");

        sprintf(led7ID, "LED7", macStr);
        led7 = new HASwitch(led7ID);
        led7->onCommand(onSwitchCommand);
        led7->setIcon("mdi:led-on");
        led7->setName("LED 7");

        sprintf(led8ID, "LED8", macStr);
        led8 = new HASwitch(led8ID);
        led8->onCommand(onSwitchCommand);
        led8->setIcon("mdi:led-on");
        led8->setName("LED 8");

        sprintf(btn1ID, "BTN1", macStr);
        btn1 = new HASensor(btn1ID);
        btn1->setIcon("mdi:radiobox-marked");
        btn1->setName("Button 1");
        btn1->setValue("-");

        sprintf(btn2ID, "BTN2", macStr);
        btn2 = new HASensor(btn2ID);
        btn2->setIcon("mdi:radiobox-marked");
        btn2->setName("Button 2");
        btn2->setValue("-");

        sprintf(btn3ID, "BTN3", macStr);
        btn3 = new HASensor(btn3ID);
        btn3->setIcon("mdi:radiobox-marked");
        btn3->setName("Button 3");
        btn3->setValue("-");

        sprintf(btn4ID, "BTN4", macStr);
        btn4 = new HASensor(btn4ID);
        btn4->setIcon("mdi:radiobox-marked");
        btn4->setName("Button 4");
        btn4->setValue("-");

        sprintf(btn5ID, "BTN5", macStr);
        btn5 = new HASensor(btn5ID);
        btn5->setIcon("mdi:radiobox-marked");
        btn5->setName("Button 5");
        btn5->setValue("-");

        sprintf(btn6ID, "BTN6", macStr);
        btn6 = new HASensor(btn6ID);
        btn6->setIcon("mdi:radiobox-marked");
        btn6->setName("Button 6");
        btn6->setValue("-");

        sprintf(btn7ID, "BTN7", macStr);
        btn7 = new HASensor(btn7ID);
        btn7->setIcon("mdi:radiobox-marked");
        btn7->setName("Button 7");
        btn7->setValue("-");

        sprintf(btn8ID, "BTN8", macStr);
        btn8 = new HASensor(btn8ID);
        btn8->setIcon("mdi:radiobox-marked");
        btn8->setName("Button 8");
        btn8->setValue("-");
        Serial.println("Devices setup");
    }
    else
    {
        Serial.println("Homeassistant discovery disabled");
        mqtt.disableHA();
    }
    connect();
}

void MqttManager_::tick()
{
    if (SystemManager.MQTT_HOST != "")
    {
        mqtt.loop();
    }
}

void MqttManager_::publish(const char *topic, const char *payload)
{
    if (!mqtt.isConnected())
        return;
    char result[100];
    strcpy(result, SystemManager.MQTT_PREFIX.c_str());
    strcat(result, "/");
    strcat(result, topic);
    mqtt.publish(result, payload, false);
}

void MqttManager_::HAState(uint8_t btn, const char *state)
{
    if (!SystemManager.HA_DISCOVERY)
        return;
    if (!mqtt.isConnected())
        return;
    switch (btn)
    {
    case 0:
        btn1->setValue(state);
        break;
    case 1:
        btn2->setValue(state);
        break;
    case 2:
        btn3->setValue(state);
        break;
    case 3:
        btn4->setValue(state);
        break;
    case 4:
        btn5->setValue(state);
        break;
    case 5:
        btn6->setValue(state);
        break;
    case 6:
        btn7->setValue(state);
        break;
    case 7:
        btn8->setValue(state);
        break;
    default:
        break;
    }
}
