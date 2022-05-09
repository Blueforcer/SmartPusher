#include <SystemManager.h>
#include <EEPROM.h>
#include <Wire.h>
#include <SPI.h>
#include <WiFi.h>
#include "stockIcons/wifiAndConnectionIcons16x12.h"
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <config.h>

unsigned long previousMillis = 0;
const long interval = 1000;
const char *Pushtype;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
const unsigned long updateDelay = 900000; // update time every 15 min
const unsigned long retryDelay = 5000;    // retry 5 sec later if time query failed
const String weekDays[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
unsigned long lastUpdatedTime = updateDelay * -1;
unsigned int second_prev = 0;
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
    WiFi.begin(SSID, password);
    WiFi.mode(WIFI_STA);
    gfx.setFont(u8g2_font_pcsenior_8f);
    gfx.drawStr(0, 10, "Connecting");
    gfx.drawStr(0, 20, "  to WiFi...");
    gfx.sendBuffer();
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(250);
        Serial.print(".");
    }
    Serial.println("Connected to the WiFi network");
    Serial.println(WiFi.localIP());
    gfx.clearBuffer();
}

unsigned int getYear()
{
    time_t rawtime = timeClient.getEpochTime();
    struct tm *ti;
    ti = localtime(&rawtime);
    unsigned int year = ti->tm_year + 1900;
    return year;
}

unsigned int getMonth()
{
    time_t rawtime = timeClient.getEpochTime();
    struct tm *ti;
    ti = localtime(&rawtime);
    unsigned int month = ti->tm_mon + 1;
    return month;
}

unsigned int getDate()
{
    time_t rawtime = timeClient.getEpochTime();
    struct tm *ti;
    ti = localtime(&rawtime);
    unsigned int month = ti->tm_mday;
    return month;
}

void renderTitleScreen(unsigned int encoderValue, RenderPressMode clicked)
{
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval)
    {
        // save the last time you blinked the LED
        gfx.clearBuffer();
        previousMillis = currentMillis;

        unsigned int year = getYear();
        unsigned int month = getMonth();
        unsigned int day = getDate();
        unsigned int hour = timeClient.getHours();
        unsigned int minute = timeClient.getMinutes();
        unsigned int second = timeClient.getSeconds();
        String weekDay = weekDays[timeClient.getDay()];

        if (second != second_prev && Colonblink)
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

    if (currentMillis - previousMillis >= interval)
    {
        previousMillis = currentMillis;
        renderer.takeOverDisplay(renderTitleScreen);
    }
}

void CALLBACK_FUNCTION onTakeOverDisplay(int /*id*/)
{
    renderer.takeOverDisplay(renderTitleScreen);
}

void SystemManager_::ShowTitleScreen()
{
    renderer.takeOverDisplay(renderTitleScreen);
}

void SystemManager_::ShowButtonScreen(const char *type)
{
    Pushtype = type;
    renderer.takeOverDisplay(renderButtonScreen);
}

void SystemManager_::EnterMenu()
{
    gfx.clearBuffer();
    gfx.setFont(u8g2_font_tenfatguys_tr);
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
                               renderer.takeOverDisplay(renderTitleScreen); });
    setupMenu();
    gfx.clearBuffer();
    gfx.setFont(u8g2_font_tenfatguys_tr);
    gfx.drawStr(3,35,"SmartPusher");
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
        
    timeClient.begin();
    timeClient.setTimeOffset(UTCoffset * 3600);
    timeClient.update();
    renderer.takeOverDisplay(renderTitleScreen);
}

void SystemManager_::tick()
{
    taskManager.runLoop();
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
    // on esp you must commit after calling save.
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
    renderer.takeOverDisplay(renderTitleScreen);
}

void CALLBACK_FUNCTION Light(int id)
{
    ButtonManager.setStates();
}