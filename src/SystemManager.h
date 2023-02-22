#ifndef SystemManager_h
#define SystemManager_h

#include <ButtonManager.h>

class SystemManager_
{
private:
    SystemManager_() = default;

public:
    const char *VERSION = "2.11";
    String MQTT_HOST;
    uint16_t MQTT_PORT = 1883;
    String MQTT_USER;
    String MQTT_PASS;
    String MQTT_PREFIX = "Smartpusher";
    String CITY = "Berlin,de";
    bool IS_METRIC = true;
    bool BTN1_PUSH = false;
    bool BTN2_PUSH = false;
    bool BTN3_PUSH = false;
    bool BTN4_PUSH = false;
    bool BTN5_PUSH = false;
    bool BTN6_PUSH = false;
    bool BTN7_PUSH = false;
    bool BTN8_PUSH = false;
    bool RGB_BUTTONS = false;
    bool CUSTOM_PAGES = false;
    bool SERIAL_OUT = false;
    bool HA_DISCOVERY = false;
    bool PAGE_BUTTONS = false;
    String ledMode = "On";
    int TIME_PER_FRAME = 5000;
    int TIME_PER_TRANSITION = 700;
    String NTP_SERVER = "de.pool.ntp.org";
    String NTP_TZ = "CET-1CEST,M3.5.0,M10.5.0/3";
    static SystemManager_ &getInstance();
    void setup();
    void tick();
    void show();
    void clear();
    void renderMessagePage();
    void renderCustomPage();
    void renderImagePage();
    void renderButtonPage();
    void renderClockPage();
    void nextPage();
    void previousPage();
    void renderWeatherPage();
    void drawtext(uint8_t x, uint8_t y, String text);
    void ShowButtonPage(uint8_t btn, const char *type);
    void setCustomPageVariables(String PageName, String variableName, String Value);
    void setBrightness(uint8_t val);
    void BrightnessOnOff(boolean val);
    void ShowMessage(String msg);
    void ShowImage(String msg);
    const char *getValue(const char *name);
    boolean getBool(const char *name);
    bool loadOptions();
    void saveOptions();
    
};

extern SystemManager_ &SystemManager;

#endif