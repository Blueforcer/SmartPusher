#include <ButtonManager.h>
#include <SystemManager.h>
#include <ArduinoJson.h>
#include "SwitchInput.h"

BooleanMenuItem *ClickItems[]{&menubtn1Click, &menubtn2Click, &menubtn3Click, &menubtn4Click, &menubtn5Click, &menubtn6Click, &menubtn7Click, &menubtn8Click};
BooleanMenuItem *LongClickItems[]{&menubtn1LongClick, &menubtn2LongClick, &menubtn3LongClick, &menubtn4LongClick, &menubtn5LongClick, &menubtn6LongClick, &menubtn7LongClick, &menubtn8LongClick};
BooleanMenuItem *DoubleClickItems[]{&menubtn1DoubleClick, &menubtn2DoubleClick, &menubtn3DoubleClick, &menubtn4DoubleClick, &menubtn5DoubleClick, &menubtn6DoubleClick, &menubtn7DoubleClick, &menubtn8DoubleClick};
BooleanMenuItem *DownItems[]{&menubtn1Down, &menubtn2Down, &menubtn3Down, &menubtn4Down, &menubtn5Down, &menubtn6Down, &menubtn7Down, &menubtn8Down};
BooleanMenuItem *UpItems[]{&menubtn1Up, &menubtn2Up, &menubtn3Up, &menubtn4Up, &menubtn5Up, &menubtn6Up, &menubtn7Up, &menubtn8Up};

bool MenuEntered = false;
bool ResetLights = false;
unsigned long prevMillis = 0;

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
    buttonConfig->setDoubleClickDelay(300);
    setStates();
}

void ButtonManager_::LeaveMenuState()
{
    Serial.println("Leave Menu");
    MenuEntered = false;
    setStates();
}

void ButtonManager_::tick()
{
    for (int i = 0; i < 8; i++)
    {
        leds[i].Update();
    }
    CheckMenu();

    // Check if the menu buttons are being pressed. If so, show menu
    checkButtons();

    if (ResetLights)
    {
        unsigned long currentMillis = millis();
        for (uint8_t i = 0; i < 8; i++)
        {
            if (leds[i].IsRunning())
                return;
        }

        if (currentMillis - prevMillis >= 1000)
        {
            prevMillis = currentMillis;
            setStates();
            ResetLights = false;
        }
    }
}

void ButtonManager_::CheckMenu()
{
    if (longPressed[0] && longPressed[1])
    {
        for (int i = 0; i < 8; i++)
        {
            longPressed[i] = false;
            leds[i].Breathe(2000).Forever();
        }
        if (MenuEntered)
        {
            SystemManager.ShowTitleScreen();
            setStates();
            MenuEntered = false;
            return;
        }
        else
        {
            MenuEntered = true;
            SystemManager.EnterMenu();
            Serial.println("Menu Entered");
            return;
        }
    }
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
    ResetLights = true;
    prevMillis = millis();
}

void ButtonManager_::handleSingleClick(uint8_t btn)
{
    if (ClickItems[btn]->getBoolean() && !MenuEntered)
    {
        SystemManager.ShowButtonScreen("Click");

        ShowAnimation(1, btn);
        SendState(1, btn);
    }
}

void ButtonManager_::handleDoubleClick(uint8_t btn)
{
    if (DoubleClickItems[btn]->getBoolean() && !MenuEntered)
    {
        SystemManager.ShowButtonScreen("Double");

        ShowAnimation(2, btn);
        SendState(2, btn);
    }
}

void ButtonManager_::handleLongClick(uint8_t btn)
{

    longPressed[btn] = true;
    if (!MenuEntered)
        CheckMenu();
    if (LongClickItems[btn]->getBoolean() && !MenuEntered)
    {
        SystemManager.ShowButtonScreen("Long");
        ShowAnimation(3, btn);
        SendState(3, btn);
    }
}

void ButtonManager_::handlePressed(uint8_t btn)
{
    if (DownItems[btn]->getBoolean() && !MenuEntered)
    {
        SystemManager.ShowButtonScreen("Down");
        ShowAnimation(4, btn);
        SendState(4, btn);
    }
}

void ButtonManager_::handleReleased(uint8_t btn)
{
    longPressed[btn] = false;
    if (UpItems[btn]->getBoolean() && !MenuEntered)
    {
        SystemManager.ShowButtonScreen("Up");
        ShowAnimation(5, btn);
        SendState(5, btn);
    }
}

void ButtonManager_::SendState(int type, uint8_t btn)
{
    String json;
    StaticJsonDocument<200> doc;
    switch (type)
    {
    case 1:
        doc["click"] = true;
        break;
    case 2:
        doc["double_click"] = true;
        break;
    case 3:
        doc["long_click"] = true;
        break;
    case 4:
        doc["push"] = true;
        break;
    case 5:
        doc["push"] = false;
        break;
    default:
        break;
    }

    serializeJsonPretty(doc, json);
    MqttManager.publish(("Button" + String(btn + 1)).c_str(), json.c_str());

    if (type != 4 || type != 5)
    {
        json = "";
        delay(500);
        switch (type)
        {
        case 1:
            doc["click"] = false;
            break;
        case 2:
            doc["double_click"] = false;
            break;
        case 3:
            doc["long_click"] = false;
            break;
        default:
            break;
        }

        serializeJsonPretty(doc, json);
        MqttManager.publish(("Button" + String(btn + 1)).c_str(), json.c_str());
    }
}

bool ButtonManager_::getButtonState(uint8_t btn)
{
    return states[btn];
}

void ButtonManager_::setButtonState(uint8_t btn, bool state)
{
    states[btn] = state;
    setStates();
}

void ButtonManager_::setButtonLight(uint8_t btn, uint8_t mode)
{
    switch (mode)
    {
    case 0:
        leds[btn].Off();
        break;
    case 1:
        leds[btn].On();
        break;
    case 2:
        leds[btn].Breathe(2000).Forever();
        break;
    case 3:
        leds[btn].Off();
        break;
    default:
        break;
    }
}

void ButtonManager_::setStates()
{
    setButtonLight(0, menuLightBtn1.getCurrentValue());
    setButtonLight(1, menuLightBtn2.getCurrentValue());
    setButtonLight(2, menuLightBtn3.getCurrentValue());
    setButtonLight(3, menuLightBtn4.getCurrentValue());
    setButtonLight(4, menuLightBtn5.getCurrentValue());
    setButtonLight(5, menuLightBtn6.getCurrentValue());
    setButtonLight(6, menuLightBtn7.getCurrentValue());
    setButtonLight(7, menuLightBtn8.getCurrentValue());
}
