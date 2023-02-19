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
#include <ArduinoJson.h>

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
String Message;
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
                                        "'label':'LED Mode',"
                                        "'type':" +
                String(INPUTSELECT) + ","
                                      "'options':["
                                      "{'v':'0','l':'Off'},"
                                      "{'v':'1','l':'On'},"
                                      "{'v':'2','l':'Fade'},"
                                      "{'v':'3','l':'Extern'},"
                                      "{'v':'4','l':'OnPush'}],"
                                      "'default':'1'"
                                      "}"
                                      "]";

WebServer server;
WebConfig conf;

void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
}

void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info)
{
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
    Serial.println("Disconnected from WiFi access point");
    Serial.print("WiFi lost connection. Reason: ");
    Serial.println("Trying to Reconnect");
    WiFi.reconnect();
}

boolean initWiFi()
{

    WiFi.mode(WIFI_STA);
    WiFi.onEvent(WiFiStationConnected, WiFiEvent_t::SYSTEM_EVENT_STA_CONNECTED);
    WiFi.onEvent(WiFiGotIP, WiFiEvent_t::SYSTEM_EVENT_STA_GOT_IP);
    WiFi.onEvent(WiFiStationDisconnected, WiFiEvent_t::SYSTEM_EVENT_STA_DISCONNECTED);
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
        gfx.setFont(ArialMT_Plain_16);
        WiFi.mode(WIFI_AP);
        WiFi.softAP("SmartPusher", "", 1);
        gfx.clear();
        gfx.drawString(25, 15, "AP MODE");
        gfx.drawString(20, 35, "192.168.4.1");
        gfx.display();
    }
    return connected;
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

#ifndef Web

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

String getContentType(String filename)
{ // convert the file extension to the MIME type
    if (filename.endsWith(".htm"))
        return "text/html";
    else if (filename.endsWith(".css"))
        return "text/css";
    else if (filename.endsWith(".js"))
        return "application/javascript";
    else if (filename.endsWith(".ico"))
        return "image/x-icon";
    else if (filename.endsWith(".gz"))
        return "application/x-gzip";
    else if (filename.endsWith(".bmp"))
        return "image/bmp";
    else if (filename.endsWith(".tif"))
        return "image/tiff";
    else if (filename.endsWith(".pbm"))
        return "image/x-portable-bitmap";
    else if (filename.endsWith(".jpg"))
        return "image/jpeg";
    else if (filename.endsWith(".gif"))
        return "image/gif";
    else if (filename.endsWith(".png"))
        return "image/png";
    else if (filename.endsWith(".svg"))
        return "image/svg+xml";
    else if (filename.endsWith(".html"))
        return "text/html";
    else if (filename.endsWith(".wav"))
        return "audio/x-wav";
    else if (filename.endsWith(".zip"))
        return "application/zip";
    else if (filename.endsWith(".rgb"))
        return "image/x-rg";
    else if (filename.endsWith(".bin"))
        return "application/octet-stream";
    // Complete List on https://wiki.selfhtml.org/wiki/MIME-Type/Übersicht
    return "text/plain";
}

bool handleFileRead(String path)
{ // send the right file to the client (if it exists)
    Serial.println("handleFileRead: " + path);
    if (path.endsWith("/"))
        path += "index.html";                  // If a folder is requested, send the index file
    String contentType = getContentType(path); // Get the MIME type
    String pathWithGz = path + ".gz";
    if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path))
    {                                                       // If the file exists, either as a compressed archive, or normal
        if (SPIFFS.exists(pathWithGz))                      // If there's a compressed version available
            path += ".gz";                                  // Use the compressed verion
        File file = SPIFFS.open(path, "r");                 // Open the file
        size_t sent = server.streamFile(file, contentType); // Send it to the client
        file.close();                                       // Close the file again
        return true;
    }
    return false;
}

