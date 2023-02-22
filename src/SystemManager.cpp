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
#include <HTTPClient.h>
#include "OLEDDisplayUi.h"
#include "MqttManager.h"

SSD1306 display(0x3c, SDA, SCL);
OLEDDisplayUi ui(&display);

// This array keeps function pointers to all frames
// frames are the single views that slide in

#define DISPLAY_WIDTH 128 // OLED display width, in pixels
#define DISPLAY_HEIGHT 64 // OLED display height, in pixels

const char *VERSION = "2.1";

time_t now;
tm timeInfo;

uint8_t InternalScreen = 0;
boolean connected = false;

StaticJsonDocument<1024> pages;

unsigned long previousMillis = 0;
const long CLOCK_INTERVAL = 1000;
const long PICTURE_INTERVAL = 2000;
const long CHECK_WIFI_TIME = 10000;
bool readyForWeatherUpdate = false;
long timeSinceLastWUpdate = 0;
unsigned long PREVIOUS_WIFI_CHECK = 0;
unsigned long PREVIOUS_WEATHER_CHECK = 0;
unsigned long PREVIOUS_WIFI_MILLIS = 0;
const int UPDATE_INTERVAL_SECS = 20 * 60; // Update every 20 minutes
const char *Pushtype;

String MY_CITY;
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

