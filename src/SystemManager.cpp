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
#include "converter.h"

// Image data structure
typedef struct
{
    uint8_t *buffer;   // pointer to image data in RAM
    size_t bufferSize; // size of image data in bytes
    char *name;        // name of image file in SPIFFS
} ImageData;

#define MAX_IMAGES 10         // maximum number of images that can be stored in RAM
ImageData images[MAX_IMAGES]; // array of stored images
int numImages = 0;            // number of images stored in the array

SSD1306 display(0x3c, SDA, SCL);
OLEDDisplayUi ui(&display);
File fsUploadFile;

HTTPClient http;
#define DISPLAY_WIDTH 128 // OLED display width, in pixels
#define DISPLAY_HEIGHT 64 // OLED display height, in pixels
int16_t x_con = 128;
const char *VERSION = "2.50";

time_t now;
tm timeInfo;

uint8_t InternalScreen = 0;
boolean connected = false;

StaticJsonDocument<4096> pages;

unsigned long previousMillis = 0;
const long CLOCK_INTERVAL = 1000;
const long PICTURE_INTERVAL = 2000;
const long CHECK_WIFI_TIME = 10000;
bool readyDataUpdate = false;
long timeSinceLastWUpdate = 0;
unsigned long PREVIOUS_WIFI_CHECK = 0;
unsigned long PREVIOUS_WEATHER_CHECK = 0;
unsigned long PREVIOUS_WIFI_MILLIS = 0;
const int UPDATE_INTERVAL_SECS = 20 * 60; // Update every 20 minutes

bool _SHOW_DATE;
bool _SHOW_SECONDS;
String _MY_CITY;

const char *Pushtype;
String MQTTMessage;
String ImageName;
uint8_t BtnNr;

boolean TypeShown;
boolean MessageShown;
boolean ImageShown;
String weekDay;
String fYear;
String fDate;
String fTime;
uint8_t lastBrightness = 100;

const String weekDays[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

#define FILESYSTEM LittleFS
WebServer server(80);
FSWebServer mws(FILESYSTEM, server);

#define LIST_SIZE 5
const char *dropdownList[LIST_SIZE] = {
    "Off", "On", "Fade", "Extern", "OnPush"};

String cur_temp, cur_condition, cur_icon;

// The getter for the instantiated singleton instance
SystemManager_ &SystemManager_::getInstance()
{
    static SystemManager_ instance;
    return instance;
}

IPAddress local_IP;
IPAddress gateway;
IPAddress subnet;
IPAddress primaryDNS;
IPAddress secondaryDNS;

int overlaysCount = 1;
int TotalFrames = 0;
int CustomFrames = 0;
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
        DEBUG_PRINTLN();
    }
    else
    {
        DEBUG_PRINTLN("ERROR on mounting filesystem. It will be formmatted!");
        FILESYSTEM.format();
        ESP.restart();
    }
}

////////////////////  Load&Save application options from filesystem  ////////////////////
bool SystemManager_::loadOptions()
{
    if (FILESYSTEM.exists("/config.json"))
    {
        mws.getOptionValue("Use RGB buttons", RGB_BUTTONS);
        mws.getOptionValue("Show customized pages", CUSTOM_PAGES);
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
        mws.getOptionValue("Homeassistant discovery", HA_DISCOVERY);
        mws.getOptionValue("Duration per Page", TIME_PER_FRAME);
        mws.getOptionValue("Transistion duration", TIME_PER_TRANSITION);
        mws.getOptionValue("Control with Button 7&8", PAGE_BUTTONS);
        mws.getOptionValue("ioBroker advertisement ", IO_BROKER);
        mws.getOptionValue("Static IP", NET_STATIC);
        mws.getOptionValue("Local IP", NET_IP);
        mws.getOptionValue("Gateway", NET_GW);
        mws.getOptionValue("Subnet", NET_SN);
        mws.getOptionValue("Primary DNS", NET_PDNS);
        mws.getOptionValue("Secondary DNS", NET_SDNS);
        mws.getOptionValue("Hide date", SHOW_DATE);
        mws.getOptionValue("Hide seconds", SHOW_SECONDS);
        mws.getOptionValue("Show DateTime page", SHOW_DATETIME);
        mws.getOptionValue("Show Weather page", SHOW_WEATHER);

        if (!local_IP.fromString(NET_IP) || !gateway.fromString(NET_GW) || !subnet.fromString(NET_SN) || !primaryDNS.fromString(NET_PDNS) || !secondaryDNS.fromString(NET_SDNS))
            NET_STATIC = false;

        return true;
    }
    else
        DEBUG_PRINTLN(F("File \"config.json\" not exist"));
    return false;
}

