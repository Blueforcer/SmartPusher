#include <SystemManager.h>
#include <EEPROM.h>
#include <Wire.h>
#include <SPI.h>
#include <WiFi.h>
#include "stockIcons/wifiAndConnectionIcons16x12.h"
#include <config.h>

const int daylightOffset_sec = 3600;
tm timeinfo;

unsigned long previousMillis = 0;
const long CLOCK_INTERVAL = 1000;
const long CHECK_WIFI_TIME = 10000;
unsigned long PREVIOUS_WIFI_MILLIS = 0;
const char *Pushtype;

const String weekDays[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
bool colon_switch = true;

TitleWidget wifiWidget(iconsWifi, 5, 16, 12);

ArduinoEEPROMAbstraction *eeprom = NULL;

// The getter for the instantiated singleton instance
SystemManager_ &SystemManager_::getInstance()
{
    static SystemManager_ instance;
    return instance;
}

// Initialize the global shared instance
SystemManager_ &SystemManager = SystemManager.getInstance();

void startWiFi()
{
    gfx.clearBuffer();
    Serial.println("Connecting to Wifi ");
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    WiFi.mode(WIFI_STA);
    gfx.setFont(u8g2_font_tenfatguys_tr);
    gfx.drawStr(10, 10, "Connecting");
    gfx.drawStr(10, 30, "to WiFi...");
    gfx.sendBuffer();
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(250);
        Serial.print(".");
    }
    Serial.println();
    Serial.println("Connected to WiFi");
    Serial.println(WiFi.localIP());
    gfx.clearBuffer();
    gfx.drawStr(15, 30, "Connected!");
    gfx.sendBuffer();
    delay(1000);
}

void renderTimeScreen(unsigned int encoderValue, RenderPressMode clicked)
{

    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= CLOCK_INTERVAL)
    {
        // save the last time you blinked the LED
        gfx.clearBuffer();
        previousMillis = currentMillis;

        if (!getLocalTime(&timeinfo))
        {
            Serial.println("Failed to obtain time");
            return;
        }

        unsigned int year = 1900 + timeinfo.tm_year;
        unsigned int month = timeinfo.tm_mon + 1;
        unsigned int day = timeinfo.tm_mday;
        unsigned int hour = timeinfo.tm_hour;
        unsigned int minute = timeinfo.tm_min;
        String weekDay = weekDays[timeinfo.tm_wday];

        if (COLON_BLINK)
            colon_switch = !colon_switch;

        String fYear = String(year);
        String fDate = (day < 10 ? "0" : "") + String(day) + "/" + (month < 10 ? "0" : "") + String(month);
        String fTime = (hour < 10 ? "0" : "") + String(hour) + (colon_switch ? ":" : " ") + (minute < 10 ? "0" : "") + String(minute);

        gfx.setFont(u8g2_font_inr16_mf);
        gfx.drawStr(0, 16, strcpy(new char[fDate.length() + 1], fDate.c_str()));
        gfx.setFont(u8g2_font_pxplusibmcgathin_8f);
        gfx.drawStr(93, 8, strcpy(new char[fYear.length() + 1], fYear.c_str()));
        gfx.setFont(u8g2_font_pxplusibmcgathin_8f);
        gfx.drawStr(93, 17, strcpy(new char[weekDay.length() + 1], weekDay.c_str()));
        gfx.setFont(u8g2_font_inb30_mn);
        gfx.drawStr(2, 58, strcpy(new char[fTime.length() + 1], fTime.c_str()));

        gfx.sendBuffer();
    }
}

void renderButtonScreen(unsigned int encoderValue, RenderPressMode clicked)
{
    unsigned long currentMillis = millis();
    gfx.clearBuffer();

    gfx.setFont(u8g2_font_inb24_mr);
    gfx.drawStr((gfx.getDisplayWidth() - gfx.getUTF8Width(Pushtype)) / 2, 45, Pushtype);

    gfx.sendBuffer();

    if (currentMillis - previousMillis >= CLOCK_INTERVAL)
    {
        previousMillis = currentMillis;
        gfx.setColorIndex(1);
        renderer.takeOverDisplay(renderTimeScreen);
    }
}

