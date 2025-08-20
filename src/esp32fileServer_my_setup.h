// *******************************************************
//  esp32fileServer          by NoRi 2025-08-20
// -------------------------------------------------------
// esp32fileSever_my_setup.h
// *******************************************************
#ifndef _ESP32FILESERVER_MY_SETUP_H
#define _ESP32FILESERVER_MY_SETUP_H
// -------------------------------------------------------


// (Part.1) ### direct WiFi setting ###
//  change for your wifi enviroment
//  also, you can setup with "wifi.txt" or smartConfig app
#define YOUR_SSID "your_ssid"
#define YOUR_SSID_PASS "your_ssid_pass"
#define YOUR_HOST_NAME "esp32fileServer"
// -------------------------------------------------------

// (Part.2) ### select file system ###
#define FILES_LITTLEFS  // if not defined,  use SPIFFS 
#define FILES_SD				//
// -------------------------------------------------------

// (Part.3) ### select device type ###
//  if not selected, use General-purpose ESP32 Development Boards
//#define CARDPUTER
//#define CORES3
//#define CORE2
// -------------------------------------------------------

// (Part.4) ### SPI PORT define to control SD ###
//  for General-purpuse ESP32 Development Boards only, not use in M5STACK_DEVICE
#define SD_CS    4
#define SD_MOSI  5
#define SD_MISO  6
#define SD_SCK   7
// -------------------------------------------------------

#if defined(CARDPUTER) || defined(CORES3) || defined(CORE2)
#define M5STACK_DEVICE
#endif

#if defined(CORES3) || defined(CORE2)
#define RTC_BUILT_IN
#endif

// -------------------------------------------------------
#endif
