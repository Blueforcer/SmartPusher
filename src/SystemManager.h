#ifndef SystemManager_h
#define SystemManager_h

#include <ButtonManager.h>

#include "TCMenu_menu.h"

class SystemManager_
{
private:
    SystemManager_() = default;

public:
    static SystemManager_ &getInstance();

    void setup();
    void tick();
    void EnterMenu();
    void ShowTitleScreen();
    void ShowButtonScreen(const char *type);
    void setBrightness(uint8_t val);
};

extern SystemManager_ &SystemManager;

#endif