void SystemManager_::saveOptions()
{
    mws.saveOptionValue("Homeassistant discovery", HA_DISCOVERY);
    mws.saveOptionValue("Use RGB buttons", RGB_BUTTONS);
    mws.saveOptionValue("Show customized pages", CUSTOM_PAGES);
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
    mws.saveOptionValue("ioBroker advertisement ", IO_BROKER);
    mws.saveOptionValue("Static IP", NET_STATIC);
    mws.saveOptionValue("Local IP", NET_IP);
    mws.saveOptionValue("Gateway", NET_GW);
    mws.saveOptionValue("Subnet", NET_SN);
    mws.saveOptionValue("Primary DNS", NET_PDNS);
    mws.saveOptionValue("Secondary DNS", NET_SDNS);
    mws.saveOptionValue("Hide date", SHOW_DATE);
    mws.saveOptionValue("Hide seconds", SHOW_SECONDS);
    mws.saveOptionValue("Show DateTime page", SHOW_DATETIME);
    mws.saveOptionValue("Show Weather page", SHOW_WEATHER);

    DEBUG_PRINTLN(F("Application options saved."));
}

// function to get the name of the current screen
String getPageByIndex(int index)
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

int getPagebyName(const String &name)
{
    int i = 0;
    for (const auto &kv : pages.as<JsonObject>())
    {
        if (strcmp(kv.key().c_str(), name.c_str()) == 0)
        {
            return i + 2;
        }
        i++;
    }
    return 0; // return -1 if the name is not found
}

void SystemManager_::scrolling(bool active)
{
    active ? ui.enableAutoTransition() : ui.disableAutoTransition();
}

void SystemManager_::showPage(String pageName)
{

    if (pageName == "time")
    {
        ui.transitionToFrame(0);
    }
    else if (pageName == "weather")
    {
        ui.transitionToFrame(1);
    }
    else
    {
        ui.transitionToFrame(getPagebyName(pageName));
    }
}

void SystemManager_::sendCustomPageKeys()
{
    for (JsonPair page : pages.as<JsonObject>())
    {
        for (auto obj : page.value().as<JsonArray>())
        {
            for (JsonPair item : obj.as<JsonObject>())
            {
                String key = item.key().c_str();
                if (key == "t" && item.value().as<String>() == "text")
                    continue;
                if (key != "x" && key != "y" && key != "s" && key != "t")
                {
                    String topic = "custompage/" + String(page.key().c_str()) + "/" + key;
                    String value = item.value().as<String>();
                    MqttManager.publish(topic.c_str(), value.c_str());
                }
            }
        }
    }
}

void addImageToRAM(const String &name)
{
    DEBUG_PRINTLN(name);
    for (int i = 0; i < numImages; i++)
    {
        if (strcmp(images[i].name, name.c_str()) == 0)
        {
            // Image already in RAM, do nothing
            return;
        }
    }

    // Load image from SPIFFS into RAM
    // Load image from SPIFFS into RAM
    File myFile = FILESYSTEM.open("/" + name + ".bin", "r");
    if (myFile)
    {
        uint8_t w = myFile.read();
        uint8_t h = myFile.read();
        size_t dataIndex = 2;
        size_t xbmDataIndex = 0;
        uint8_t xbmData[(w / 8) * h];
        for (size_t y = 0; y < h; y++)
        {
            for (size_t i = 0; i < (w / 8); i++)
            {
                uint8_t b = myFile.read();
                xbmData[xbmDataIndex++] = b;
                dataIndex++;
            }
        }
        myFile.close();
        // Add image data to array
        images[numImages].buffer = (uint8_t *)malloc(xbmDataIndex + 2);
        if (!images[numImages].buffer)
        {
            DEBUG_PRINTLN("Out of memory error");
            return;
        }
        images[numImages].buffer[0] = w;
        images[numImages].buffer[1] = h;
        memcpy(&images[numImages].buffer[2], xbmData, xbmDataIndex);
        images[numImages].bufferSize = xbmDataIndex + 2;
        images[numImages].name = strdup(name.c_str());
        numImages++;
    }
}

