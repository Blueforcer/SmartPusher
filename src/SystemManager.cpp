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

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

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
    // scanNetworks();
    Serial.println("Connecting to Wifi ");
    WiFi.begin(SSID, password);
    WiFi.mode(WIFI_STA);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(250);
        Serial.print(".");
    }
     Serial.println("Connected to the WiFi network");
     Serial.println(WiFi.localIP());
}

void renderTitleScreen(unsigned int encoderValue, RenderPressMode clicked)
{
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval)
    {
        // save the last time you blinked the LED
        gfx.clearBuffer();
        previousMillis = currentMillis;
        timeClient.setTimeOffset(menuUTCOffset.getCurrentValue() * 3600);
        timeClient.update();
        gfx.setFont(u8g2_font_timB24_tr);
        gfx.drawStr(8, 48, timeClient.getFormattedTime().c_str());

        if (WiFi.status() != WL_CONNECTED)
        {
            gfx.setFont(u8g2_font_5x8_tf);
            gfx.drawStr(22, 62, "please check WiFi");
        }
        gfx.sendBuffer();
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

void SystemManager_::EnterMenu()
{
    gfx.clearBuffer();
    gfx.setFont(u8g2_font_timB24_tr);
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
    gfx.setFont(u8g2_font_timB24_tr);
    gfx.drawStr(20, 40, "APBA");
    gfx.setFont(u8g2_font_5x8_tf);
    gfx.drawStr(5, 55, "all purpose button array");
    gfx.sendBuffer();
    menuMgr.load(*eeprom);
    startWiFi();
    // menuIoTMonitor.registerCommsNotification(onCommsChange);
    delay(1000);
    taskManager.scheduleFixedRate(1000, []
                                  {
        if(WiFi.status() == WL_CONNECTED) {
            wifiWidget.setCurrentState(fromWiFiRSSITo4StateIndicator(WiFi.RSSI()));
        }
        else {
            wifiWidget.setCurrentState(0);
        } });
    timeClient.begin();
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