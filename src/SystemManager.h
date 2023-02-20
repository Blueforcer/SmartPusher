#ifndef SystemManager_h
#define SystemManager_h

#include <ButtonManager.h>

class SystemManager_
{
private:
    SystemManager_() = default;

public:
    String mqtthost;
    uint16_t mqttport = 1883;
    String mqttuser;
    String mqttpass;
    String mqttprefix = "Smartpusher";
    boolean btn1push = false;
    boolean btn2push = false;
    boolean btn3push = false;
    boolean btn4push = false;
    boolean btn5push = false;
    boolean btn6push = false;
    boolean btn7push = false;
    boolean btn8push = false;
    String NTPServer = "de.pool.ntp.org";
    String NTPTZ = "CET-1CEST,M3.5.0,M10.5.0/3";
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
    bool loadOptions();
    void saveOptions();
    int getInt(const char *name);
    String getString(const char *name);
};

extern SystemManager_ &SystemManager;

#endif