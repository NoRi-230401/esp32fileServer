// *******************************************************
//  esp32fileServer          by NoRi 2025-08-30
// -------------------------------------------------------
// esp32fileServer.cpp
// *******************************************************
#include "fileServer/fileServer.h"
//---------------------------------------------------------------------------
const String PROG_NAME = "esp32fileServer";
const String VERSION = "v1.06";
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

// vsCode terminal cannot get serial data before 5 sec...!
// #define DEBUG_PLATFORMIO

void FLSV_setup();
void FLSV_loop();


void FLSV_setup()
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
  delay(1000);
#ifdef FILES_SD
  SD_ENABLE = SD_begin();
  if(SD_ENABLE)
    SD_start();
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
  dbPrtln("* fileServer setup done!*");
}

void FLSV_loop()
{
  requestManage();
}

