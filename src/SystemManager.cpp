#include <SystemManager.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <WebConfig.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <config.h>
#include <WiFi.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

U8G2_SSD1306_128X64_NONAME_F_SW_I2C gfx(U8G2_R0, /* clock=*/SCL, /* data=*/SDA, /* reset=*/U8X8_PIN_NONE);

const int daylightOffset_sec = 3600;
tm timeinfo;

uint8_t screen = 0;
boolean connected = false;
unsigned long previousMillis = 0;
const long CLOCK_INTERVAL = 1000;
const long CHECK_WIFI_TIME = 10000;
unsigned long PREVIOUS_WIFI_MILLIS = 0;
const char *Pushtype;
uint8_t BtnNr;
boolean TypeShown;

const String weekDays[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
bool colon_switch = true;

String params = "["
                "{"
                "'name':'ssid',"
                "'label':'SSID',"
                "'type':" +
                String(INPUTTEXT) + ","
                                    "'default':''"
                                    "},"
                                    "{"
                                    "'name':'pwd',"
                                    "'label':'Password',"
                                    "'type':" +
                String(INPUTPASSWORD) + ","
                                        "'default':''"
                                        "},"
                                        "{"
                                        "'name':'mqttbroker',"
                                        "'label':'MQTT Broker',"
                                        "'type':" +
                String(INPUTTEXT) + ","
                                    "'default':''"
                                    "},"
                                    "{"
                                    "'name':'mqttprefix',"
                                    "'label':'MQTT Prefix',"
                                    "'type':" +
                String(INPUTTEXT) + ","
                                    "'default':'SmartPusher'"
                                    "},"
                                    "{"
                                    "'name':'mqttport',"
                                    "'label':'MQTT Port',"
                                    "'min':1,'max':9999,"
                                    "'type':" +
                String(INPUTNUMBER) + ","
                                      "'default':'1883'"
                                      "},"
                                      "{"
                                      "'name':'mqttuser',"
                                      "'label':'MQTT Username',"
                                      "'type':" +
                String(INPUTTEXT) + ","
                                    "'default':''"
                                    "},"
                                    "{"
                                    "'name':'mqttpwd',"
                                    "'label':'MQTT Password',"
                                    "'type':" +
                String(INPUTPASSWORD) + ","
                                        "'default':''"
                                        "},"
                                        "{"
                                        "'name':'colonblink',"
                                        "'label':'Colon Blink',"
                                        "'type':" +
                String(INPUTCHECKBOX) + ","
                                        "'default':'1'"
                                        "},"
                                        "{"
                                        "'name':'btn1push',"
                                        "'label':'Button 1 Pushmode',"
                                        "'type':" +
                String(INPUTCHECKBOX) + ","
                                        "'default':'0'"
                                        "},"
                                        "{"
                                        "'name':'btn2push',"
                                        "'label':'Button 2 Pushmode',"
                                        "'type':" +
                String(INPUTCHECKBOX) + ","
                                        "'default':'0'"
                                        "},"
                                        "{"
                                        "'name':'btn3push',"
                                        "'label':'Button 3 Pushmode',"
                                        "'type':" +
                String(INPUTCHECKBOX) + ","
                                        "'default':'0'"
                                        "},"
                                        "{"
                                        "'name':'btn4push',"
                                        "'label':'Button 4 Pushmode',"
                                        "'type':" +
                String(INPUTCHECKBOX) + ","
                                        "'default':'0'"
                                        "},"
                                        "{"
                                        "'name':'btn5push',"
                                        "'label':'Button 5 Pushmode',"
                                        "'type':" +
                String(INPUTCHECKBOX) + ","
                                        "'default':'0'"
                                        "},"
                                        "{"
                                        "'name':'btn6push',"
                                        "'label':'Button 6 Pushmode',"
                                        "'type':" +
                String(INPUTCHECKBOX) + ","
                                        "'default':'0'"
                                        "},"
                                        "{"
                                        "'name':'btn7push',"
                                        "'label':'Button 7 Pushmode',"
                                        "'type':" +
                String(INPUTCHECKBOX) + ","
                                        "'default':'0'"
                                        "},"
                                        "{"
                                        "'name':'btn8push',"
                                        "'label':'Button 8 Pushmode',"
                                        "'type':" +
                String(INPUTCHECKBOX) + ","
                                        "'default':'0'"
                                        "},{"
                                        "'name':'leds',"
                                        "'label':'LEDS',"
                                        "'type':" +
                String(INPUTSELECT) + ","
                                      "'options':["
                                      "{'v':'0','l':'Off'},"
                                      "{'v':'1','l':'On'},"
                                      "{'v':'2','l':'Fade'}],"
                                      "'default':'1'"
                                      "}"
                                      "]";

WebServer server;
WebConfig conf;

boolean initWiFi()
{

    WiFi.mode(WIFI_STA);
    Serial.print("Verbindung zu ");
    Serial.print(conf.values[0]);
    Serial.println(" herstellen");
    gfx.setFont(u8g2_font_tenfatguys_tr);
    gfx.drawStr(10, 10, "Connecting");
    gfx.drawStr(10, 30, "to WiFi...");
    gfx.sendBuffer();
    if (conf.values[0] != "")
    {
        WiFi.begin(conf.values[0].c_str(), conf.values[1].c_str());
        uint8_t cnt = 0;
        while ((WiFi.status() != WL_CONNECTED) && (cnt < 20))
        {
            delay(500);
            Serial.print(".");
            cnt++;
        }
        Serial.println();
        if (WiFi.status() == WL_CONNECTED)
        {
            Serial.print("IP-Adresse = ");
            Serial.println(WiFi.localIP());
            gfx.clearBuffer();
            gfx.drawStr(15, 25, "Connected!");
            gfx.drawStr((gfx.getDisplayWidth() - gfx.getUTF8Width(WiFi.localIP().toString().c_str())) / 2, 45, WiFi.localIP().toString().c_str());
            gfx.sendBuffer();
            connected = true;
            delay(3000);
        }
    }
    if (!connected)
    {
        WiFi.mode(WIFI_AP);
        WiFi.softAP("SmartPusher", "", 1);
        gfx.clearBuffer();
        gfx.drawStr(15, 25, "AP MODE");
        gfx.drawStr(10, 40, "192.168.1.4");
        gfx.sendBuffer();
    }
    return connected;
}

void handleRoot()
{
    conf.handleFormRequest(&server);
    if (server.hasArg("SAVE"))
    {
        uint8_t cnt = conf.getCount();
        Serial.println("*********** Konfiguration ************");
        for (uint8_t i = 0; i < cnt; i++)
        {
            Serial.print(conf.getName(i));
            Serial.print(" = ");
            Serial.println(conf.values[i]);
        }
    }
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

void SystemManager_::setup()
{

    gfx.begin();
    gfx.clearBuffer();                 // clear the internal memory
    gfx.setFont(u8g2_font_ncenB08_tr); // choose a suitable font
    delay(1000);
    conf.registerOnSave(SettingsSaved);
    conf.setDescription(params);
    conf.readConfig();

    initWiFi();

    server.on("/", handleRoot);
    server.begin(80);

    if (connected)
    {
        char dns[30];
        sprintf(dns, "%s.local", conf.getApName());
        if (MDNS.begin(dns))
        {
            Serial.println("MDNS responder gestartet");
        }

        configTzTime(TIMEZONE, NTP_SERVER);
        getLocalTime(&timeinfo);
    }
}

void SystemManager_::tick()
{
    server.handleClient();

    switch (screen)
    {
    case 0:
        renderClockScreen();
        break;
    case 1:
        renderButtonScreen();
    default:
        break;
    }
}

void SystemManager_::drawtext(uint8_t x, uint8_t y, String text)
{
}

void SystemManager_::show()
{
    gfx.sendBuffer();
}

void SystemManager_::clear()
{
    gfx.clearBuffer();
}

void SystemManager_::setBrightness(uint8_t val)
{
}

const char *SystemManager_::getValue(const char *tag)
{
    return conf.getValue(tag);
}

boolean SystemManager_::getBool(const char *tag)
{
    return conf.getBool(tag);
}

String SystemManager_::getString(const char *tag)
{
    return conf.getString(tag);
}

int SystemManager_::getInt(const char *tag)
{
    return conf.getInt(tag);
}

void SystemManager_::ShowButtonScreen(uint8_t btn, const char *type)
{
    previousMillis = millis();
    Pushtype = type;
    BtnNr = btn;
    screen = 1;
}

void SystemManager_::renderButtonScreen()
{
    if (!TypeShown)
    {
        gfx.clearBuffer();
        gfx.setFont(u8g2_font_inb24_mr);
        gfx.drawStr((gfx.getDisplayWidth() - gfx.getUTF8Width(Pushtype)) / 2, 45, Pushtype);
        gfx.sendBuffer();
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

void SystemManager_::renderClockScreen()
{
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= CLOCK_INTERVAL)
    {
        gfx.clearBuffer();
        previousMillis = currentMillis;

               String weekDay = weekDays[timeinfo.tm_wday];

        if (conf.getBool("colonblink"))
        {
            colon_switch = !colon_switch;
        }
        else
        {
            colon_switch = true;
        }

        String fYear = String(1900 + timeinfo.tm_year);
        String fDate = (timeinfo.tm_mday < 10 ? "0" : "") + String(timeinfo.tm_mday) + "/" + (timeinfo.tm_mon + 1 < 10 ? "0" : "") + String(timeinfo.tm_mon + 1);
        String fTime = (timeinfo.tm_hour < 10 ? "0" : "") + String(timeinfo.tm_hour) + (colon_switch ? ":" : " ") + (timeinfo.tm_min < 10 ? "0" : "") + String(timeinfo.tm_min);

        gfx.setFont(u8g2_font_inr16_mf);
        gfx.drawStr(0, 16, strcpy(new char[fDate.length() + 1], fDate.c_str()));
        gfx.setFont(u8g2_font_pxplusibmcgathin_8f);
        gfx.drawStr(93, 8, strcpy(new char[fYear.length() + 1], fYear.c_str()));
        gfx.drawStr(93, 17, strcpy(new char[weekDay.length() + 1], weekDay.c_str()));
        gfx.setFont(u8g2_font_inb30_mn);
        gfx.drawStr(2, 58, strcpy(new char[fTime.length() + 1], fTime.c_str()));

        gfx.sendBuffer();
    }
}