void renderImage(uint8_t x, uint8_t y, const String &name)
{
    // Find image in array
    int imageIndex = -1;
    for (int i = 0; i < numImages; i++)
    {
        if (strcmp(images[i].name, name.c_str()) == 0)
        {
            imageIndex = i;
            break;
        }
    }
    if (imageIndex < 0)
    {
        DEBUG_PRINTLN("Image not found in RAM, load from SPIFFS");
        addImageToRAM(name);
        imageIndex = numImages - 1;
    }
    // Display image from RAM buffer
    uint8_t w = images[imageIndex].buffer[0];
    uint8_t h = images[imageIndex].buffer[1];
    uint8_t *xbmData = &images[imageIndex].buffer[2];
    size_t xbmDataSize = images[imageIndex].bufferSize - 2;
    display.drawXbm(x, y, w, h, xbmData);
}

void SystemManager_::renderImagePage()
{
    display.clear();
    renderImage(0, 0, ImageName);
    display.display();
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= TIME_PER_FRAME)
    {
        previousMillis = currentMillis;
        InternalScreen = 0;
        ImageShown = false;
    }
}

// What's displayed along the top line
void msOverlay(OLEDDisplay *display, OLEDDisplayUiState *state)
{
    if (TotalFrames < 7)
    {
        display->setColor(WHITE);
        display->setFont(ArialMT_Plain_10);
        display->setTextAlignment(TEXT_ALIGN_LEFT);
        char buff[16];
        sprintf_P(buff, PSTR("%02d:%02d"), timeInfo.tm_hour, timeInfo.tm_min);
        display->drawString(0, 54, String(buff));

        if (cur_temp != "")
        {
            display->setTextAlignment(TEXT_ALIGN_RIGHT);
            display->drawString(128, 54, cur_temp + "°C");
            display->drawHorizontalLine(0, 52, 128);
        }
    }
}

void SystemManager_::setCustomPageVariables(String PageName, String variableName, String Value)
{
    if (CustomFrames == 0)
        return;
    if (pages.containsKey(PageName))
    {
        pages.garbageCollect();
        JsonArray page = pages[PageName].as<JsonArray>();
        for (JsonObject obj : page)
        {
            if (obj.containsKey(variableName))
            {
                DEBUG_PRINTLN("Set " + Value + " for " + variableName + " in " + PageName);
                obj[variableName] = Value;
                return;
            }
        }
    }
    else
    {
        DEBUG_PRINTLN("Page " + PageName + " not found!");
    }
}

void drawCustomFrame(uint8_t pageIndex, OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y)
{
    String pageName = getPageByIndex(pageIndex);
    if (pageName != "")
    {
        JsonArray page = pages[pageName].as<JsonArray>();
        for (JsonObject obj : page)
        {
            int x1 = obj["x"];
            int y1 = obj["y"];
            display->setTextAlignment(TEXT_ALIGN_LEFT);
            String type = obj["t"].as<String>();
            if (type == "text")
            {
                if (obj.containsKey("s"))
                {
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
                    case 30:
                        display->setFont(Roboto_Black_30);
                        break;
                    case 36:
                        display->setFont(Roboto_Black_36);
                        break;
                    default:
                        display->setFont(ArialMT_Plain_10);
                        break;
                    }
                }
                JsonObject::iterator it = obj.begin();
                while (it != obj.end())
                {
                    String key = it->key().c_str();
                    String vt = it->value().as<String>();
                    if (key == "x" || key == "y" || key == "s" || key == "t")
                    {
                        ++it;     // Gehe zum nächsten Eintrag
                        continue; // Überspringe die Verarbeitung dieses Eintrags
                    }

                    if (!it->value().is<String>())
                    {
                        DEBUG_PRINTLN("Ungültiger Wert für Schlüssel in " + pageName + ": " + key + "-" + vt);
                        serializeJson(obj, Serial);
                        break; // Beende die Schleife, um die komplette JSON-Ausgabe nur einmal anzuzeigen
                    }
                    display->drawString(x1 + x, y1 + y, vt);
                    ++it;
                }
            }
            else if (type == "image")
            {
                String image = obj["i"].as<String>();
                renderImage(x1 + x, y1 + y, image);
            }
            else if (type == "bar")
            {
                int w = obj["w"];
                int h = obj["h"];
                int v = obj["v"];
                display->drawProgressBar(x1 + x, y1 + y, w, h, v);
            }
            else
            {
                // Handle andere Strings
            }
        }
    }
}

