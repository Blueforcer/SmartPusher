#include <ButtonManager.h>
#include <SystemManager.h>
#include <ArduinoJson.h>
#include "SwitchInput.h"

#define Btn_Up 50
#define Btn_Down 51
#define Btn_Left 52
#define Btn_Right 53
#define Btn_Ok 54

BooleanMenuItem *ClickItems[]{&menubtn1Click, &menubtn2Click, &menubtn3Click, &menubtn4Click, &menubtn5Click, &menubtn6Click, &menubtn7Click, &menubtn8Click};
BooleanMenuItem *LongClickItems[]{&menubtn1LongClick, &menubtn2LongClick, &menubtn3LongClick, &menubtn4LongClick, &menubtn5LongClick, &menubtn6LongClick, &menubtn7LongClick, &menubtn8LongClick};
BooleanMenuItem *DoubleClickItems[]{&menubtn1DoubleClick, &menubtn2DoubleClick, &menubtn3DoubleClick, &menubtn4DoubleClick, &menubtn5DoubleClick, &menubtn6DoubleClick, &menubtn7DoubleClick, &menubtn8DoubleClick};
BooleanMenuItem *DownItems[]{&menubtn1Down, &menubtn2Down, &menubtn3Down, &menubtn4Down, &menubtn5Down, &menubtn6Down, &menubtn7Down, &menubtn8Down};
BooleanMenuItem *UpItems[]{&menubtn1Up, &menubtn2Up, &menubtn3Up, &menubtn4Up, &menubtn5Up, &menubtn6Up, &menubtn7Up, &menubtn8Up};

bool MenuEntered = false;

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

    // Check if the menu buttons are being pressed. If so, show menu
    if (longPressed[0] && longPressed[7])
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
    checkButtons();
}

AceButton *ButtonManager_::getButton(uint8_t index)
{
    return buttons[index];
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

void ButtonManager_::handleSingleClick(uint8_t btn)
{
    if (ClickItems[btn]->getBoolean() && !MenuEntered)
    {
        SystemManager.ShowButtonScreen("Click");
        sendState("click", btn);
    }
}

void ButtonManager_::handleDoubleClick(uint8_t btn)
{
    if (DoubleClickItems[btn]->getBoolean() && !MenuEntered)
    {
        SystemManager.ShowButtonScreen("Double");
        sendState("double", btn);
    }
}

void ButtonManager_::handleLongClick(uint8_t btn)
{

    longPressed[btn] = true;

    if (LongClickItems[btn]->getBoolean() && !MenuEntered)
    {
        SystemManager.ShowButtonScreen("Long");
        sendState("long", btn);
    }
}

void ButtonManager_::handlePressed(uint8_t btn)
{
    if (DownItems[btn]->getBoolean() && !MenuEntered)
    {
        sendState("down", btn);
    }
}

void ButtonManager_::handleReleased(uint8_t btn)
{
    longPressed[btn] = false;
    if (UpItems[btn]->getBoolean() && !MenuEntered)
    {
        sendState("up", btn);
    }
}

void ButtonManager_::sendState(String type, int btn)
{
    String json;
    StaticJsonDocument<200> doc;
    doc["type"] = type;
    doc["button"] = btn;
    serializeJsonPretty(doc, json);
    MqttManager.publish("Button" + btn, json.c_str());
    // uint32_t length = json.length();
    // Serial.printf("%c%c%c%c%s", (length & 0xFF000000) >> 24, (length & 0x00FF0000) >> 16, (length & 0x0000FF00) >> 8, (length & 0x000000FF), json.c_str());
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