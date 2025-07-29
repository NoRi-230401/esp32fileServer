// *******************************************************
//  m5stack-device.cpp          by NoRi 2025-04-15
// *******************************************************
#ifdef M5STACK_DEVICE
//-------------------------------------------------------------
#include "fileServer.h"
#include <M5Unified.h>
#include <M5StackUpdater.h>
#if defined(CARDPUTER)
#include <M5Cardputer.h>
SPIClass SPI2;
#endif

void adjustRTC();
String getTmRTC();
void POWER_OFF();
bool SD_begin();
void m5stack_begin();
void SDU_lobby();
// -------------------------------------------------------

void adjustRTC()
{
  struct tm tmInfo;

  while (!getLocalTime(&tmInfo, 1000U))
    delay(10);

  M5.Rtc.setDateTime(tmInfo);
  prtln("\nRTC adjusted .... " + strTmInfo(tmInfo));
}

String getTmRTC()
{
  char buf[60];
  static constexpr const char *const wd[7] = {"Sun", "Mon", "Tue", "Wed", "Thr", "Fri", "Sat"};
  auto dt = M5.Rtc.getDateTime();
  sprintf(buf, "%04d/%02d/%02d(%s) %02d:%02d:%02d", dt.date.year, dt.date.month, dt.date.date, wd[dt.date.weekDay], dt.time.hours, dt.time.minutes, dt.time.seconds);

  return String(buf);

  return "";
}

void POWER_OFF()
{
  Serial.println(" *** POWER OFF ***");

  SD.end();
  SPIFFS.end();
  delay(SHUTDOWN_TM_SEC * 1000L);
  M5.Power.powerOff();

  for (;;)
  { // never
    delay(1000);
  }
}

bool SD_begin()
{
  return false;

  int i = 0;

#if defined(CARDPUTER)
  // ------------- CARDPUTER -------------
  while (!SD.begin(M5.getPin(m5::pin_name_t::sd_spi_ss), SPI2) && i < 10)
#else
  // ----------- Core2 and CoreS3 ----------
  while (!SD.begin(GPIO_NUM_4, SPI, 25000000) && i < 10)
#endif
  {
    delay(500);
    i++;
  }

  if (i >= 10)
  {
    Serial.println("ERR: SD begin erro...");
    return false;
  }

  if (!SD_cardInfo())
    return false;

  return true;
}

void m5stack_begin()
{
  auto cfg = M5.config();
  cfg.serial_baudrate = 115200;

#if defined(CARDPUTER)
  // ------------- CARDPUTER ---------------
  M5Cardputer.begin(cfg, true);
  M5Cardputer.Display.setBrightness(70);
  M5Cardputer.Display.setRotation(1);
  M5Cardputer.Display.fillScreen(TFT_BLACK);
  M5Cardputer.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5Cardputer.Display.setFont(&fonts::Font0);
  M5Cardputer.Display.setTextSize(2);
  M5Cardputer.Display.setTextWrap(false);
  M5Cardputer.Display.setCursor(0, 0);

  SPI2.begin(
      M5.getPin(m5::pin_name_t::sd_spi_sclk),
      M5.getPin(m5::pin_name_t::sd_spi_miso),
      M5.getPin(m5::pin_name_t::sd_spi_mosi),
      M5.getPin(m5::pin_name_t::sd_spi_ss));

#else
  // ----------- Core2 and CoreS3 ----------
  M5.begin(cfg);
  M5.Display.setBrightness(120);
  M5.Display.setRotation(1);
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setFont(&fonts::Font0);
  M5.Display.setTextSize(2);
  M5.Display.setTextWrap(false);
  M5.Display.setCursor(0, 0);

#endif

  SD_ENABLE = SD_begin();
}

// ------------------------------------------
// SDU_lobby : SD_uploader lobby
// ------------------------------------------
// load "/menu.bin" on SD
//    if 'a' or 'BtnA' pressed at booting
// ------------------------------------------
void SDU_lobby()
{
  // CoreS3 は、最初からBtnAを押していると認識しないのでメッセージ表示後に押下する
  // Core2 と Cardputer は、最初から BtnA or 'a' を押していればいい。
#ifdef CORES3
  M5.Display.setCursor(0, M5.Display.height() / 2 - 30);
  M5.Display.setTextColor(GREEN);
  disp("  Press BtnA to load menu");
  delay(3000);
#endif

#if defined(CARDPUTER)
  M5Cardputer.update();
  if (M5Cardputer.Keyboard.isKeyPressed('a'))
#else
  M5.update();
  if (M5.BtnA.isPressed())
#endif
  {
    updateFromFS(SD, "/menu.bin");
    ESP.restart();

    while (true)
      ;
  }

#ifdef CORES3
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setCursor(0, 0);
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
#endif
}


#endif  // M5STACK_DEVICE