void customFrame1(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y)
{
    drawCustomFrame(0, display, state, x, y);
}

void customFrame2(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y)
{
    drawCustomFrame(1, display, state, x, y);
}

void customFrame3(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y)
{
    drawCustomFrame(2, display, state, x, y);
}

void customFrame4(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y)
{
    drawCustomFrame(3, display, state, x, y);
}

void customFrame5(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y)
{
    drawCustomFrame(4, display, state, x, y);
}

void customFrame6(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y)
{
    drawCustomFrame(5, display, state, x, y);
}

void customFrame7(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y)
{
    drawCustomFrame(6, display, state, x, y);
}

void customFrame8(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y)
{
    drawCustomFrame(7, display, state, x, y);
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
    return ")";
}

void weatherFrame(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y)
{
    static int16_t textWidth = display->getStringWidth(cur_condition);
    if (textWidth > 128)
    {
        display->setTextAlignment(TEXT_ALIGN_LEFT);
        if (x_con < -textWidth)
        {
            x_con = 128;
        }
        x_con--;
    }
    else
    {
        x_con = 0;
        display->setTextAlignment(TEXT_ALIGN_CENTER);
    }

    display->setFont(ArialMT_Plain_10);
    display->drawString(64 + x + x_con, 38 + y, cur_condition);
    display->setFont(ArialMT_Plain_24);
    display->setTextAlignment(TEXT_ALIGN_LEFT);
    display->drawString(60 + x, 5 + y, cur_temp + "C°");

    display->setFont(Meteocons_Plain_36);
    display->setTextAlignment(TEXT_ALIGN_CENTER);
    display->drawString(32 + x, 0 + y, getMeteoconIcon(cur_icon));
}

void drawProgress(OLEDDisplay *display, int percentage, String label)
{
    display->clear();
    display->setTextAlignment(TEXT_ALIGN_CENTER);
    display->setFont(ArialMT_Plain_16);
    display->drawStringMaxWidth(64, 4, 128, label);
    display->drawProgressBar(2, 45, 124, 10, percentage);
    display->display();
}

void DateTimeFrame(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y)
{
    now = time(nullptr);
    struct tm *timeInfo;
    timeInfo = localtime(&now);
    char buff[16];

    if (_SHOW_DATE)
    {
        display->setTextAlignment(TEXT_ALIGN_CENTER);
        display->setFont(ArialMT_Plain_16);
        String date = weekDays[timeInfo->tm_wday];

        sprintf_P(buff, PSTR("%s, %02d/%02d/%04d"), weekDays[timeInfo->tm_wday].c_str(), timeInfo->tm_mday, timeInfo->tm_mon + 1, timeInfo->tm_year + 1900);
        display->drawString(64 + x, 2 + y, String(buff));
        display->setFont(ArialMT_Plain_24);

        if (_SHOW_SECONDS)
        {
            sprintf_P(buff, PSTR("%02d:%02d:%02d"), timeInfo->tm_hour, timeInfo->tm_min, timeInfo->tm_sec);
        }
        else
        {
            sprintf_P(buff, PSTR("%02d:%02d"), timeInfo->tm_hour, timeInfo->tm_min);
        }

        display->drawString(64 + x, 19 + y, String(buff));
    }
    else
    {
        display->setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
        display->setFont(Roboto_Black_30);
        if (_SHOW_SECONDS)
        {
            sprintf_P(buff, PSTR("%02d:%02d:%02d"), timeInfo->tm_hour, timeInfo->tm_min, timeInfo->tm_sec);
        }
        else
        {
            sprintf_P(buff, PSTR("%02d:%02d"), timeInfo->tm_hour, timeInfo->tm_min);
        }
        display->drawString(64 + x, 28 + y, String(buff));
        display->setTextAlignment(TEXT_ALIGN_LEFT);
    }
}

void SettingsSaved(String result)
{
    ButtonManager.setStates();
    return;
}

void update_started()
{
    DEBUG_PRINTLN("CALLBACK:  HTTP update process started");
}

void update_finished()
{
    DEBUG_PRINTLN("CALLBACK:  HTTP update process finished");
}

