/*
    The code in this file uses open source libraries provided by thecoderscorner

    DO NOT EDIT THIS FILE, IT WILL BE GENERATED EVERY TIME YOU USE THE UI DESIGNER
    INSTEAD EITHER PUT CODE IN YOUR SKETCH OR CREATE ANOTHER SOURCE FILE.

    All the variables you may need access to are marked extern in this file for easy
    use elsewhere.
 */

#include <tcMenu.h>
#include "TCMenu_menu.h"
#include "ThemeMonoBordered.h"

// Global variable declarations
const PROGMEM  ConnectorLocalInfo applicationInfo = { "APBA", "2f01b637-3a3d-4f52-b7d6-f202ba4f0747" };
TcMenuRemoteServer remoteServer(applicationInfo);
ArduinoEEPROMAbstraction glArduinoEeprom(&EEPROM);
U8G2_SSD1306_128X64_NONAME_F_HW_I2C gfx(U8G2_R0, -1, -1, -1);
U8g2Drawable gfxDrawable(&gfx);
GraphicsDeviceRenderer renderer(30, applicationInfo.name, &gfxDrawable);
EspAsyncWebserver webServer("/ws", 80);
EspWebSocketRemoteConnection webClient1(webServer);
EspWebSocketRemoteConnection webClient2(webServer);

