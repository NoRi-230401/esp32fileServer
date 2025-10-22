// *******************************************************
//  esp32fileServer          by NoRi 2025-08-20
// -------------------------------------------------------
// esp32fileSever_my_setup.h
// *******************************************************
#ifndef _ESP32FILESERVER_MY_SETUP_H
#define _ESP32FILESERVER_MY_SETUP_H
// -------------------------------------------------------

// (Part.1) ### direct WiFi settings ###
//  change for your wifi enviroment
//  also, you can setup with "wifi.txt" or smartConfig App
#define YOUR_SSID "your_ssid"
#define YOUR_SSID_PASS "your_ssid_pass"
#define YOUR_HOST_NAME "esp32fileServer"
// -------------------------------------------------------

// (Part.2) ### device type ###
// - uncomment one device type only -
#define CARDPUTER       // m5stack cardputer v1.0 and v1.1
//#define CORES3          // m5stack coreS3
//#define CORE2           // m5stack core2  
// #define ESP32_DEV_BOARD   // General-purpuse ESP32 Development Board
// -------------------------------------------------------

// (Part.3) ### use file system ###
#define FILES_LITTLEFS  // if not defined,  use SPIFFS 
//#define FILES_SD			// if your device type is 'ESP32_DEV_BOARD' and use SD   
// -------------------------------------------------------

// (Part.4) ### NTP SERVER ###
// NTP connect information.
#define NTP_SVR1 "ntp.nict.jp"         // NTP server1
#define NTP_SVR2 "ntp.jst.mfeed.ad.jp" // NTP server2
#define NTP_GMT_OFFSET 9 * 3600L       // Sec  : GMT offset
#define NTP_DAYLIGHT_OFFSET 0          // Sec  : daylight offset
// -------------------------------------------------------

// (Part.5) ### RTC module ###
//  ** if you use M5STACK_DEVICE, SKIP this Part ** 
//  Only used with General-purpuse ESP32 Development Board
// #define RTC_MODULE
// #define RTC_MODULE_DS3231
// #define WIRE Wire
// #define WIRE Wire1
// #define SDA_PIN  16   // I2C For RTC MODULE  use: Wire1.begin(SDA_PIN, SCL_PIN);
// #define SCL_PIN  17
// -------------------------------------------------------

// (Part.6) ### SPI PORT define to control SD ###
//  ** if you use M5STACK_DEVICE, SKIP this Part ** 
//  Only used with General-purpuse ESP32 Development Board and SD
// #define SD_CS    4
// #define SD_MOSI  5
// #define SD_MISO  6
// #define SD_SCK   7
// -------------------------------------------------------


// -------------------------------------------------------
#endif
