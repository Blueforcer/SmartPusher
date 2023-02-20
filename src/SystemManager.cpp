#include <SystemManager.h>
#include <ESPmDNS.h>
#include "SSD1306.h"
#include <Wire.h>
#include <Update.h>
#include "font.h"
#include "images.h"
#include "SPI.h"
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <esp-fs-webserver.h>
#include <FS.h>
#include <LittleFS.h>
#include <config.h>

// Test "options" values
uint8_t ledPin = LED_BUILTIN;
bool boolVar = true;
uint32_t longVar = 1234567890;
float floatVar = 15.5F;

#define LED_LABEL "The LED pin number"
#define BOOL_LABEL "A bool variable"
#define LONG_LABEL "A long variable"
#define FLOAT_LABEL "A float varible"
#define STRING_LABEL "A String variable"

#define DISPLAY_WIDTH 128 // OLED display width, in pixels
#define DISPLAY_HEIGHT 64 // OLED display height, in pixels
const char *VERSION = "1.95";

// U8G2_SSD1306_1 028X64_NONAME_F_SW_I2C gfx(U8G2_R0, /* clock=*/SCL, /* data=*/SDA, /* reset=*/U8X8_PIN_NONE);
SSD1306 gfx(0x3c, SDA, SCL);

const int daylightOffset_sec = 3600;
tm timeinfo;

uint8_t screen = 0;
boolean connected = false;

unsigned long previousMillis = 0;
const long CLOCK_INTERVAL = 1000;
const long PICTURE_INTERVAL = 2000;
const long CHECK_WIFI_TIME = 10000;
unsigned long PREVIOUS_WIFI_CHECK = 0;
unsigned long PREVIOUS_WIFI_MILLIS = 0;
const char *Pushtype;
String MQTTMessage;
String Image;

uint8_t BtnNr;
boolean TypeShown;
boolean MessageShown;
boolean ImageShown;
String weekDay;
String fYear;
String fDate;
String fTime;
uint8_t lastBrightness = 100;

File fsUploadFile;
String temp = "";

