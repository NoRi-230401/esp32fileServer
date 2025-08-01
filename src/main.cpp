// *******************************************************
//  esp32fileServer          by NoRi 2025-08-01
// -------------------------------------------------------
// main.cpp
// *******************************************************
#include "fileServer/fileServer.h"
//---------------------------------------------------------------------------
const String PROG_NAME = "esp32fileServer";
const String VERSION = "v1.05";
const String GITHUB_URL = "https://github.com/NoRi-230401/esp32fileServer";
const String WIFI_TXT = "/wifi.txt";

#ifdef FILES_LITTLEFS
const bool LittleFS_USE = true; // (default) LittleFS instead of SPIFFS
const bool SPIFFS_USE = false;
#else
const bool LittleFS_USE = false;
const bool SPIFFS_USE = true;
#endif

#ifdef FILES_SD
const bool SD_USE = true;
#else
const bool SD_USE = false;
#endif

#ifdef RTC_BUILT_IN
bool RTC_ADJUST_ON = true;
#else
bool RTC_ADJUST_ON = false;
#endif
//---------------------------------------------------------------------------

// ***  SETTINGS  ***
const String YOUR_SSID = "";
const String YOUR_SSID_PASS = "";
const String YOUR_HOST_NAME = "esp32fileServer";
//---------------------------------------------------------------------------
// vsCode terminal cannot get serial data before 5 sec...!
// #define DEBUG_PLATFORMIO

void setup()
{
#ifdef M5STACK_DEVICE
  m5stack_begin();
  if (SD_ENABLE)
    SDU_lobby();
  prtln("- " + PROG_NAME + " -");
  if (SD_USE)
    SD_start();
#else
  Serial.begin(115200);
#ifdef DEBUG_PLATFORMIO
  delay(5000);
#endif
  prtln("- " + PROG_NAME + " -");
#endif

  if (LittleFS_USE)
    LittleFS_start();
  else if (SPIFFS_USE)
    SPIFFS_start();

  if (wifiStart())
  {
    IP_ADDR = WiFi.localIP().toString();
    SSID = WiFi.SSID();
    prt("\n");
    dbPrtln("*** Connected ***");
    dbPrtln("SSID:" + SSID);
    prtln("WiFi    .....  OK");
  }
  else
  {
    prtln("WiFi    .....  NG");
    STOP();
  }

  if (!setupServer())
    STOP();

  prtln("\nIP: " + IP_ADDR);
  prtln("SV: " + HOST_NAME);
  dbPrtln("*** setup() done! ***");
}

void loop()
{
  requestManage();
  vTaskDelay(1);
}