void update_progress(int cur, int total)
{
    static int last_percent = 0;
    Serial.printf("CALLBACK:  HTTP update process at %d of %d bytes...\n", cur, total);

    int percent = (100 * cur) / total;
    DEBUG_PRINTLN(percent);

    display.display();
    if (percent != last_percent)
    {
        uint8_t light = percent / 12.5;
        ButtonManager.setButtonLight(light, 1);
        ButtonManager.tick();
        display.clear();
        display.setTextAlignment(TEXT_ALIGN_CENTER);
        display.setFont(ArialMT_Plain_24);
        display.drawString(64, DISPLAY_HEIGHT / 2, String(percent) + "%");
        last_percent = percent;
        display.display();
    }
}

std::vector<FrameCallback> frames;
OverlayCallback overlays[] = {msOverlay};

// function to load screens from JSON file
bool loadCustomScreens()
{
    if (FILESYSTEM.exists("/pages.json"))
    {
        File file = FILESYSTEM.open("/pages.json", "r");
        if (!file)
        {
            DEBUG_PRINTLN("Failed to open pages.json file");
            return false;
        }
        DeserializationError error = deserializeJson(pages, file);
        if (error)
        {
            DEBUG_PRINTLN("Failed to parse pages.json file");
            return false;
        }

        CustomFrames = pages.size();
        DEBUG_PRINTLN("Found " + String(CustomFrames) + " custom pages");
        void (*customFrames[8])(OLEDDisplay *, OLEDDisplayUiState *, int16_t, int16_t) = {customFrame1, customFrame2, customFrame3, customFrame4, customFrame5, customFrame6, customFrame7, customFrame8};
        for (int i = 0; i < CustomFrames && i < 8; i++)
        {
            if (customFrames[i] != NULL)
            {
                frames.push_back(customFrames[i]);
            }
        }
    }
    return true;
}