void CALLBACK_FUNCTION onTakeOverDisplay(int /*id*/)
{
    gfx.setColorIndex(1);
    renderer.takeOverDisplay(renderTimeScreen);
}

void SystemManager_::ShowTitleScreen()
{
    gfx.setColorIndex(1);
    renderer.takeOverDisplay(renderTimeScreen);
}

void SystemManager_::ShowButtonScreen(const char *type)
{
    Pushtype = type;
    renderer.takeOverDisplay(renderButtonScreen);
}

void SystemManager_::EnterMenu()
{
    gfx.clearBuffer();
    gfx.setFont(u8g2_font_tenfatguys_tn);
    gfx.drawStr(20, 45, "MENU");
    gfx.sendBuffer();
    delay(1000);
    menuMgr.navigateToMenu(menuMgr.getRoot());
    renderer.giveBackDisplay();
}

void SystemManager_::setup()
{
    EEPROM.begin(512);
    eeprom = new ArduinoEEPROMAbstraction(&EEPROM);
    Wire.begin();
    Wire.beginTransmission(0x3C);
    renderer.setFirstWidget(&wifiWidget);
    renderer.setResetCallback([]
                              {ButtonManager.LeaveMenuState();
                                gfx.setColorIndex(1);
                               renderer.takeOverDisplay(renderTimeScreen); });
    setupMenu();
    gfx.clearBuffer();
    gfx.setFont(u8g2_font_tenfatguys_tr);
    gfx.drawStr(3, 35, "SmartPusher");
    gfx.sendBuffer();
    menuMgr.load(*eeprom);

    delay(2000);

    startWiFi();

    taskManager.scheduleFixedRate(1000, []
                                  {
        if(WiFi.status() == WL_CONNECTED) {
            wifiWidget.setCurrentState(fromWiFiRSSITo4StateIndicator(WiFi.RSSI()));
        }
        else {
            wifiWidget.setCurrentState(0);
        } });

    taskManager.scheduleFixedRate(10000, []
                                  {
        if ((WiFi.status() != WL_CONNECTED))
    {
        Serial.println("Reconnecting to WiFi...");
        WiFi.disconnect();
        WiFi.reconnect();
    } });

    configTzTime(TIMEZONE, NTP_SERVER);
    gfx.setColorIndex(1);
    renderer.takeOverDisplay(renderTimeScreen);
}

void SystemManager_::tick()
{
    taskManager.runLoop();
}

void SystemManager_::setBrightness(uint8_t val)
{
    gfx.setContrast(val);
    gfx.setPowerSave(val == 0);
    ButtonManager.setBrightness(val);
}

void onClearFinished(ButtonType btnPressed, void * /*userdata*/)
{
    if (btnPressed != BTNTYPE_OK)
    {
        for (int i = 0; i < 513; i++)
        {
            EEPROM.write(i, 0);
        }
        EEPROM.commit();
        gfx.clearBuffer();
        gfx.setFont(u8g2_font_7x13B_mf);
        gfx.drawStr(8, 45, "CLEARED");
        gfx.sendBuffer();
        delay(1000);
        ESP.restart();
    }
}

const char clearSettingsPgm[] PROGMEM = "Clear settings?";
void CALLBACK_FUNCTION onClearSettings(int id)
{
    BaseDialog *dlg = renderer.getDialog();
    if (dlg)
    {
        dlg->setButtons(BTNTYPE_ACCEPT, BTNTYPE_CANCEL);
        dlg->show(clearSettingsPgm, false, onClearFinished);
    }
}

void CALLBACK_FUNCTION onSaveAll(int id)
{
    Serial.println("Saving values to EEPROM");
    menuMgr.save();
    EEPROM.commit();
    BaseDialog *dlg = renderer.getDialog();
    if (dlg)
    {
        dlg->setButtons(BTNTYPE_NONE, BTNTYPE_CLOSE);
        dlg->show("Saved", false);
    }
}

void CALLBACK_FUNCTION onExit(int id)
{
    ButtonManager.LeaveMenuState();
    renderer.takeOverDisplay(renderTimeScreen);
}

void CALLBACK_FUNCTION Light(int id)
{
    ButtonManager.setStates();
}