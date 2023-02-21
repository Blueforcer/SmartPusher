#ifndef ButtonManager_h
#define ButtonManager_h

#include <AceButton.h>
#include <Arduino.h>
#include <vector>
#include <jled.h>
#include <MqttManager.h>

using namespace ace_button;
using namespace std;

class ButtonManager_
{
private:
    // The pins the buttons are connected to
    const uint8_t buttonPins[8] = {32, 25, 27, 12, 19, 5, 16, 0};

    // Different states and modes of the buttons
    uint8_t states[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    bool longPressed[8] = {false, false, false, false, false, false, false, false};
    bool modes[8] = {0, 0, 0, 0, 0, 0, 0, 0};

    // The button references themselves
    vector<AceButton *> buttons;

    void SendState(int type, uint8_t btn);
    uint8_t getButtonIndex(uint8_t);

    // This method is the class-internal event handler with access to the instance state
    void handleEvent(AceButton *, uint8_t, uint8_t);
    void handleSingleClick(uint8_t);
    void handleDoubleClick(uint8_t);
    void handleLongClick(uint8_t);
    void handlePressed(uint8_t);
    void handleReleased(uint8_t);

    // This references the global/static event handler and marks it as a friend of this class,
    // which allows it to call the class-internal event handler, even though it is marked as private
    friend void staticHandleEvent(AceButton *, uint8_t, uint8_t);

    ButtonManager_() = default;

public:
    static ButtonManager_ &getInstance();

    JLed leds[8] = {
        JLed(33).Off().LowActive(),
        JLed(26).Off().LowActive(),
        JLed(14).Off().LowActive(),
        JLed(13).Off().LowActive(),
        JLed(23).Off().LowActive(),
        JLed(18).Off().LowActive(),
        JLed(17).Off().LowActive(),
        JLed(4).Off().LowActive()};


    void setup();
    void tick();
    void setStates();
    void checkButtons();
    void LeaveMenuState();
    void turnAllOff();
    void blinkAll();
    void setBrightness(uint8_t btn);
    void ShowAnimation(uint8_t type, uint8_t btn);
    bool getButtonState(uint8_t);
    void setButtonState(uint8_t, uint8_t);
    void setButtonLight(uint8_t btn, uint8_t mode);

    AceButton *getButton(uint8_t);
};

extern ButtonManager_ &ButtonManager;

#endif