void handleNotFound()
{
    if (!handleFileRead(server.uri()))
    {
        temp = "";
        // HTML Header
        server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
        server.sendHeader("Pragma", "no-cache");
        server.sendHeader("Expires", "-1");
        server.setContentLength(CONTENT_LENGTH_UNKNOWN);
        // HTML Content
        temp += "<!DOCTYPE HTML><html lang='de'><head><meta charset='UTF-8'><meta name= viewport content='width=device-width, initial-scale=1.0,'>";
        temp += "<style type='text/css'><!-- DIV.container { min-height: 10em; display: table-cell; vertical-align: middle }.button {height:35px; width:90px; font-size:16px}";
        temp += "body {background-color: powderblue;}</style>";
        temp += "<head><title>File not found</title></head>";
        temp += "<h2> 404 File Not Found</h2><br>";
        temp += "<h4>Debug Information:</h4><br>";
        temp += "<body>";
        temp += "URI: ";
        temp += server.uri();
        temp += "\nMethod: ";
        temp += (server.method() == HTTP_GET) ? "GET" : "POST";
        temp += "<br>Arguments: ";
        temp += server.args();
        temp += "\n";
        for (uint8_t i = 0; i < server.args(); i++)
        {
            temp += " " + server.argName(i) + ": " + server.arg(i) + "\n";
        }
        temp += "<br>Server Hostheader: " + server.hostHeader();
        for (uint8_t i = 0; i < server.headers(); i++)
        {
            temp += " " + server.headerName(i) + ": " + server.header(i) + "\n<br>";
        }
        temp += "</table></form><br><br><table border=2 bgcolor = white width = 500 cellpadding =5 ><caption><p><h2>You may want to browse to:</h2></p></caption>";
        temp += "<tr><th>";
        temp += "<a href='/'>Main Page</a><br>";
        temp += "<a href='/wifi'>WIFI Settings</a><br>";
        temp += "<a href='/filesystem'>Filemanager</a><br>";
        temp += "</th></tr></table><br><br>";
        temp += "<footer><p>Programmed and designed by: Tobias Kuch</p><p>Contact information: <a href='mailto:tobias.kuch@googlemail.com'>tobias.kuch@googlemail.com</a>.</p></footer>";
        temp += "</body></html>";
        server.send(404, "", temp);
        server.client().stop(); // Stop is needed because we sent no content length
        temp = "";
    }
}

void handleRoot()
{
    conf.handleFormRequest(&server);
    if (server.hasArg("SAVE"))
    {
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
    temp += "body {background-color: black; Color: #fff;}</style><head><title>File System Manager</title></head>";
    temp += "<h2>File Uploader</h2><body><left>";
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

    temp += formatBytes(SPIFFS.usedBytes() * 1.05) + " of " + formatBytes(SPIFFS.totalBytes()) + " used. <br>";
    temp += formatBytes((SPIFFS.totalBytes() - (SPIFFS.usedBytes() * 1.05))) + " free. <br>";
    temp += "<br>";
    server.sendContent(temp);
    temp = "";
    // Check for Site Parameters
    temp += "<h4>Available Files on SPIFFS:</h4><table border=2 bgcolor = black ></tr></th><td>Filename</td><td>Size</td><td>Action </td></tr></th>";
    server.sendContent(temp);
    temp = "";
    File root = SPIFFS.open("/");
    File file = root.openNextFile();
    while (file)
    {
        temp += "<td> <a title=\"Download\" href =\"" + String(file.name()) + "\" download=\"" + String(file.name()) + "\">" + String(file.name()) + "</a> <br></th>";
        temp += "<td>" + formatBytes(file.size()) + "</td>";
        temp += "<td><a href =files?delete=" + String(file.name()) + "> Delete </a></td>";
        temp += "</tr></th>";
        file = root.openNextFile();
    }

    temp += "</tr></th>";
    temp += "</td></tr></th><br></th></tr></table><br>";
    temp += "<h4> Choose File: </h4>";
    temp += "<form method='POST' action='/upload' enctype='multipart/form-data' style='height:35px;'><input type='file' name='upload' style='height:35px; font-size:13px;' required>\r\n<input type='submit' value='Upload' class='button'></form>";
    temp += " <br>";
    server.sendContent(temp);
    temp = "";
    temp += "<br>";
    temp += " <a href='/'>Back</a><br><br><br><br>";
    server.sendContent(temp);
    temp = "";
    temp += "</body></html>";
    // server.send ( 200, "", temp );
    server.sendContent(temp);
    server.client().stop(); // Stop is needed because we sent no content length
    temp = "";
}

void handleFileUpload()
{ // Dateien ins SPIFFS schreiben
    Serial.println("FileUpload Name:");
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
#endif // end Webregion

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
    server.onNotFound(handleNotFound);
    server.on("/", handleRoot);
    server.on(
        "/upload", HTTP_POST, []()
        { server.send(200, "text/plain", ""); },
        handleFileUpload);
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
        sprintf(dns, "%s.local", conf.getString("mqttprefix"));
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
