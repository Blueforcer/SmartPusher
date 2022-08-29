#ifndef SystemManager_h
#define SystemManager_h

#include <ButtonManager.h>

class SystemManager_
{
private:
    SystemManager_() = default;

public:
    static SystemManager_ &getInstance();
    void setup();
    void tick();
    void show();
    void clear();
    void renderMessageScreen();
    void renderImageScreen();
    void renderButtonScreen();
    void renderClockScreen();
    void drawtext(uint8_t x, uint8_t y, String text);
    void ShowButtonScreen(uint8_t btn, const char *type);
    void setBrightness(uint8_t val);
    void ShowMessage(String msg);
    void ShowImage(String msg);
    const char *getValue(const char *name);
    boolean getBool(const char *name);
    int getInt(const char *name);
    String getString(const char *name);
};

extern SystemManager_ &SystemManager;

#endif