void updateData(OLEDDisplay *display);
bool FIRST_UPDATE;
const String weekDays[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

#define FILESYSTEM LittleFS
WebServer server(80);
FSWebServer mws(FILESYSTEM, server);

#define LIST_SIZE 5
const char *dropdownList[LIST_SIZE] = {
    "Off", "On", "Fade", "Extern", "OnPush"};

// Create a buffer for the JSON data
const size_t capacity = JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + 100;
DynamicJsonDocument doc(3072);
String cur_temp, cur_condition, cur_icon;

// The getter for the instantiated singleton instance
SystemManager_ &SystemManager_::getInstance()
{
    static SystemManager_ instance;
    return instance;
}

// Initialize the global shared instance
SystemManager_ &SystemManager = SystemManager.getInstance();

////////////////////////////////  Filesystem  /////////////////////////////////////////
void startFilesystem()
{
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

void displaywidget(String code_no)
{
}
////////////////////  Load&Save application options from filesystem  ////////////////////
bool SystemManager_::loadOptions()
{
    if (FILESYSTEM.exists("/config.json"))
    {
        mws.getOptionValue("Use RGB buttons", RGB_BUTTONS);
        mws.getOptionValue("Use customized pages", CUSTOM_PAGES);
        mws.getOptionValue("Pushmode for Button 1", BTN1_PUSH);
        mws.getOptionValue("Pushmode for Button 2", BTN2_PUSH);
        mws.getOptionValue("Pushmode for Button 3", BTN3_PUSH);
        mws.getOptionValue("Pushmode for Button 4", BTN4_PUSH);
        mws.getOptionValue("Pushmode for Button 5", BTN5_PUSH);
        mws.getOptionValue("Pushmode for Button 6", BTN6_PUSH);
        mws.getOptionValue("Pushmode for Button 7", BTN7_PUSH);
        mws.getOptionValue("Pushmode for Button 8", BTN8_PUSH);
        mws.getOptionValue("NTP Server", NTP_SERVER);
        mws.getOptionValue("Timezone", NTP_TZ);
        mws.getOptionValue("Broker", MQTT_HOST);
        mws.getOptionValue("Port", MQTT_PORT);
        mws.getOptionValue("Username", MQTT_USER);
        mws.getOptionValue("Password", MQTT_PASS);
        mws.getOptionValue("Prefix", MQTT_PREFIX);
        mws.getOptionValue("Actions over serial", SERIAL_OUT);
        mws.getOptionValue("LED Mode", ledMode);
        mws.getOptionValue("City", CITY);
        mws.getOptionValue("Homeassistant Discovery", HA_DISCOVERY);
        mws.getOptionValue("Duration per Page", TIME_PER_FRAME);
        mws.getOptionValue("Transistion duration", TIME_PER_TRANSITION);
        mws.getOptionValue("Control with Button 7&8", PAGE_BUTTONS);
        return true;
    }
    else
        Serial.println(F("File \"config.json\" not exist"));
    return false;
}

void SystemManager_::saveOptions()
{
    mws.saveOptionValue("Homeassistant Discovery", HA_DISCOVERY);
    mws.saveOptionValue("Use RGB buttons", RGB_BUTTONS);
    mws.saveOptionValue("Use customized pages", CUSTOM_PAGES);
    mws.saveOptionValue("Pushmode for Button 1", BTN1_PUSH);
    mws.saveOptionValue("Pushmode for Button 2", BTN2_PUSH);
    mws.saveOptionValue("Pushmode for Button 3", BTN3_PUSH);
    mws.saveOptionValue("Pushmode for Button 4", BTN4_PUSH);
    mws.saveOptionValue("Pushmode for Button 5", BTN5_PUSH);
    mws.saveOptionValue("Pushmode for Button 6", BTN6_PUSH);
    mws.saveOptionValue("Pushmode for Button 7", BTN7_PUSH);
    mws.saveOptionValue("Pushmode for Button 8", BTN8_PUSH);
    mws.saveOptionValue("NTP Server", NTP_SERVER);
    mws.saveOptionValue("Timezone", NTP_TZ);
    mws.saveOptionValue("Broker", MQTT_HOST);
    mws.saveOptionValue("Port", MQTT_PORT);
    mws.saveOptionValue("Username", MQTT_USER);
    mws.saveOptionValue("Password", MQTT_PASS);
    mws.saveOptionValue("Prefix", MQTT_PREFIX);
    mws.saveOptionValue("Actions over serial", SERIAL_OUT);
    mws.saveOptionValue("LED Mode", ledMode);
    mws.saveOptionValue("City", CITY);
    mws.saveOptionValue("Duration per Page", TIME_PER_FRAME);
    mws.saveOptionValue("Transistion duration", TIME_PER_TRANSITION);
    mws.saveOptionValue("Control with Button 7&8", PAGE_BUTTONS);
    Serial.println(F("Application options saved."));
}

// function to get the name of the current screen
String getPageNameName(int index)
{

    int i = 0;
    for (const auto &kv : pages.as<JsonObject>())
    {
        if (i == index)
        {
            return kv.key().c_str();
        }
        i++;
    }
    return "";
}

// What's displayed along the top line
void msOverlay(OLEDDisplay *display, OLEDDisplayUiState *state)
{
    display->setColor(WHITE);
    display->setFont(ArialMT_Plain_10);
    display->setTextAlignment(TEXT_ALIGN_LEFT);
    char buff[16];
    sprintf_P(buff, PSTR("%02d:%02d"), timeInfo.tm_hour, timeInfo.tm_min);
    display->drawString(0, 54, String(buff));
    display->setTextAlignment(TEXT_ALIGN_RIGHT);
    display->drawString(128, 54, cur_temp + "°C");
    display->drawHorizontalLine(0, 52, 128);
}

void customFrame(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y)
{
    String pageName;
    if (ui.getUiState()->frameState == FIXED)
    {
        pageName = getPageNameName(ui.getUiState()->currentFrame - 2);
    }
    else
    {
        pageName = getPageNameName(ui.getUiState()->currentFrame - 1);
    }

    if (pageName != "")
    {
        JsonArray page = pages[pageName].as<JsonArray>();
        for (JsonObject obj : page)
        {
            int x1 = obj["x"];
            int y1 = obj["y"];
            int s = obj["s"];
            switch (s)
            {
            case 10:
                display->setFont(ArialMT_Plain_10);
                break;
            case 16:
                display->setFont(ArialMT_Plain_16);
                break;
            case 24:
                display->setFont(ArialMT_Plain_24);
                break;
            default:
                display->setFont(ArialMT_Plain_10);
                break;
            }
            JsonObject::iterator it = obj.begin();
            while (it != obj.end())
            {
                String key = it->key().c_str();
                if (key != "x" && key != "y" && key != "s")
                {
                    String vt = it->value().as<String>();
                    display->setTextAlignment(TEXT_ALIGN_LEFT);
                    display->drawString(x1 + x, y1 + y, vt);
                }
                ++it;
            }
        }
    }
}

String getMeteoconIcon(String icon)
{
    bool night;
    if ((timeInfo.tm_hour > 18 && timeInfo.tm_hour < 24) || (timeInfo.tm_hour > -1 && timeInfo.tm_hour < 6))
    {
        night = true;
    }
    else
    {
        night = false;
    }

    // 01d
    if (icon == "o" && !night)
    {
        return "B";
    }
    // 01n
    if (icon == "o" && night)
    {
        return "C";
    }
    // few clouds
    // 02d
    if (icon == "m" && !night)
    {
        return "H";
    }
    // 02n
    if (icon == "m" && night)
    {
        return "4";
    }
    // scattered clouds
    // 03d
    if (icon == "mm" && !night)
    {
        return "N";
    }
    // 03n
    if (icon == "mm" && night)
    {
        return "5";
    }
    // broken clouds
    // 04d
    if (icon == "mmm" && !night)
    {
        return "Y";
    }
    // 04n
    if (icon == "mmm" && night)
    {
        return "%";
    }
    // shower rain
    // 09d
    if (icon == "//" && !night)
    {
        return "R";
    }
    // 09n
    if (icon == "//" && night)
    {
        return "8";
    }
    // rain
    // 10d
    if (icon == "/" && !night)
    {
        return "Q";
    }
    // 10n
    if (icon == "/" && night)
    {
        return "7";
    }
    // thunderstorm
    // 11d
    if (icon == "/!/" && !night)
    {
        return "P";
    }
    // 11n
    if (icon == "/!/" && night)
    {
        return "6";
    }
    // snow
    // 13d
    if (icon == "*" && !night)
    {
        return "W";
    }
    // 13n
    if (icon == "*" && night)
    {
        return "#";
    }
    // mist
    // 50d
    if (icon == "=" && !night)
    {
        return "M";
    }
    // 50n
    if (icon == "=" && night)
    {
        return "M";
    }
    // Nothing matched: N/A
    Serial.println(icon);
    return ")";
}

void weatherFrame(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y)
{
    display->setFont(ArialMT_Plain_10);
    display->setTextAlignment(TEXT_ALIGN_CENTER);
    display->drawString(64 + x, 38 + y, cur_condition);

    display->setFont(ArialMT_Plain_24);
    display->setTextAlignment(TEXT_ALIGN_LEFT);
    String temp = cur_temp + "C°";
    display->drawString(60 + x, 5 + y, temp);

    display->setFont(Meteocons_Plain_36);
    display->setTextAlignment(TEXT_ALIGN_CENTER);
    display->drawString(32 + x, 0 + y, getMeteoconIcon(cur_icon));
}

void drawProgress(OLEDDisplay *display, int percentage, String label)
{
    display->clear();
    display->setTextAlignment(TEXT_ALIGN_CENTER);
    display->setFont(ArialMT_Plain_10);
    display->drawString(64, 10, label);
    display->drawProgressBar(2, 28, 124, 10, percentage);
    display->display();
}

void DateTimeFrame(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y)
{
    now = time(nullptr);
    struct tm *timeInfo;
    timeInfo = localtime(&now);
    char buff[16];

    display->setTextAlignment(TEXT_ALIGN_CENTER);
    display->setFont(ArialMT_Plain_16);
    String date = weekDays[timeInfo->tm_wday];

    sprintf_P(buff, PSTR("%s, %02d/%02d/%04d"), weekDays[timeInfo->tm_wday].c_str(), timeInfo->tm_mday, timeInfo->tm_mon + 1, timeInfo->tm_year + 1900);
    display->drawString(64 + x, 2 + y, String(buff));
    display->setFont(ArialMT_Plain_24);

    sprintf_P(buff, PSTR("%02d:%02d:%02d"), timeInfo->tm_hour, timeInfo->tm_min, timeInfo->tm_sec);
    display->drawString(64 + x, 19 + y, String(buff));
    display->setTextAlignment(TEXT_ALIGN_LEFT);
}

FrameCallback frames[] = {DateTimeFrame, weatherFrame, customFrame, customFrame, customFrame, customFrame};
OverlayCallback overlays[] = {msOverlay};
int overlaysCount = 1;
int frameCount = 2;

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

    display.display();
    if (percent != last_percent)
    {
        uint8_t light = percent / 12.5;
        ButtonManager.setButtonLight(light, 1);
        ButtonManager.tick();
        display.clear();
        display.drawString(DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2, String(percent) + "%");
        last_percent = percent;
        display.display();
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

// function to load screens from JSON file
bool loadCustomScreens()
{
    if (FILESYSTEM.exists("/pages.json"))
    {
        File file = FILESYSTEM.open("/pages.json", "r");
        if (!file)
        {
            Serial.println("Failed to open pages.json file");
            return false;
        }
        DeserializationError error = deserializeJson(pages, file);
        if (error)
        {
            Serial.println("Failed to parse pages.json file");
            return false;
        }

        frameCount = frameCount + pages.size();
        Serial.println(frameCount);
    }
    return true;
}

void SystemManager_::setup()
{

    delay(2000);
    startFilesystem();

    ui.setTargetFPS(60);
    ui.setIndicatorPosition(BOTTOM);      // You can change this to TOP, LEFT, BOTTOM, RIGHT
    ui.setIndicatorDirection(LEFT_RIGHT); // Defines where the first frame is located in the bar
    ui.setFrameAnimation(SLIDE_LEFT);     // You can change the transition that is used SLIDE_LEFT, SLIDE_RIGHT, SLIDE_UP, SLIDE_DOWN

    ui.setOverlays(overlays, overlaysCount); // Add overlays
    ui.init();                               // Initialising the UI will init the display too.
    display.flipScreenVertically();
    display.clear();
    display.drawXbm(0, 0, 128, 64, logo);
    display.display();
    delay(1500);
    display.clear();
    display.setFont(ArialMT_Plain_24);
    display.drawString(45, 20, "v" + String(VERSION));
    display.display();
    delay(800);

    if (loadOptions())
        Serial.println(F("Application option loaded"));
    else
        Serial.println(F("Application options NOT loaded!"));
    MY_CITY = CITY;
    ui.setTimePerFrame(TIME_PER_FRAME);
    ui.setTimePerTransition(TIME_PER_TRANSITION);
    if (PAGE_BUTTONS)
        ui.disableAutoTransition();
    display.clear();

    drawProgress(&display, 0, "Connecting to WiFi");
    IPAddress myIP = mws.startWiFi(15000, "SmartPusher", "12345678");
    mws.addOptionBox("MQTT");
    mws.addOption("Broker", MQTT_HOST);
    mws.addOption("Port", MQTT_PORT);
    mws.addOption("Username", MQTT_USER);
    mws.addOption("Password", MQTT_PASS);
    mws.addOption("Prefix", MQTT_PREFIX);
    mws.addOption("Homeassistant Discovery", HA_DISCOVERY);
    mws.addOptionBox("Buttons");
    mws.addDropdownList("LED Mode", dropdownList, LIST_SIZE);
    mws.addOption("Pushmode for Button 1", BTN1_PUSH);
    mws.addOption("Pushmode for Button 2", BTN2_PUSH);
    mws.addOption("Pushmode for Button 3", BTN3_PUSH);
    mws.addOption("Pushmode for Button 4", BTN4_PUSH);
    mws.addOption("Pushmode for Button 5", BTN5_PUSH);
    mws.addOption("Pushmode for Button 6", BTN6_PUSH);
    mws.addOption("Pushmode for Button 7", BTN7_PUSH);
    mws.addOption("Pushmode for Button 8", BTN8_PUSH);
    mws.addOptionBox("NTP");
    mws.addOption("NTP Server", NTP_SERVER);
    mws.addOption("Timezone", NTP_TZ);
    mws.addHTML("<p>Find your timezone at <a href='https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv' target='_blank' rel='noopener noreferrer'>posix_tz_db</a>.</p>", "tz_link");
    mws.addOptionBox("General");
    mws.addOption("Use RGB buttons", RGB_BUTTONS);
    mws.addOption("Actions over serial", SERIAL_OUT);
    mws.addHTML("<h3>Weather</h3>", "weather_settings");
    mws.addOption("City", CITY);
    mws.addHTML("<h3>Page Settings</h3>", "page_settings");
    mws.addOption("Control with Button 7&8", PAGE_BUTTONS);
    mws.addOption("Use customized pages", CUSTOM_PAGES);
    mws.addOption("Duration per Page", TIME_PER_FRAME);
    mws.addOption("Transistion duration", TIME_PER_TRANSITION);

    mws.begin();
    connected = !(myIP == IPAddress(192, 168, 4, 1));
    if (CUSTOM_PAGES)
        loadCustomScreens();

    ui.setFrames(frames, frameCount); // Add frames
    if (!connected)
    {
        display.setFont(ArialMT_Plain_16);
        display.clear();
        display.setTextAlignment(TEXT_ALIGN_CENTER);
        display.drawString(64, 15, "AP MODE");
        display.drawString(64, 35, "192.168.4.1");
        display.display();
        ButtonManager.blinkAll();
    }
    else
    {
        drawProgress(&display, 0, String(myIP));
        connected = true;
        Update.onProgress(update_progress);
        configTzTime(NTP_TZ.c_str(), NTP_SERVER.c_str());
        MqttManager.setup();
        drawProgress(&display, 10, "Connecting to MQTT");
        MqttManager.tick();
        updateData(&display);
    }
    setBrightness(255);
}

void updateData(OLEDDisplay *display)
{
    drawProgress(display, 30, "Updating time");
    getLocalTime(&timeInfo);
    drawProgress(display, 50, "Updating weather");
    HTTPClient http;
    String url = "https://wttr.in/" + MY_CITY + "?format=j2";
    http.begin(url);
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK)
    {
        String response = http.getString();
        DeserializationError error = deserializeJson(doc, response);
        if (error)
        {
            Serial.println("Error deserializing JSON data: " + String(error.c_str()));
            return;
        }
        cur_temp = doc["current_condition"][0]["temp_C"].as<String>();
        cur_condition = doc["current_condition"][0]["weatherDesc"][0]["value"].as<String>();
    }
    else
    {
        Serial.println(httpCode);
    }

    http.end();

    drawProgress(display, 70, "Updating weathericon");
    url = "https://wttr.in/" + MY_CITY + "?format=%x";
    http.begin(url);
    httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK)
    {
        cur_icon = http.getString();
    }

    http.end();
    FIRST_UPDATE = true;
    drawProgress(display, 50, "Updating forecasts");

    readyForWeatherUpdate = false;
    drawProgress(display, 100, "Done!");
    delay(1000);
}

void SystemManager_::tick()
{
    mws.run();
    if (!connected)
    {

        return;
    }

    if (InternalScreen != 0)
    {
        switch (InternalScreen)
        {
        case 1:
            renderMessagePage();
            break;
        case 2:
            renderImagePage();
            break;
        case 3:
            renderButtonPage();
            break;
        default:
            break;
        }
    }
    else
    {
        if (millis() - timeSinceLastWUpdate > (1000L * UPDATE_INTERVAL_SECS))
        {
            readyForWeatherUpdate = true;
            timeSinceLastWUpdate = millis();
        }

        if (readyForWeatherUpdate && ui.getUiState()->frameState == FIXED)
        {
            updateData(&display);
        }

        int remainingTimeBudget = ui.update();

        if (remainingTimeBudget > 0)
        {
            // You can do some work here
            // Don't do stuff if you are below your
            // time budget.
            delay(remainingTimeBudget);
        }
    }
}

void SystemManager_::show()
{
    display.display();
}

void SystemManager_::clear()
{
    display.clear();
}

void SystemManager_::setBrightness(uint8_t val)
{

    display.setContrast(val);
    if (val == 0)
    {
        display.displayOff();
    }
    else
    {
        display.displayOn();
    };
    lastBrightness = val;
    ButtonManager.setBrightness(val);
}

void SystemManager_::BrightnessOnOff(boolean val)
{
    if (val)
    {
        display.displayOn();
        ButtonManager.setBrightness(lastBrightness);
    }
    else
    {
        display.displayOff();
        ButtonManager.setBrightness(0);
    };
}

void SystemManager_::ShowMessage(String msg)
{
    MQTTMessage = msg;
    previousMillis = millis();
    InternalScreen = 1;
}

void SystemManager_::ShowImage(String img)
{
    Image = img;
    previousMillis = millis();
    InternalScreen = 2;
}

void SystemManager_::ShowButtonPage(uint8_t btn, const char *type)
{
    previousMillis = millis();
    Pushtype = type;
    BtnNr = btn;
    InternalScreen = 3;
}

void SystemManager_::renderMessagePage()
{
    static uint16_t start_at = 0;
    display.clear();
    display.setFont(ArialMT_Plain_24);
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    uint16_t firstline = display.drawStringMaxWidth(64, 0, 128, MQTTMessage.substring(start_at));
    display.display();

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
                InternalScreen = 0;
                MessageShown = false;
                return;
            }
            MessageShown = true;
        }
    }
}

