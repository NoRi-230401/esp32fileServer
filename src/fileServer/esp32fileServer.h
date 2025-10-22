// *******************************************************
//  esp32fileServer          by NoRi 2025-08-01
// -------------------------------------------------------
// fileServer.h
// *******************************************************
#ifndef _ESP32_FILE_SERVER_H
#define _ESP32_FILE_SERVER_H
// -------------------------------------------------------

#include <Arduino.h>
extern void fileServer_setup();
extern void fileServer_loop();
extern String getCurrentDateTime();
extern bool SD_ENABLE;
extern bool NTP_SYNC;
// -------------------------------------------------------
#endif