const String weekDays[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
bool colon_switch = true;
const char *updateIndex = "<form method='POST' action='/doupdate' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";

#define FILESYSTEM LittleFS
WebServer server(80);

FSWebServer mws(FILESYSTEM, server);



////////////////////////////////  Filesystem  /////////////////////////////////////////
void startFilesystem()
{
    // FILESYSTEM INIT
    if (FILESYSTEM.begin())
    {
        File root = FILESYSTEM.open("/", "r");
        File file = root.openNextFile();
        while (file)
        {
            const char *fileName = file.name();
            size_t fileSize = file.size();
            Serial.printf("FS File: %s, size: %lu\n", fileName, (long unsigned)fileSize);
            file = root.openNextFile();
        }
        Serial.println();
    }
    else
    {
        Serial.println("ERROR on mounting filesystem. It will be formmatted!");
        FILESYSTEM.format();
        ESP.restart();
    }
}

////////////////////  Load application options from filesystem  ////////////////////
bool SystemManager_::loadOptions()
{
    if (FILESYSTEM.exists("/config.json"))
    {
        mws.getOptionValue("Broker", mqtthost);
        mws.getOptionValue("Port", mqttport);
        mws.getOptionValue("Username", mqttuser);
        mws.getOptionValue("Password", mqttpass);
        mws.getOptionValue("Prefix", mqttprefix);
        return true;
    }
    else
        Serial.println(F("File \"config.json\" not exist"));
    return false;
}

void SystemManager_::saveOptions()
{
    mws.saveOptionValue("Port", mqttport);
    mws.saveOptionValue("Username", mqttuser);
    mws.saveOptionValue("Password", mqttpass);
    mws.saveOptionValue("Prefix", mqttprefix);
    Serial.println(F("Application options saved."));
}

// The getter for the instantiated singleton instance
SystemManager_ &SystemManager_::getInstance()
{
    static SystemManager_ instance;
    return instance;
}

// Initialize the global shared instance
SystemManager_ &SystemManager = SystemManager.getInstance();

void SettingsSaved(String result)
{
    ButtonManager.setStates();
    return;
}

void update_started()
{
    Serial.println("CALLBACK:  HTTP update process started");
}

void update_finished()
{
    Serial.println("CALLBACK:  HTTP update process finished");
}

void update_progress(int cur, int total)
{
    static int last_percent = 0;
    Serial.printf("CALLBACK:  HTTP update process at %d of %d bytes...\n", cur, total);

    int percent = (100 * cur) / total;
    Serial.println(percent);

    gfx.display();
    if (percent != last_percent)
    {
        uint8_t light = percent / 12.5;
        ButtonManager.setButtonLight(light, 1);
        ButtonManager.tick();
        gfx.clear();
        gfx.drawString(DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2, String(percent) + "%");
        last_percent = percent;
        gfx.display();
    }
}

////////////////////////////  HTTP Request Handlers  ////////////////////////////////////
void handleLoadOptions()
{
    WebServerClass *webRequest = mws.getRequest();
    // loadOptions();
    Serial.println(F("Application option loaded after web request"));
    webRequest->send(200, "text/plain", "Options loaded");
}

void SystemManager_::setup()
{
    delay(2000);
    gfx.init();
    gfx.flipScreenVertically();
    gfx.clear(); // clear the internal memory
    gfx.drawXbm(0, 0, 128, 64, logo);

    gfx.display();

    delay(2000);
    gfx.clear();
    gfx.setFont(ArialMT_Plain_24);
    gfx.drawString(45, 20, "v" + String(VERSION));
    gfx.display();
    delay(800);

    // FILESYSTEM INIT
    startFilesystem();

    if (loadOptions())
        Serial.println(F("Application option loaded"));
    else
        Serial.println(F("Application options NOT loaded!"));

    // Try to connect to stored SSID, start AP if fails after timeout
    IPAddress myIP = mws.startWiFi(15000, "SmartPusher", "12345678");

    // Add custom page handlers to webserver
    mws.addHandler("/reload", HTTP_GET, handleLoadOptions);

    // Configure /setup page and start Web Server
    mws.addOptionBox("MQTT");
    mws.addOption("Broker", mqtthost);
    mws.addOption("Port", mqttport);
    mws.addOption("Username", mqttuser);
    mws.addOption("Password", mqttpass);
    mws.addOption("Prefix", mqttprefix);
    mws.addOptionBox("Buttons");
    mws.addOption("Pushmode for Button 1", btn1push);
    mws.addOption("Pushmode for Button 2", btn2push);
    mws.addOption("Pushmode for Button 3", btn3push);
    mws.addOption("Pushmode for Button 4", btn4push);
    mws.addOption("Pushmode for Button 5", btn5push);
    mws.addOption("Pushmode for Button 6", btn6push);
    mws.addOption("Pushmode for Button 7", btn7push);
    mws.addOption("Pushmode for Button 8", btn8push);
    mws.addOptionBox("NTP");
    mws.addOption("NTP Server", NTPServer);
    mws.addOption("Timezone", NTPTZ);


    if (mws.begin())
    {
        Serial.println(F("Smartpusher Web Server started on IP Address: "));
        Serial.println(myIP);
        Serial.println(F("Open /setup page to configure optional parameters"));
        Serial.println(F("Open /edit page to view and edit files"));
        Serial.println(F("Open /update page to upload firmware and filesystem updates"));
    }

    connected = !mws.inAPmode();

    if (!connected)
    {
        gfx.setFont(ArialMT_Plain_16);
        gfx.clear();
        gfx.drawString(25, 15, "AP MODE");
        gfx.drawString(20, 35, "192.168.4.1");
        gfx.display();
    }
    gfx.clear();

    Update.onProgress(update_progress);

    configTzTime(NTPTZ.c_str(), NTPServer.c_str());
    getLocalTime(&timeinfo);
}

void SystemManager_::tick()
{
    mws.run();

    if (connected)
    {
        switch (screen)
        {
        case 0:
            renderClockScreen();
            break;
        case 1:
            renderButtonScreen();
            break;
        case 2:
            renderMessageScreen();
            break;
        case 3:
            renderImageScreen();
            break;
        default:
            break;
        }
    }
    else
    {
        delay(50);
    }
}

void SystemManager_::drawtext(uint8_t x, uint8_t y, String text)
{
}

void SystemManager_::show()
{
    gfx.display();
}

void SystemManager_::clear()
{
    gfx.clear();
}

void SystemManager_::setBrightness(uint8_t val)
{
    Serial.println(val);
    gfx.setContrast(val);
    if (val == 0)
    {
        gfx.displayOff();
    }
    else
    {
        gfx.displayOn();
    };
    lastBrightness = val;
    ButtonManager.setBrightness(val);
}

void SystemManager_::BrightnessOnOff(boolean val)
{
    if (val)
    {
        gfx.displayOn();
        ButtonManager.setBrightness(lastBrightness);
    }
    else
    {
        gfx.displayOff();
        ButtonManager.setBrightness(0);
    };
}

const char *SystemManager_::getValue(const char *tag)
{
    return 0;
}

boolean SystemManager_::getBool(const char *tag)
{
    return false;
}

String SystemManager_::getString(const char *tag)
{
    return "";
}

int SystemManager_::getInt(const char *tag)
{
    return 0;
}

void SystemManager_::ShowButtonScreen(uint8_t btn, const char *type)
{
    previousMillis = millis();
    Pushtype = type;
    BtnNr = btn;
    screen = 1;
}

void SystemManager_::ShowMessage(String msg)
{
    MQTTMessage = msg;
    previousMillis = millis();
    screen = 2;
}

void SystemManager_::ShowImage(String img)
{
    Image = img;
    previousMillis = millis();
    screen = 3;
}

void SystemManager_::renderMessageScreen()
{
    static uint16_t start_at = 0;
    gfx.clear();
    gfx.setFont(ArialMT_Plain_24);
    uint16_t firstline = gfx.drawStringMaxWidth(0, 0, 128, MQTTMessage.substring(start_at));
    gfx.display();

    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= CLOCK_INTERVAL)
    {
        previousMillis = currentMillis;
        if (firstline != 0)
        {
            start_at += firstline;
        }
        else
        {
            if (MessageShown)
            {
                start_at = 0;
                screen = 0;
                MessageShown = false;
                return;
            }
            MessageShown = true;
        }
    }
}

