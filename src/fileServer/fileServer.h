// *******************************************************
//  esp32fileServer          by NoRi 2025-08-01
// -------------------------------------------------------
// fileServer.h
// *******************************************************
#ifndef _M5STACK_FILE_SERVER_H
#define _M5STACK_FILE_SERVER_H
// -------------------------------------------------------
#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <algorithm>
#include <vector>
#include <FS.h>
#include <LittleFS.h>
#include <SPIFFS.h>
#include <SD.h>
#include <nvs.h>
#include <time.h>
#ifdef M5STACK_DEVICE
#ifndef CARDPUTER
  #include <M5Unified.h>
#else
  #include <M5Cardputer.h>
#endif
#endif

// --- used in 'main.cpp' ----
extern void m5stack_begin();
extern void dbPrtln(String message);
extern void dbPrt(String message);
extern void prt(String message);
extern void prtln(String message);

extern void SDU_lobby();
extern void SD_start();
extern void LittleFS_start();
extern void SPIFFS_start();
extern bool setupServer();
extern void requestManage();
extern void STOP();
//---------------------------
extern void adjustRTC();
extern String getTmRTC();
extern void POWER_OFF();
extern void m5stack_begin();
extern void SDU_lobby();
extern bool SD_begin();
extern bool SD_cardInfo(void);
extern String strTmInfo(struct tm &timeInfo);

typedef struct
{
  String filename;
  String ftype;
  String fsize;
} fileinfo;

//---- units ------
#define UNIT_AUTO 1
#define UNIT_BYTE 2
#define UNIT_KIRO 3
#define UNIT_MEGA 4
#define UNIT_GIGA 5
#define UNIT_TERA 6

// - File System Types -
#define FS_SPIFFS 1
#define FS_SD 2
#define FS_LittleFS 3

// -- REQUEST Manager --
#define REQ_NONE 0
#define REQ_REBOOT 98
#define REQ_SHUTDOWN 99

extern bool wifiStart();
extern bool mdnsStart(void);
extern String HTML_Header();
extern String HTML_Footer();
extern String getContentType(String filenametype);
extern bool compareFileinfo(const fileinfo &a, const fileinfo &b);
extern String ConvBytesUnits(uint64_t bytes, int dp, int unit = UNIT_AUTO);
extern String getTmRTC();
extern String getTmNTP();
extern String urlEncode(const String &input);
extern String urlDecode(const String &input);
extern bool getWiFiSettings(int flType, const String filename);
extern uint64_t getFileSize(int flType, String filename);
extern void webApiSetup();
extern void sendReq(int reqNo);

// -------------------------------------------------------
extern const String PROG_NAME, VERSION, GITHUB_URL;
extern const String YOUR_SSID, YOUR_SSID_PASS, YOUR_HOST_NAME;
extern String SSID, SSID_PASS, HOST_NAME, IP_ADDR;

extern const String WIFI_TXT;
extern const bool SD_USE, SPIFFS_USE, LittleFS_USE;
extern bool SD_ENABLE, LittleFS_ENABLE, SPIFFS_ENABLE;
extern bool RTC_ENABLE;
extern String SdPath, LfPath;

extern bool RTC_ADJUST_ON;
extern uint32_t TM_SETUP_DONE;
extern uint32_t TM_RTC_ADJUST;
extern uint32_t SHUTDOWN_TM_SEC;
// -------------------------------------------------------
#endif // _M5STACK_FILE_SERVER_H
