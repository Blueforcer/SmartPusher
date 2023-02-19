#ifndef SystemManager_h
#define SystemManager_h

#include <ButtonManager.h>

class SystemManager_
{
private:
    SystemManager_() = default;

public:
    const char *VERSION = "1.95";
    static SystemManager_ &getInstance();
    void setup();
    void tick();
    void show();
    void clear();
    void renderMessageScreen();
    void renderCustomScreen();
    void renderImageScreen();
    void renderButtonScreen();
    void renderClockScreen();
    void drawtext(uint8_t x, uint8_t y, String text);
    void ShowButtonScreen(uint8_t btn, const char *type);
    void setBrightness(uint8_t val);
    void BrightnessOnOff(boolean val);
    void ShowMessage(String msg);
    void ShowImage(String msg);
    const char *getValue(const char *name);
    boolean getBool(const char *name);
    int getInt(const char *name);
    String getString(const char *name);
};

extern SystemManager_ &SystemManager;

#endif