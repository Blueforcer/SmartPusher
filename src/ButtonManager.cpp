#include <ButtonManager.h>
#include <SystemManager.h>
#include <ArduinoJson.h>

bool ResetLights = false;
unsigned long prevMillis = 0;
int demoState = 0;

// The getter for the instantiated singleton instance
ButtonManager_ &ButtonManager_::getInstance()
{
    static ButtonManager_ instance;
    return instance;
}

// Initialize the global shared instance
ButtonManager_ &ButtonManager = ButtonManager.getInstance();

// The "static" event handler simply calls the non-static event handler on the
// shared singleton instance
void staticHandleEvent(AceButton *button, uint8_t eventType, uint8_t buttonState)
{
    ButtonManager.handleEvent(button, eventType, buttonState);
}

void ButtonManager_::setup()
{
    for (int i = 0; i < 8; i++)
    {
        // Initialize the AceButton objects
        buttons.push_back(new AceButton(buttonPins[i]));
        // Set the input mode of the button pin
        pinMode(buttonPins[i], INPUT_PULLUP);
    }

    ButtonConfig *buttonConfig = ButtonConfig::getSystemButtonConfig();
    buttonConfig->setEventHandler(staticHandleEvent);
    buttonConfig->setFeature(ButtonConfig::kFeatureClick);
    buttonConfig->setFeature(ButtonConfig::kFeatureDoubleClick);
    buttonConfig->setFeature(ButtonConfig::kFeatureLongPress);
    buttonConfig->setFeature(ButtonConfig::kFeatureRepeatPress);
    buttonConfig->setFeature(
        ButtonConfig::kFeatureSuppressClickBeforeDoubleClick);
    buttonConfig->setClickDelay(300);
    buttonConfig->setDoubleClickDelay(600);
    setStates();
}

void ButtonManager_::tick()
{
    for (int i = 0; i < 8; i++)
    {
        leds[i].Update();
    }

    // Check if the menu buttons are being pressed. If so, show menu
    checkButtons();
}

AceButton *ButtonManager_::getButton(uint8_t index)
{
    return buttons[index];
}

void ButtonManager_::setBrightness(uint8_t val)
{
  

    for (int i = 0; i < 8; i++)
    {
        leds[i].MaxBrightness(val);
    }
    setStates();
}

void ButtonManager_::turnAllOff()
{
    for (int i = 0; i < 8; i++)
    {
        leds[i].Off();
    }
}

void ButtonManager_::checkButtons()
{
    for (int i = 0; i < 8; i++)
        getButton(i)->check();
}

uint8_t ButtonManager_::getButtonIndex(uint8_t pin)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        if (buttonPins[i] == pin)
            return i;
    }
    return -1;
}

void ButtonManager_::handleEvent(AceButton *button, uint8_t eventType, uint8_t buttonState)
{
    uint8_t btn = getButtonIndex(button->getPin());
    switch (eventType)
    {
    case AceButton::kEventClicked:
        return handleSingleClick(btn);
    case AceButton::kEventLongPressed:
        return handleLongClick(btn);
    case AceButton::kEventDoubleClicked:
        return handleDoubleClick(btn);
    case AceButton::kEventPressed:
        return handlePressed(btn);
    case AceButton::kEventReleased:
        return handleReleased(btn);
    }
}

void ButtonManager_::ShowAnimation(uint8_t type, uint8_t btn)
{
    int ledtype = SystemManager.getInt("leds");
    if (ledtype != 4)
        return;
    int count = 1;
    switch (type)
    {
    case 1:
        leds[btn].FadeOn(50).FadeOff(800).Repeat(type);

        for (uint8_t i = btn + 1; i < 8; i++)
        {
            leds[i].DelayBefore(50 * ++count).FadeOn(400).FadeOff(400).Repeat(type);
        }
        count = 1;
        for (int x = btn - 1; x >= 0; x--)
        {
            leds[x].DelayBefore(50 * ++count).FadeOn(400).FadeOff(400).Repeat(type);
        }
        break;
    case 2:
        for (uint8_t i = 0; i < 8; i++)
        {
            leds[i].Blink(100, 100).Repeat(2);
        }
        break;
    case 3:
        for (uint8_t i = 0; i < 8; i++)
        {
            leds[i].Breathe(2000).Repeat(1);
        }
        break;
    case 4:
        for (uint8_t i = 0; i < 8; i++)
        {
            leds[i].FadeOn(200).Repeat(1);
        }
        break;
    case 5:
        for (uint8_t i = 0; i < 8; i++)
        {
            leds[i].FadeOff(200).Repeat(1);
        }
        break;
    }
    prevMillis = 0;
}

