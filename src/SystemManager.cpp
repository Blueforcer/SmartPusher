#include <SystemManager.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <WebConfig.h>
#include "SSD1306.h"
#include <Wire.h>
#include <config.h>
#include <WiFi.h>
#include <Update.h>
#include "font.h"
#include "images.h"
#include "SPI.h"
#include <SPIFFS.h>

#define DISPLAY_WIDTH 128 // OLED display width, in pixels
#define DISPLAY_HEIGHT 64 // OLED display height, in pixels
const char *VERSION = "1.5";

// U8G2_SSD1306_128X64_NONAME_F_SW_I2C gfx(U8G2_R0, /* clock=*/SCL, /* data=*/SDA, /* reset=*/U8X8_PIN_NONE);
SSD1306 gfx(0x3c, SDA, SCL);

const int daylightOffset_sec = 3600;
tm timeinfo;

uint8_t screen = 0;
boolean connected = false;
unsigned long previousMillis = 0;
const long CLOCK_INTERVAL = 1000;
const long CHECK_WIFI_TIME = 10000;
unsigned long PREVIOUS_WIFI_MILLIS = 0;
const char *Pushtype;
String Message;
String Image;

uint8_t BtnNr;
boolean TypeShown;
boolean MessageShown;
String weekDay;
String fYear;
String fDate;
String fTime;

File fsUploadFile;
String temp = "";

const String weekDays[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
bool colon_switch = true;
const char *updateIndex = "<form method='POST' action='/doupdate' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";
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
                                        "'name':'ntp',"
                                        "'label':'NTP Server',"
                                        "'type':" +
                String(INPUTTEXT) + ","
                                    "'default':'de.pool.ntp.org'"
                                    "},"
                                    "{"
                                    "'name':'tz',"
                                    "'label':'TZ INFO',"
                                    "'type':" +
                String(INPUTTEXT) + ","
                                    "'default':'CET-1CEST,M3.5.0/02,M10.5.0/03'"
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
                                      "{'v':'2','l':'Fade'},"
                                      "{'v':'3','l':'Extern'}],"
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
    gfx.setFont(ArialMT_Plain_16);
    gfx.drawString(10, 5, "Connecting");
    gfx.drawString(10, 30, "to WiFi...");
    gfx.display();
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
            gfx.setFont(ArialMT_Plain_16);
            Serial.print("IP-Adresse = ");
            Serial.println(WiFi.localIP());
            gfx.clear();
            gfx.drawString(20, 10, "Connected!");
            gfx.drawString((DISPLAY_WIDTH - gfx.getStringWidth(WiFi.localIP().toString())) / 2, 40, WiFi.localIP().toString());
            gfx.display();
            connected = true;
            delay(3000);
        }
    }
    if (!connected)
    {
        gfx.setFont(ArialMT_Plain_24);
        WiFi.mode(WIFI_AP);
        WiFi.softAP("SmartPusher", "", 1);
        gfx.clear();
        gfx.drawString(15, 15, "AP MODE");
        gfx.drawString(10, 30, "192.168.1.4");
        gfx.display();
    }
    return connected;
}

