// *******************************************************
//  esp32fileServer          by NoRi 2025-08-30
// -------------------------------------------------------
// esp32fileServer.cpp
// *******************************************************
#include "fileServer/fileServer.h"
#include <TimeLib.h>
#include <time.h> // Timeライブラリ
//---------------------------------------------------------------------------
const String PROG_NAME = "esp32fileServer";
const String VERSION = "v1.06";
const String GITHUB_URL = "https://github.com/NoRi-230401/esp32fileServer";
const String WIFI_TXT = "/wifi.txt";

bool SD_ENABLE = false;
bool NTP_SYNC = false;

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

bool RTC_ADJUST_ON = false;
//---------------------------------------------------------------------------

// vsCode terminal cannot get serial data before 5 sec...!
// #define DEBUG_PLATFORMIO

void FLSV_setup();
void FLSV_loop();
String getCurrentDateTime();

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
  prtln("- " + PROG_NAME + " -");
#ifdef FILES_SD
  SD_ENABLE = SD_begin();
  if (SD_ENABLE)
    SD_start();
#endif
#endif

  if (LittleFS_USE)
    LittleFS_start();
  else if (SPIFFS_USE)
    SPIFFS_start();

  if (wifiStart())
  {
    IP_ADDR = WiFi.localIP().toString();
    SSID = WiFi.SSID();
    prtln("\nwife Connected!");
    prtln("SSID:" + SSID);
    prtln("WiFi    .....  OK");
  }
  else
  {
    prtln("\nWiFi    .....  NG");
    STOP();
  }

// check RTC enable
#ifdef M5STACK_DEVICE
  RTC_ENABLE = M5.Rtc.isEnabled();
#else
#ifdef RTC_MODULE
  RTC_ENABLE = true;
#else
  RTC_ENABLE = false;
#endif
#endif

  if (RTC_ENABLE)
  {
    prtln("RTC is enable");
#ifdef RTC_MODULE_DS3231
    DS3231_begin();
#endif
  }
  else
  {
    prtln("RTC is disable");
  }

  NTP_SYNC = NTP_begin();
  if (NTP_SYNC && RTC_ENABLE)
  { // RTC and DevTm syncronized with NTP
    adjustRTC();
  }
  else if (!NTP_SYNC && RTC_ENABLE)
  { // DevTm syncronized with RTC
    adjustDevTm();
  }

  if (!setupServer())
    STOP();

  prtln("IP: " + IP_ADDR);
  prtln("SV: " + HOST_NAME);
}

void FLSV_loop()
{
  requestManage();
}

String getCurrentDateTime()
{
  return getTmDev();
}