void SystemManager_::renderButtonPage()
{
    if (!TypeShown)
    {
        display.clear();
        display.setFont(Roboto_Black_36);
        display.setTextAlignment(TEXT_ALIGN_CENTER);
        display.drawString(64, 15, Pushtype);
        display.display();
        TypeShown = true;
    }
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= 500)
    {
        previousMillis = currentMillis;
        InternalScreen = 0;
        TypeShown = false;
    }
}

void SystemManager_::renderImagePage()
{
    if (FILESYSTEM.exists("/" + Image + ".bin"))
    {

        File myFile = FILESYSTEM.open("/" + Image + ".bin", "r");
        if (myFile)
        {
            display.clear();
            uint8_t w = myFile.read();
            uint8_t h = myFile.read();
            for (size_t y = 0; y < h; y++)
            {
                uint8_t xpos = 0;
                for (size_t i = 0; i < (w / 8); i++)
                {
                    uint8_t b = myFile.read();
                    for (uint8_t bt = 0; bt < 8; bt++)
                    {
                        display.setPixelColor(xpos++, y, (bitRead(b, bt) ? WHITE : BLACK));
                    }
                }
            }
            myFile.close();
            display.display();
        }
    }
    else
    {
        display.setFont(ArialMT_Plain_16);
        display.drawString(14, 25, "NOT FOUND!");
        display.display();
    }

    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= TIME_PER_FRAME)
    {
        previousMillis = currentMillis;
        InternalScreen = 0;
        ImageShown = false;
    }
    SPIFFS.end();
}

void SystemManager_::setCustomPageVariables(String PageName, String variableName, String Value)
{
    if (pages.containsKey(PageName))
    {
        JsonArray page = pages[PageName].as<JsonArray>();
        for (JsonObject obj : page)
        {
            if (obj.containsKey(variableName))
            {
                obj[variableName] = Value;
                return;
            }
        }
    }
    else
    {
        Serial.println("Page " + PageName + " not found!");
    }
}

void SystemManager_::nextPage()
{
    ui.nextFrame();
}

void SystemManager_::previousPage()
{
    ui.previousFrame();
}