String formatBytes(size_t bytes)
{ // lesbare Anzeige der Speichergrößen
    if (bytes < 1024)
    {
        return String(bytes) + " Byte";
    }
    else if (bytes < (1024 * 1024))
    {
        return String(bytes / 1024.0) + " KB";
    }
    else if (bytes < (1024 * 1024 * 1024))
    {
        return String(bytes / 1024.0 / 1024.0) + " MB";
    }
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

void handleDisplayFS()
{ // HTML Filesystem
    //  Page: /filesystem
    temp = "";
    // HTML Header
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    server.sendHeader("Pragma", "no-cache");
    server.sendHeader("Expires", "-1");
    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    // HTML Content
    server.send(200, "text/html", temp);
    temp += "<!DOCTYPE HTML><html lang='de'><head><meta charset='UTF-8'><meta name= viewport content='width=device-width, initial-scale=1.0,'>";
    server.sendContent(temp);
    temp = "";
    temp += "<style type='text/css'><!-- DIV.container { min-height: 10em; display: table-cell; vertical-align: middle }.button {height:35px; width:90px; font-size:16px}";
    server.sendContent(temp);
    temp = "";
    temp += "body {background-color: powderblue;}</style><head><title>File System Manager</title></head>";
    temp += "<h2>Serial Peripheral Interface Flash Filesystem</h2><body><left>";
    server.sendContent(temp);
    temp = "";
    if (server.args() > 0) // Parameter wurden ubergeben
    {
        if (server.hasArg("delete"))
        {
            String FToDel = server.arg("delete");
            if (SPIFFS.exists(FToDel))
            {
                SPIFFS.remove(FToDel);
                temp += "File " + FToDel + " successfully deleted.";
            }
            else
            {
                temp += "File " + FToDel + " cannot be deleted.";
            }
            server.sendContent(temp);
            temp = "";
        }
        if (server.hasArg("format") and server.arg("on"))
        {
            SPIFFS.format();
            temp += "SPI File System successfully formatted.";
            server.sendContent(temp);
            temp = "";
        } //   server.client().stop(); // Stop is needed because we sent no content length
    }

    temp += "<table border=2 bgcolor = white width = 400 ><td><h4>Current SPIFFS Status: </h4>";
    temp += formatBytes(SPIFFS.usedBytes() * 1.05) + " of " + formatBytes(SPIFFS.totalBytes()) + " used. <br>";
    temp += formatBytes((SPIFFS.totalBytes() - (SPIFFS.usedBytes() * 1.05))) + " free. <br>";
    temp += "</td></table><br>";
    server.sendContent(temp);
    temp = "";
    // Check for Site Parameters

    temp += "<table border=2 bgcolor = white width = 400><tr><th><br>";
    temp += "<h4>Available Files on SPIFFS:</h4><table border=2 bgcolor = white ></tr></th><td>Filename</td><td>Size</td><td>Action </td></tr></th>";
    server.sendContent(temp);
    temp = "";
    File root = SPIFFS.open("/");
    File file = root.openNextFile();
    while (file)
    {
        temp += "<td> <a title=\"Download\" href =\"" + String(file.name()) + "\" download=\"" + String(file.name()) + "\">" + String(file.name()) + "</a> <br></th>";
        temp += "<td>" + formatBytes(file.size()) + "</td>";
        temp += "<td><a href =filesystem?delete=" + String(file.name()) + "> Delete </a></td>";
        temp += "</tr></th>";
        file = root.openNextFile();
    }

    temp += "</tr></th>";
    temp += "</td></tr></th><br></th></tr></table></table><br>";
    temp += "<table border=2 bgcolor = white width = 400 ><td><h4>Upload</h4>";

    temp += "<label> Choose File: </label>";
    temp += "<form method='POST' action='/upload' enctype='multipart/form-data' style='height:35px;'><input type='file' name='upload' style='height:35px; font-size:13px;' required>\r\n<input type='submit' value='Upload' class='button'></form>";
    temp += " </table><br>";
    server.sendContent(temp);
    temp = "";
    temp += "<td><a href =filesystem?format=on> Format SPIFFS Filesystem. (Takes up to 30 Seconds) </a></td>";
    temp += "<table border=2 bgcolor = white width = 500 cellpadding =5 ><caption><p><h3>Systemlinks:</h2></p></caption><tr><th><br>";
    temp += " <a href='/'>Main Page</a><br><br></th></tr></table><br><br>";
    server.sendContent(temp);
    temp = "";
    temp += "<footer><p>Programmed and designed by: Tobias Kuch</p><p>Contact information: <a href='mailto:tobias.kuch@googlemail.com'>tobias.kuch@googlemail.com</a>.</p></footer></body></html>";
    // server.send ( 200, "", temp );
    server.sendContent(temp);
    server.client().stop(); // Stop is needed because we sent no content length
    temp = "";
}

void handleFileUpload()
{ // Dateien ins SPIFFS schreiben
    if (server.uri() != "/upload")
        return;
    HTTPUpload &upload = server.upload();
    if (upload.status == UPLOAD_FILE_START)
    {
        String filename = upload.filename;
        if (upload.filename.length() > 30)
        {
            upload.filename = upload.filename.substring(upload.filename.length() - 30, upload.filename.length()); // Dateinamen auf 30 Zeichen kürzen
        }
        Serial.println("FileUpload Name: " + upload.filename);
        if (!filename.startsWith("/"))
            filename = "/" + filename;
        // fsUploadFile = SPIFFS.open(filename, "w");
        fsUploadFile = SPIFFS.open("/" + server.urlDecode(upload.filename), "w");
        filename = String();
    }
    else if (upload.status == UPLOAD_FILE_WRITE)
    {
        //  Serial.print("handleFileUpload Data: "); Serial.println(upload.currentSize);
        if (fsUploadFile)
            fsUploadFile.write(upload.buf, upload.currentSize);
    }
    else if (upload.status == UPLOAD_FILE_END)
    {
        if (fsUploadFile)
            fsUploadFile.close();

        //  Serial.print("handleFileUpload Size: "); Serial.println(upload.totalSize);
        // server.sendContent(Header);
        handleDisplayFS();
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
    conf.registerOnSave(SettingsSaved);
    conf.setDescription(params);
    conf.readConfig();
    gfx.clear();
    initWiFi();
    Update.onProgress(update_progress);
    server.on("/", handleRoot);
    server.on("/files", HTTP_GET, handleDisplayFS);
    server.on("/update", HTTP_GET, []()
              {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", updateIndex); });
    server.on(
        "/doupdate", HTTP_POST, []()
        {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "NOK" : "OK");
    delay(1000);
    ESP.restart(); },
        []()
        {
            HTTPUpload &upload = server.upload();
            if (upload.status == UPLOAD_FILE_START)
            {
                gfx.setFont(ArialMT_Plain_24);
                ButtonManager.turnAllOff();
                ButtonManager.tick();
                Serial.setDebugOutput(true);
                Serial.printf("Update: %s\n", upload.filename.c_str());
                uint32_t maxSketchSpace = (1048576 - 0x1000) & 0xFFFFF000;
                gfx.clear();
                gfx.drawString(15, 25, "UPDATE");

                gfx.display();

                if (!Update.begin(maxSketchSpace))
                { // start with max available size
                    Update.printError(Serial);
                }
            }
            else if (upload.status == UPLOAD_FILE_WRITE)
            {

                if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)
                {
                    Update.printError(Serial);
                }
            }
            else if (upload.status == UPLOAD_FILE_END)
            {
                if (Update.end(true))
                { // true to set the size to the current progress
                    Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
                }
                else
                {
                    Update.printError(Serial);
                }
                Serial.setDebugOutput(false);
            }
            yield();
        });
    server.begin(80);

    if (connected)
    {
        char dns[30];
        sprintf(dns, "%s.local", conf.getApName());
        if (MDNS.begin(dns))
        {
            Serial.println("MDNS responder gestartet");
        }

        configTzTime(conf.getString("tz").c_str(), conf.getString("ntp").c_str());
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
    gfx.setContrast(val);
    if (val == 0)
    {
        gfx.displayOff();
    }
    else
    {
        gfx.displayOn();
    };

    ButtonManager.setBrightness(val);
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

void SystemManager_::ShowMessage(String msg)
{
    Message = msg;
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
    uint16_t firstline = gfx.drawStringMaxWidth(0, 0, 128, Message.substring(start_at));
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
    File f = SPIFFS.open("/" + Image + ".mono", "r");
    if (f)
    {
        int s = f.size();
        Serial.printf("File Opened , Size=%d\r\n", s);
        String data = f.readString();
        uint8_t data1[data.length()];
        data.getBytes(data1, data.length());
        f.close();
        gfx.drawXbm(0, 0, 128, 64, data1);
        gfx.display();
    }
    else
    {
        Serial.println("File Not Opened");
    }

    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= CLOCK_INTERVAL)
    {
        previousMillis = currentMillis;
        screen = 0;
    }
}

void SystemManager_::renderClockScreen()
{
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= CLOCK_INTERVAL)
    {
        getLocalTime(&timeinfo);
        gfx.clear();

        previousMillis = currentMillis;
        weekDay = weekDays[timeinfo.tm_wday];

        if (conf.getBool("colonblink"))
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