void SystemManager_::renderButtonScreen()
{
    if (!TypeShown)
    {
        gfx.clear();
        gfx.setFont(Roboto_Black_36);
        gfx.drawString((DISPLAY_WIDTH - gfx.getStringWidth(Pushtype)) / 2, 15, Pushtype);
        gfx.display();
        TypeShown = true;
    }
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= CLOCK_INTERVAL)
    {
        previousMillis = currentMillis;
        screen = 0;
        TypeShown = false;
    }
}

void SystemManager_::renderImageScreen()
{
    gfx.clear();
    uint8_t h;
    uint8_t w;
    uint8_t xpos;
    uint8_t b;
    if (SPIFFS.exists("/" + Image + ".bin"))
    {
        File myFile = SPIFFS.open("/" + Image + ".bin", "r");
        if (myFile)
        {
            w = myFile.read(); // read the dimension of the bitmap
            h = myFile.read();
            for (size_t y = 0; y < h; y++)
            {
                xpos = 0;
                for (size_t i = 0; i < (w / 8); i++)
                {
                    {
                        b = myFile.read();
                        for (uint8_t bt = 0; bt < 8; bt++)
                        {
                            if (bitRead(b, bt))
                            { // check one pixel
                                gfx.setPixelColor(xpos, y, WHITE);
                            }
                            else
                            {
                                gfx.setPixelColor(xpos, y, BLACK);
                            }
                            xpos++;
                        }
                    }
                }
            }

            myFile.close(); // all done, close the file
            gfx.display();
        }
    }
    else
    {
        gfx.setFont(ArialMT_Plain_16);
        gfx.drawString(14, 25, "NOT FOUND!");
        gfx.display();
    }

    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= PICTURE_INTERVAL)
    {
        previousMillis = currentMillis;
        screen = 0;
        ImageShown = false;
    }
}

void SystemManager_::renderCustomScreen()
{
}

void SystemManager_::renderClockScreen()
{
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= CLOCK_INTERVAL)
    {
        // getLocalTime(&timeinfo);
        gfx.clear();

        previousMillis = currentMillis;
        weekDay = weekDays[timeinfo.tm_wday];

        if (true) // conf.getBool("colonblink")
        {
            colon_switch = !colon_switch;
        }
        else
        {
            colon_switch = true;
        }

        fYear = String(1900 + timeinfo.tm_year);
        fDate = (timeinfo.tm_mday < 10 ? "0" : "") + String(timeinfo.tm_mday) + "/" + (timeinfo.tm_mon + 1 < 10 ? "0" : "") + String(timeinfo.tm_mon + 1);
        fTime = (timeinfo.tm_hour < 10 ? "0" : "") + String(timeinfo.tm_hour) + (colon_switch ? ":" : " ") + (timeinfo.tm_min < 10 ? "0" : "") + String(timeinfo.tm_min);
        gfx.setFont(ArialMT_Plain_16);
        gfx.drawString(0, 0, fDate);
        gfx.setFont(ArialMT_Plain_16);
        gfx.drawString(95, 0, weekDay);
        gfx.setFont(DSEG14_Modern_Mini_Regular_30);
        gfx.drawString((DISPLAY_WIDTH - gfx.getStringWidth(fTime)) / 2, 25, fTime);
        gfx.display();
    }
}