boolean getPushSetting(uint8_t btn)
{
    switch (btn)
    {
    case 0:
        return SystemManager.getBool("btn1push");
        break;
    case 1:
        return SystemManager.getBool("btn2push");
        break;
    case 2:
        return SystemManager.getBool("btn3push");
        break;
    case 3:
        return SystemManager.getBool("btn4push");
        break;
    case 4:
        return SystemManager.getBool("btn5push");
        break;
    case 5:
        return SystemManager.getBool("btn6push");
        break;
    case 6:
        return SystemManager.getBool("btn7push");
        break;
    case 7:
        return SystemManager.getBool("btn8push");
        break;
    default:
        return false;
        break;
    }
}

void ButtonManager_::handleSingleClick(uint8_t btn)
{

    if (!getPushSetting(btn))
    {
        SystemManager.ShowButtonScreen(btn, "Click");
        ShowAnimation(1, btn);
        SendState(1, btn);
    }
}

void ButtonManager_::handleDoubleClick(uint8_t btn)
{
    if (!getPushSetting(btn))
    {
        SystemManager.ShowButtonScreen(btn, "Double");
        ShowAnimation(2, btn);
        SendState(2, btn);
    }
}

void ButtonManager_::handleLongClick(uint8_t btn)
{

    if (!getPushSetting(btn))
    {
        SystemManager.ShowButtonScreen(btn, "Long");
        ShowAnimation(3, btn);
        SendState(3, btn);
    }
}

void ButtonManager_::handlePressed(uint8_t btn)
{
    if (getPushSetting(btn))
    {
        SystemManager.ShowButtonScreen(btn, "Down");
        ShowAnimation(4, btn);
        SendState(4, btn);
    }
}

void ButtonManager_::handleReleased(uint8_t btn)
{
    if (getPushSetting(btn))
    {
        SystemManager.ShowButtonScreen(btn, "Up");
        ShowAnimation(5, btn);
        SendState(5, btn);
    }
}


void serialOutput(uint8_t btn, uint8_t type) {
  String Str = "SP"+String(btn) + String(type);
  Serial.println(Str);
}

void ButtonManager_::SendState(int type, uint8_t btn)
{
    serialOutput(btn,type);
    switch (type)
    {
    case 1:
        MqttManager.publish(("button" + String(btn + 1) + "/click").c_str(), "true");
        MqttManager.HAState(btn, "click");
        break;
    case 2:
        MqttManager.publish(("button" + String(btn + 1) + "/double_click").c_str(), "true");
        MqttManager.HAState(btn, "double");
        break;
    case 3:
        MqttManager.publish(("button" + String(btn + 1) + "/long_click").c_str(), "true");
        MqttManager.HAState(btn, "long");
        break;
    case 4:
        MqttManager.publish(("button" + String(btn + 1) + "/push").c_str(), "true");
        MqttManager.HAState(btn, "down");
        break;
    case 5:
        MqttManager.publish(("button" + String(btn + 1) + "/push").c_str(), "false");
        MqttManager.HAState(btn, "up");
        break;
    default:
        break;
    }

    if (type != 4 || type != 5)
    {
        delay(50);
        MqttManager.HAState(btn, "-");
        switch (type)
        {
        case 1:
            MqttManager.publish(("button" + String(btn + 1) + "/click").c_str(), "false");
            break;
        case 2:
            MqttManager.publish(("button" + String(btn + 1) + "/double_click").c_str(), "false");
            break;
        case 3:
            MqttManager.publish(("button" + String(btn + 1) + "/long_click").c_str(), "false");
            break;
        default:
            break;
        }
    }
}

bool ButtonManager_::getButtonState(uint8_t btn)
{
    return states[btn];
}

void ButtonManager_::setButtonState(uint8_t btn, uint8_t state)
{
    states[btn] = state;
    setStates();
}

void ButtonManager_::setButtonLight(uint8_t btn, uint8_t mode)
{
    switch (mode)
    {
    case 0:
        leds[btn].Off(1000);
        break;
    case 1:
        leds[btn].On(1000);
        break;
    case 2:
        leds[btn].DelayBefore(btn * 200).Breathe(2000).Forever();
        break;
    case 3:
        switch (states[btn])
        {
        case 0:
            leds[btn].Off(1000);
            break;
        case 1:
            leds[btn].On(1000);
            break;
        case 2:
            leds[btn].Breathe(1000).Forever();
            break;
        default:
            break;
        }
        break;
    default:
        leds[btn].Off(1000);
    }
}

void ButtonManager_::setStates()
{
    int ledtype = SystemManager.getInt("leds");
    if (ledtype != 4)
    {

        for (int i = 0; i <= 7; i++)
        {
            setButtonLight(i, ledtype);
        }
    }
}
