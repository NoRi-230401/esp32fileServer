// *******************************************************
//  esp32fileServer          by NoRi 2025-07-29
// -------------------------------------------------------
// main.cpp
// *******************************************************
#include "fileServer/fileServer.h"

//-------------------------------------------
// vsCode terminal cannot get serial data before 5 sec...!
#define DEBUG_PLATFORMIO

const String PROG_NAME = "esp32fileServer";
const String VERSION = "v1.04";
const String GITHUB_URL = "https://github.com/NoRi-230401/esp32fileServer";

//--------------------
// ***  SETTINGS  ***
//--------------------
// #define M5STACK_DEVICE       // for M5stack Core2,CoreS3,Cardputer
//---------------------------------------------------------------------------
const String WIFI_TXT = "/wifi.txt";
const String YOUR_SSID = "";
const String YOUR_SSID_PASS = "";
const String YOUR_HOST_NAME = "esp32fileServer";
#ifdef M5STACK_DEVICE
bool RTC_ADJUST_ON = true; // 'false' if don't adjust RTC
#else
bool RTC_ADJUST_ON = false; // 'false' if don't adjust RTC
#endif
//---------------------------------------------------------------------------
#ifdef FILES_LITTLEFS
const bool LittleFS_USE = true; // (default) LittleFS instead of SPIFFS
const bool SPIFFS_USE = false;
#else
const bool LittleFS_USE = false; // (default) LittleFS instead of SPIFFS
const bool SPIFFS_USE = true;
#endif
#ifdef FILES_SD
const bool SD_USE = true;
#else
const bool SD_USE = false;
#endif

void setup()
{
#ifdef M5STACK_DEVICE
  m5stack_begin();
#ifdef DEBUG_PLATFORMIO
  delay(5000);
#endif

  if (SD_ENABLE)
  { // M5stack-SD-Updater lobby
    SDU_lobby();
    SD.end();
  }
  if (SD_USE)
    SD_start();
#else
  Serial.begin(115200);
#ifdef DEBUG_PLATFORMIO
  delay(5000);
#endif

#endif

  prtln("- " + PROG_NAME + " -");

  if (LittleFS_USE)
    LittleFS_start();
  else if (SPIFFS_USE)
    SPIFFS_start();

  if (wifiStart())
  {
    IP_ADDR = WiFi.localIP().toString();
    SSID = WiFi.SSID();
    prtln("\n*** Connected ***");
    prtln("IP_ADDR = " + IP_ADDR);
    prtln("SSID = " + SSID);
    prtln("WiFi    .....  OK");
  }
  else
  {
    prtln("WiFi    .....  NG");
    STOP();
  }

  if (!setupServer())
    STOP();

  prtln("*** setup() done! ***");
}

void loop()
{
  requestManage();
  vTaskDelay(1);
}