// Global Menu Item declarations
const PROGMEM AnyMenuInfo minfoExit = { "Exit", 84, 0xffff, 0, onExit };
ActionMenuItem menuExit(&minfoExit, NULL);
const PROGMEM AnyMenuInfo minfoSaveAll = { "Save all", 83, 0xffff, 0, onSaveAll };
ActionMenuItem menuSaveAll(&minfoSaveAll, &menuExit);
const PROGMEM AnyMenuInfo minfoClearSettings = { "Clear Settings", 81, 0xffff, 0, onClearSettings };
ActionMenuItem menuClearSettings(&minfoClearSettings, &menuSaveAll);
const PROGMEM BooleanMenuInfo minfobtn8Up = { "Up", 76, 203, 1, NO_CALLBACK, NAMING_YES_NO };
BooleanMenuItem menubtn8Up(&minfobtn8Up, false, NULL);
const PROGMEM BooleanMenuInfo minfobtn8Down = { "Down", 75, 202, 1, NO_CALLBACK, NAMING_YES_NO };
BooleanMenuItem menubtn8Down(&minfobtn8Down, false, &menubtn8Up);
const PROGMEM BooleanMenuInfo minfobtn8DoubleClick = { "Doubleclick", 74, 201, 1, NO_CALLBACK, NAMING_YES_NO };
BooleanMenuItem menubtn8DoubleClick(&minfobtn8DoubleClick, false, &menubtn8Down);
const PROGMEM BooleanMenuInfo minfobtn8LongClick = { "Long click", 73, 200, 1, NO_CALLBACK, NAMING_YES_NO };
BooleanMenuItem menubtn8LongClick(&minfobtn8LongClick, false, &menubtn8DoubleClick);
const PROGMEM BooleanMenuInfo minfobtn8Click = { "Click", 72, 199, 1, NO_CALLBACK, NAMING_YES_NO };
BooleanMenuItem menubtn8Click(&minfobtn8Click, false, &menubtn8LongClick);
const char enumStrLightBtn8_0[] PROGMEM = "static off";
const char enumStrLightBtn8_1[] PROGMEM = "static on";
const char enumStrLightBtn8_2[] PROGMEM = "breathe";
const char enumStrLightBtn8_3[] PROGMEM = "extern";
const char* const enumStrLightBtn8[] PROGMEM  = { enumStrLightBtn8_0, enumStrLightBtn8_1, enumStrLightBtn8_2, enumStrLightBtn8_3 };
const PROGMEM EnumMenuInfo minfoLightBtn8 = { "Light", 92, 318, 3, Light, enumStrLightBtn8 };
EnumMenuItem menuLightBtn8(&minfoLightBtn8, 0, &menubtn8Click);
RENDERING_CALLBACK_NAME_INVOKE(fnButton8RtCall, backSubItemRenderFn, "Button 8", -1, NO_CALLBACK)
const PROGMEM SubMenuInfo minfoButton8 = { "Button 8", 71, 0xffff, 0, NO_CALLBACK };
BackMenuItem menuBackButton8(fnButton8RtCall, &menuLightBtn8);
SubMenuItem menuButton8(&minfoButton8, &menuBackButton8, NULL);
const PROGMEM BooleanMenuInfo minfobtn7Up = { "Up", 70, 198, 1, NO_CALLBACK, NAMING_YES_NO };
BooleanMenuItem menubtn7Up(&minfobtn7Up, false, NULL);
const PROGMEM BooleanMenuInfo minfobtn7Down = { "Down", 69, 197, 1, NO_CALLBACK, NAMING_YES_NO };
BooleanMenuItem menubtn7Down(&minfobtn7Down, false, &menubtn7Up);
const PROGMEM BooleanMenuInfo minfobtn7DoubleClick = { "Doubleclick", 68, 196, 1, NO_CALLBACK, NAMING_YES_NO };
BooleanMenuItem menubtn7DoubleClick(&minfobtn7DoubleClick, false, &menubtn7Down);
const PROGMEM BooleanMenuInfo minfobtn7LongClick = { "Long click", 67, 195, 1, NO_CALLBACK, NAMING_YES_NO };
BooleanMenuItem menubtn7LongClick(&minfobtn7LongClick, false, &menubtn7DoubleClick);
const PROGMEM BooleanMenuInfo minfobtn7Click = { "Click", 66, 194, 1, NO_CALLBACK, NAMING_YES_NO };
BooleanMenuItem menubtn7Click(&minfobtn7Click, false, &menubtn7LongClick);
const char enumStrLightBtn7_0[] PROGMEM = "static off";
const char enumStrLightBtn7_1[] PROGMEM = "static on";
const char enumStrLightBtn7_2[] PROGMEM = "breathe";
const char enumStrLightBtn7_3[] PROGMEM = "extern";
const char* const enumStrLightBtn7[] PROGMEM  = { enumStrLightBtn7_0, enumStrLightBtn7_1, enumStrLightBtn7_2, enumStrLightBtn7_3 };
const PROGMEM EnumMenuInfo minfoLightBtn7 = { "Light", 91, 316, 3, Light, enumStrLightBtn7 };
EnumMenuItem menuLightBtn7(&minfoLightBtn7, 0, &menubtn7Click);
RENDERING_CALLBACK_NAME_INVOKE(fnButton7RtCall, backSubItemRenderFn, "Button 7", -1, NO_CALLBACK)
const PROGMEM SubMenuInfo minfoButton7 = { "Button 7", 65, 0xffff, 0, NO_CALLBACK };
BackMenuItem menuBackButton7(fnButton7RtCall, &menuLightBtn7);
SubMenuItem menuButton7(&minfoButton7, &menuBackButton7, &menuButton8);
const PROGMEM BooleanMenuInfo minfobtn6Up = { "Up", 64, 193, 1, NO_CALLBACK, NAMING_YES_NO };
BooleanMenuItem menubtn6Up(&minfobtn6Up, false, NULL);
const PROGMEM BooleanMenuInfo minfobtn6Down = { "Down", 63, 192, 1, NO_CALLBACK, NAMING_YES_NO };
BooleanMenuItem menubtn6Down(&minfobtn6Down, false, &menubtn6Up);
const PROGMEM BooleanMenuInfo minfobtn6DoubleClick = { "Doubleclick", 62, 191, 1, NO_CALLBACK, NAMING_YES_NO };
BooleanMenuItem menubtn6DoubleClick(&minfobtn6DoubleClick, false, &menubtn6Down);
const PROGMEM BooleanMenuInfo minfobtn6LongClick = { "Long click", 61, 190, 1, NO_CALLBACK, NAMING_YES_NO };
BooleanMenuItem menubtn6LongClick(&minfobtn6LongClick, false, &menubtn6DoubleClick);
const PROGMEM BooleanMenuInfo minfobtn6Click = { "Click", 60, 189, 1, NO_CALLBACK, NAMING_YES_NO };
BooleanMenuItem menubtn6Click(&minfobtn6Click, false, &menubtn6LongClick);
const char enumStrLightBtn6_0[] PROGMEM = "statuc off";
const char enumStrLightBtn6_1[] PROGMEM = "static on";
const char enumStrLightBtn6_2[] PROGMEM = "breathe";
const char enumStrLightBtn6_3[] PROGMEM = "extern";
const char* const enumStrLightBtn6[] PROGMEM  = { enumStrLightBtn6_0, enumStrLightBtn6_1, enumStrLightBtn6_2, enumStrLightBtn6_3 };
const PROGMEM EnumMenuInfo minfoLightBtn6 = { "Light", 90, 314, 3, Light, enumStrLightBtn6 };
EnumMenuItem menuLightBtn6(&minfoLightBtn6, 0, &menubtn6Click);
RENDERING_CALLBACK_NAME_INVOKE(fnButton6RtCall, backSubItemRenderFn, "Button 6", -1, NO_CALLBACK)
const PROGMEM SubMenuInfo minfoButton6 = { "Button 6", 59, 0xffff, 0, NO_CALLBACK };
BackMenuItem menuBackButton6(fnButton6RtCall, &menuLightBtn6);
SubMenuItem menuButton6(&minfoButton6, &menuBackButton6, &menuButton7);
const PROGMEM BooleanMenuInfo minfobtn5Up = { "Up", 58, 188, 1, NO_CALLBACK, NAMING_YES_NO };
BooleanMenuItem menubtn5Up(&minfobtn5Up, false, NULL);
const PROGMEM BooleanMenuInfo minfobtn5Down = { "Down", 57, 187, 1, NO_CALLBACK, NAMING_YES_NO };
BooleanMenuItem menubtn5Down(&minfobtn5Down, false, &menubtn5Up);
const PROGMEM BooleanMenuInfo minfobtn5DoubleClick = { "Doubleclick", 56, 186, 1, NO_CALLBACK, NAMING_YES_NO };
BooleanMenuItem menubtn5DoubleClick(&minfobtn5DoubleClick, false, &menubtn5Down);
const PROGMEM BooleanMenuInfo minfobtn5LongClick = { "Long click", 55, 185, 1, NO_CALLBACK, NAMING_YES_NO };
BooleanMenuItem menubtn5LongClick(&minfobtn5LongClick, false, &menubtn5DoubleClick);
const PROGMEM BooleanMenuInfo minfobtn5Click = { "Click", 54, 184, 1, NO_CALLBACK, NAMING_YES_NO };
BooleanMenuItem menubtn5Click(&minfobtn5Click, false, &menubtn5LongClick);
const char enumStrLightBtn5_0[] PROGMEM = "static off";
const char enumStrLightBtn5_1[] PROGMEM = "static on";
const char enumStrLightBtn5_2[] PROGMEM = "breathe";
const char enumStrLightBtn5_3[] PROGMEM = "extern";
const char* const enumStrLightBtn5[] PROGMEM  = { enumStrLightBtn5_0, enumStrLightBtn5_1, enumStrLightBtn5_2, enumStrLightBtn5_3 };
const PROGMEM EnumMenuInfo minfoLightBtn5 = { "Light", 89, 312, 3, Light, enumStrLightBtn5 };
EnumMenuItem menuLightBtn5(&minfoLightBtn5, 0, &menubtn5Click);
RENDERING_CALLBACK_NAME_INVOKE(fnButton5RtCall, backSubItemRenderFn, "Button 5", -1, NO_CALLBACK)
const PROGMEM SubMenuInfo minfoButton5 = { "Button 5", 53, 0xffff, 0, NO_CALLBACK };
BackMenuItem menuBackButton5(fnButton5RtCall, &menuLightBtn5);
SubMenuItem menuButton5(&minfoButton5, &menuBackButton5, &menuButton6);
const PROGMEM BooleanMenuInfo minfobtn4Up = { "Up", 52, 183, 1, NO_CALLBACK, NAMING_YES_NO };
BooleanMenuItem menubtn4Up(&minfobtn4Up, false, NULL);
const PROGMEM BooleanMenuInfo minfobtn4Down = { "Down", 51, 182, 1, NO_CALLBACK, NAMING_YES_NO };
BooleanMenuItem menubtn4Down(&minfobtn4Down, false, &menubtn4Up);
const PROGMEM BooleanMenuInfo minfobtn4DoubleClick = { "Doubleclick", 50, 181, 1, NO_CALLBACK, NAMING_YES_NO };
BooleanMenuItem menubtn4DoubleClick(&minfobtn4DoubleClick, false, &menubtn4Down);
const PROGMEM BooleanMenuInfo minfobtn4LongClick = { "Long click", 49, 180, 1, NO_CALLBACK, NAMING_YES_NO };
BooleanMenuItem menubtn4LongClick(&minfobtn4LongClick, false, &menubtn4DoubleClick);
const PROGMEM BooleanMenuInfo minfobtn4Click = { "Click", 48, 179, 1, NO_CALLBACK, NAMING_YES_NO };
BooleanMenuItem menubtn4Click(&minfobtn4Click, false, &menubtn4LongClick);
const char enumStrLightBtn4_0[] PROGMEM = "static off";
const char enumStrLightBtn4_1[] PROGMEM = "static on";
const char enumStrLightBtn4_2[] PROGMEM = "breathe";
const char enumStrLightBtn4_3[] PROGMEM = "extern";
const char* const enumStrLightBtn4[] PROGMEM  = { enumStrLightBtn4_0, enumStrLightBtn4_1, enumStrLightBtn4_2, enumStrLightBtn4_3 };
const PROGMEM EnumMenuInfo minfoLightBtn4 = { "Light", 88, 310, 3, Light, enumStrLightBtn4 };
EnumMenuItem menuLightBtn4(&minfoLightBtn4, 0, &menubtn4Click);
RENDERING_CALLBACK_NAME_INVOKE(fnButton4RtCall, backSubItemRenderFn, "Button 4", -1, NO_CALLBACK)
const PROGMEM SubMenuInfo minfoButton4 = { "Button 4", 47, 0xffff, 0, NO_CALLBACK };
BackMenuItem menuBackButton4(fnButton4RtCall, &menuLightBtn4);
SubMenuItem menuButton4(&minfoButton4, &menuBackButton4, &menuButton5);
const PROGMEM BooleanMenuInfo minfobtn3Up = { "Up", 46, 178, 1, NO_CALLBACK, NAMING_YES_NO };
BooleanMenuItem menubtn3Up(&minfobtn3Up, false, NULL);
const PROGMEM BooleanMenuInfo minfobtn3Down = { "Down", 45, 177, 1, NO_CALLBACK, NAMING_YES_NO };
BooleanMenuItem menubtn3Down(&minfobtn3Down, false, &menubtn3Up);
const PROGMEM BooleanMenuInfo minfobtn3DoubleClick = { "Doubleclick", 44, 176, 1, NO_CALLBACK, NAMING_YES_NO };
BooleanMenuItem menubtn3DoubleClick(&minfobtn3DoubleClick, false, &menubtn3Down);
const PROGMEM BooleanMenuInfo minfobtn3LongClick = { "Long click", 43, 175, 1, NO_CALLBACK, NAMING_YES_NO };
BooleanMenuItem menubtn3LongClick(&minfobtn3LongClick, false, &menubtn3DoubleClick);
const PROGMEM BooleanMenuInfo minfobtn3Click = { "Click", 42, 174, 1, NO_CALLBACK, NAMING_YES_NO };
BooleanMenuItem menubtn3Click(&minfobtn3Click, false, &menubtn3LongClick);
const char enumStrLightBtn3_0[] PROGMEM = "static off";
const char enumStrLightBtn3_1[] PROGMEM = "static on";
const char enumStrLightBtn3_2[] PROGMEM = "breathe";
const char enumStrLightBtn3_3[] PROGMEM = "extern";
const char* const enumStrLightBtn3[] PROGMEM  = { enumStrLightBtn3_0, enumStrLightBtn3_1, enumStrLightBtn3_2, enumStrLightBtn3_3 };
const PROGMEM EnumMenuInfo minfoLightBtn3 = { "Light", 87, 308, 3, Light, enumStrLightBtn3 };
EnumMenuItem menuLightBtn3(&minfoLightBtn3, 0, &menubtn3Click);
RENDERING_CALLBACK_NAME_INVOKE(fnButton3RtCall, backSubItemRenderFn, "Button 3", -1, NO_CALLBACK)
const PROGMEM SubMenuInfo minfoButton3 = { "Button 3", 41, 0xffff, 0, NO_CALLBACK };
BackMenuItem menuBackButton3(fnButton3RtCall, &menuLightBtn3);
SubMenuItem menuButton3(&minfoButton3, &menuBackButton3, &menuButton4);
const PROGMEM BooleanMenuInfo minfobtn2Up = { "Up", 40, 107, 1, NO_CALLBACK, NAMING_YES_NO };
BooleanMenuItem menubtn2Up(&minfobtn2Up, false, NULL);
const PROGMEM BooleanMenuInfo minfobtn2Down = { "Down", 39, 106, 1, NO_CALLBACK, NAMING_YES_NO };
BooleanMenuItem menubtn2Down(&minfobtn2Down, false, &menubtn2Up);
const PROGMEM BooleanMenuInfo minfobtn2DoubleClick = { "Doubleclick", 38, 105, 1, NO_CALLBACK, NAMING_YES_NO };
BooleanMenuItem menubtn2DoubleClick(&minfobtn2DoubleClick, false, &menubtn2Down);
const PROGMEM BooleanMenuInfo minfobtn2LongClick = { "Long click", 37, 104, 1, NO_CALLBACK, NAMING_YES_NO };
BooleanMenuItem menubtn2LongClick(&minfobtn2LongClick, false, &menubtn2DoubleClick);
const PROGMEM BooleanMenuInfo minfobtn2Click = { "Click", 36, 103, 1, NO_CALLBACK, NAMING_YES_NO };
BooleanMenuItem menubtn2Click(&minfobtn2Click, false, &menubtn2LongClick);
const char enumStrLightBtn2_0[] PROGMEM = "static off";
const char enumStrLightBtn2_1[] PROGMEM = "static on";
const char enumStrLightBtn2_2[] PROGMEM = "breathe";
const char enumStrLightBtn2_3[] PROGMEM = "extern";
const char* const enumStrLightBtn2[] PROGMEM  = { enumStrLightBtn2_0, enumStrLightBtn2_1, enumStrLightBtn2_2, enumStrLightBtn2_3 };
const PROGMEM EnumMenuInfo minfoLightBtn2 = { "Light", 86, 306, 3, Light, enumStrLightBtn2 };
EnumMenuItem menuLightBtn2(&minfoLightBtn2, 0, &menubtn2Click);
RENDERING_CALLBACK_NAME_INVOKE(fnButton2RtCall, backSubItemRenderFn, "Button 2", -1, NO_CALLBACK)
const PROGMEM SubMenuInfo minfoButton2 = { "Button 2", 35, 0xffff, 0, NO_CALLBACK };
BackMenuItem menuBackButton2(fnButton2RtCall, &menuLightBtn2);
SubMenuItem menuButton2(&minfoButton2, &menuBackButton2, &menuButton3);
const PROGMEM BooleanMenuInfo minfobtn1Up = { "Up", 34, 173, 1, NO_CALLBACK, NAMING_YES_NO };
BooleanMenuItem menubtn1Up(&minfobtn1Up, false, NULL);
const PROGMEM BooleanMenuInfo minfobtn1Down = { "Down", 33, 172, 1, NO_CALLBACK, NAMING_YES_NO };
BooleanMenuItem menubtn1Down(&minfobtn1Down, false, &menubtn1Up);
const PROGMEM BooleanMenuInfo minfobtn1DoubleClick = { "Doubleclick", 32, 170, 1, NO_CALLBACK, NAMING_YES_NO };
BooleanMenuItem menubtn1DoubleClick(&minfobtn1DoubleClick, false, &menubtn1Down);
const PROGMEM BooleanMenuInfo minfobtn1LongClick = { "Long click", 31, 171, 1, NO_CALLBACK, NAMING_YES_NO };
BooleanMenuItem menubtn1LongClick(&minfobtn1LongClick, false, &menubtn1DoubleClick);
const PROGMEM BooleanMenuInfo minfobtn1Click = { "Click", 30, 169, 1, NO_CALLBACK, NAMING_YES_NO };
BooleanMenuItem menubtn1Click(&minfobtn1Click, false, &menubtn1LongClick);
const char enumStrLightBtn1_0[] PROGMEM = "static off";
const char enumStrLightBtn1_1[] PROGMEM = "static on";
const char enumStrLightBtn1_2[] PROGMEM = "breathe";
const char enumStrLightBtn1_3[] PROGMEM = "extern";
const char* const enumStrLightBtn1[] PROGMEM  = { enumStrLightBtn1_0, enumStrLightBtn1_1, enumStrLightBtn1_2, enumStrLightBtn1_3 };
const PROGMEM EnumMenuInfo minfoLightBtn1 = { "Light", 85, 304, 3, Light, enumStrLightBtn1 };
EnumMenuItem menuLightBtn1(&minfoLightBtn1, 0, &menubtn1Click);
RENDERING_CALLBACK_NAME_INVOKE(fnButton1RtCall, backSubItemRenderFn, "Button 1", -1, NO_CALLBACK)
const PROGMEM SubMenuInfo minfoButton1 = { "Button 1", 14, 0xffff, 0, NO_CALLBACK };
BackMenuItem menuBackButton1(fnButton1RtCall, &menuLightBtn1);
SubMenuItem menuButton1(&minfoButton1, &menuBackButton1, &menuButton2);
RENDERING_CALLBACK_NAME_INVOKE(fnButtonsRtCall, backSubItemRenderFn, "Buttons", -1, NO_CALLBACK)
const PROGMEM SubMenuInfo minfoButtons = { "Buttons", 1, 0xffff, 0, NO_CALLBACK };
BackMenuItem menuBackButtons(fnButtonsRtCall, &menuButton1);
SubMenuItem menuButtons(&minfoButtons, &menuBackButtons, &menuClearSettings);

void setupMenu() {
    // First we set up eeprom and authentication (if needed).
    menuMgr.setEepromRef(&glArduinoEeprom);
    // Now add any readonly, non-remote and visible flags.
    menuExit.setLocalOnly(true);

    // Code generated by plugins.
    gfx.begin();
    renderer.setUpdatesPerSecond(10);
    switches.initialise(internalDigitalIo(), true);
    menuMgr.initForUpDownOk(&renderer, &menuButtons, 25, 32, 27);
    remoteServer.addConnection(&webClient1);
    remoteServer.addConnection(&webClient2);
    renderer.setTitleMode(BaseGraphicalRenderer::TITLE_FIRST_ROW);
    renderer.setUseSliderForAnalog(false);
    installMonoBorderedTheme(renderer, MenuFontDef(nullptr, 1), MenuFontDef(nullptr, 1), true);
}