void SystemManager_::setup()
{
    delay(2000);
    startFilesystem();
    ui.setTargetFPS(40);
    ui.setIndicatorPosition(BOTTOM);         // You can change this to TOP, LEFT, BOTTOM, RIGHT
    ui.setIndicatorDirection(LEFT_RIGHT);    // Defines where the first frame is located in the bar
    ui.setFrameAnimation(SLIDE_LEFT);        // You can change the transition that is used SLIDE_LEFT, SLIDE_RIGHT, SLIDE_UP, SLIDE_DOWN
    ui.setOverlays(overlays, overlaysCount); // Add overlays
    ui.init();                               // Initialising the UI will init the display too.
    display.flipScreenVertically();
    display.clear();
    display.drawXbm(0, 0, 128, 64, logo);
    display.display();
    delay(1000);
    display.clear();
    display.setFont(ArialMT_Plain_24);
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(64, 20, "v" + String(VERSION));
    display.display();
    delay(500);
    drawProgress(&display, 5, "Loading Settings");
    if (loadOptions())
        DEBUG_PRINTLN("Application option loaded");
    _MY_CITY = CITY;
    _SHOW_DATE = SHOW_DATE;
    _SHOW_SECONDS = SHOW_SECONDS;
    ui.setTimePerFrame(TIME_PER_FRAME);
    ui.setTimePerTransition(TIME_PER_TRANSITION);
    if (PAGE_BUTTONS)
        ui.disableAutoTransition();

    drawProgress(&display, 10, "Connecting to WiFi");
    if (NET_STATIC)
    {
        WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS);
    }

    IPAddress myIP = mws.startWiFi(10000, "SmartPusher", "12345678");
    connected = !(myIP == IPAddress(192, 168, 4, 1));
    if (connected)
    {
        drawProgress(&display, 10, WiFi.localIP().toString());
        delay(2000);
    }

    drawProgress(&display, 20, "Loading Webinterface");
    mws.addOptionBox("Network");
    mws.addOption("Static IP", NET_STATIC);
    mws.addOption("Local IP", NET_IP);
    mws.addOption("Gateway", NET_GW);
    mws.addOption("Subnet", NET_SN);
    mws.addOption("Primary DNS", NET_PDNS);
    mws.addOption("Secondary DNS", NET_SDNS);
    mws.addOptionBox("MQTT");
    mws.addOption("Broker", MQTT_HOST);
    mws.addOption("Port", MQTT_PORT);
    mws.addOption("Username", MQTT_USER);
    mws.addOption("Password", MQTT_PASS);
    mws.addOption("Prefix", MQTT_PREFIX);
    mws.addOption("Homeassistant discovery", HA_DISCOVERY);
    mws.addOption("ioBroker advertisement ", IO_BROKER);
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
    mws.addOptionBox("Time");
    mws.addOption("Show date", SHOW_DATE);
    mws.addOption("Show seconds", SHOW_SECONDS);
    mws.addOption("NTP Server", NTP_SERVER);
    mws.addOption("Timezone", NTP_TZ);
    mws.addHTML("<p>Find your timezone at <a href='https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv' target='_blank' rel='noopener noreferrer'>posix_tz_db</a>.</p>", "tz_link");
    mws.addOptionBox("General");
    mws.addOption("Use RGB buttons", RGB_BUTTONS);
    mws.addOption("Actions over serial", SERIAL_OUT);
    mws.addHTML("<h3>Weather</h3>", "weather_settings");
    mws.addOption("City", CITY);
    mws.addHTML("<h3>Page Settings</h3>", "page_settings");
    mws.addOption("Show DateTime page", SHOW_DATETIME);
    mws.addOption("Show Weather page", SHOW_WEATHER);
    mws.addOption("Show customized pages", CUSTOM_PAGES);
    mws.addOption("Control with Button 7&8", PAGE_BUTTONS);
    mws.addOption("Duration per Page", TIME_PER_FRAME);
    mws.addOption("Transistion duration", TIME_PER_TRANSITION);
    mws.addOptionBox("Images");
    mws.addHTML(custom_html, "custom-html");
    mws.addJavascript(custom_script);
    mws.addCSS(custom_css);
    mws.getRequest()->setContentLength(10240);
    mws.begin();

    if (SHOW_DATETIME)
        frames.push_back(DateTimeFrame);
    if (SHOW_WEATHER)
        frames.push_back(weatherFrame);
    if (CUSTOM_PAGES)
        loadCustomScreens();
    TotalFrames = frames.size();
    ui.setFrames(frames.data(), TotalFrames); // Add frames
    if (TotalFrames == 1)
        ui.disableAutoTransition();
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
        connected = true;
        Update.onProgress(update_progress);
        configTzTime(NTP_TZ.c_str(), NTP_SERVER.c_str());
        MqttManager.setup();
        drawProgress(&display, 30, "Connecting to MQTT");
        MqttManager.tick();
        UpdateData();
    }

    setBrightness(255);
}

void SystemManager_::UpdateData()
{
    DEBUG_PRINTLN("Updating Data");
    drawProgress(&display, 40, "Updating time");
    getLocalTime(&timeInfo);

    if (SHOW_WEATHER)
    {
        drawProgress(&display, 60, "Updating weather");
        String url = "https://wttr.in/" + _MY_CITY + "?format=j2";
        http.begin(url);
        int httpCode = http.GET();
        if (httpCode == HTTP_CODE_OK)
        {
            String response = http.getString();
            DynamicJsonDocument doc(3072);
            DeserializationError error = deserializeJson(doc, response);
            if (error)
            {
                DEBUG_PRINTLN("Error deserializing JSON data: " + String(error.c_str()));
                return;
            }
            cur_temp = doc["current_condition"][0]["temp_C"].as<String>();
            cur_condition = doc["current_condition"][0]["weatherDesc"][0]["value"].as<String>();
        }
        else
        {
            DEBUG_PRINTLN(httpCode);
        }

        http.end();
        drawProgress(&display, 70, "Updating weathericon");
        url = "https://wttr.in/" + _MY_CITY + "?format=%x";
        http.begin(url);
        httpCode = http.GET();
        if (httpCode == HTTP_CODE_OK)
        {
            cur_icon = http.getString();
        }

        http.end();
        drawProgress(&display, 80, "Updating forecasts");
    }
    readyDataUpdate = false;

    drawProgress(&display, 100, "Done!");
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
            readyDataUpdate = true;
            timeSinceLastWUpdate = millis();
        }

        if (readyDataUpdate && ui.getUiState()->frameState == FIXED)
        {
            UpdateData();
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
    ImageName = img;
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

void SystemManager_::nextPage()
{
    ui.nextFrame();
}

void SystemManager_::previousPage()
{
    ui.previousFrame();
}