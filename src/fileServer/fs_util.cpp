// *******************************************************
//  esp32fileServer          by NoRi 2025-04-15
// -------------------------------------------------------
// fs_util.cpp
// *******************************************************
#include "fileServer.h"

void prt(String message);
void prtln(String message);
void dbPrtln(String message);
void dbPrt(String message);
String ConvBytesUnits(uint64_t bytes, int dp, int unit);
bool wifiStart();
bool wifiConnect01();
bool wifiConnect02();
bool wifiConnect03();
bool mdnsStart(void);
String getTmNTP();
String strTmInfo(struct tm &timeInfo);
bool getWiFiSettings(int flType, const String filename);
String urlEncode(const String &input);
String urlDecode(const String &input);
void requestManage();
void sendReq(int reqNo);
void STOP();
void REBOOT();
bool SPIFFS_begin();
void SPIFFS_start();
bool SD_begin();
void SD_start();
bool SD_cardInfo(void);
bool getWiFiInfo();

// -------------------------------------------------------
uint32_t SHUTDOWN_TM_SEC = 3; // default 3sec after shutdown api
int REQUEST_NO = REQ_NONE;

#define DEBUG2
void dbPrtln(String message)
{
#ifdef DEBUG2
  Serial.println(message);
#endif
}
void dbPrt(String message)
{
#ifdef DEBUG2
  Serial.print(message);
#endif
}
void prtln(String message)
{
  Serial.println(message);

#ifdef M5STACK_DEVICE
#ifdef CARDPUTER
  M5Cardputer.Display.println(message);
#else
  M5.Display.println(message);
#endif
#endif
}

void prt(String message)
{
  Serial.print(message);
#ifdef M5STACK_DEVICE
#ifdef CARDPUTER
  M5Cardputer.Display.print(message);
#else
  M5.Display.print(message);
#endif
#endif
}

String ConvBytesUnits(uint64_t bytes, int dp, int unit)
{ // int dp : 小数点以下の桁数、decimal places
  const uint64_t KILO = 1024ULL;
  const uint64_t MEGA = KILO * KILO;
  const uint64_t GIGA = MEGA * KILO;
  const uint64_t TERA = GIGA * KILO;

  if (unit == UNIT_AUTO)
  {
    if (bytes < KILO)
    {
      return (String(bytes) + " B");
    }
    else if (bytes < MEGA)
    {
      float kb = (float)bytes / (float)KILO;
      return String(kb, dp) + " KB";
    }
    else if (bytes < GIGA)
    {
      float mb = (float)bytes / (float)MEGA;
      return (String(mb, dp) + " MB");
    }
    else if (bytes < TERA)
    {
      float gb = (float)bytes / (float)GIGA;
      return (String(gb, dp) + " GB");
    }
    else
    {
      float tb = (float)bytes / (float)TERA;
      return (String(tb, dp) + " TB");
    }
  }
  else if (unit == UNIT_KIRO)
  {
    float kb = (float)bytes / (float)KILO;
    return String(kb, dp) + " KB";
  }
  else if (unit == UNIT_MEGA)
  {
    float mb = (float)bytes / (float)MEGA;
    return (String(mb, dp) + " MB");
  }
  else if (unit == UNIT_GIGA)
  {
    float gb = (float)bytes / (float)GIGA;
    return (String(gb, dp) + " GB");
  }
  else if (unit == UNIT_TERA)
  {
    float tb = (float)bytes / (float)TERA;
    return (String(tb, dp) + " TB");
  }
  // UNIT_BYTE
  return (String(bytes) + " B");
}

bool wifiStart()
{
  dbPrtln("*** WiFi Start ***");

  // "wifi.txt" info at LittleFS, SPIFFS, SD
  if (getWiFiInfo())
  {
    if (wifiConnect01())
      return true;
  }

  // privious connected wifi-info use
  if (wifiConnect02())
    return true;

  // ESP SmartConfig
  if (wifiConnect03())
    return true;

  return false;
}

bool wifiConnect01()
{
  dbPrtln("use wifi.txt info");
  WiFi.disconnect();
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_STA);
  delay(500);
  WiFi.begin(SSID, SSID_PASS);
  prt("*");
  int count = 1;
  const int COUNT_MAX = 20;
  delay(500);

  while (WiFi.status() != WL_CONNECTED)
  {
    count++;
    prt("*");
    delay(500);
    if (count >= COUNT_MAX)
    {
      dbPrtln("\ncannot connect ,Wifi faile!");
      return false;
    }
  }
  return true;
}

bool wifiConnect02()
{
  // privious connected wifi-info use
  dbPrtln("\nprivious connected wifi-info use");
  WiFi.disconnect();
  WiFi.begin();
  prtln("\n");

  int loopCount10sec = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    prt("*");
    delay(500);
    // 10秒以上接続できなかったら false
    if (loopCount10sec++ > 10 * 2)
    {
      dbPrtln("\nPrivious connect info use ,Wifi faile!");
      return false;
    }
  }
  return true;
}

bool wifiConnect03()
{ // ** ESP SmartConfig  ***
  // ---------------------------------------
  // Init WiFi as Station, begin SmartConfig
  WiFi.mode(WIFI_AP_STA);
  WiFi.beginSmartConfig();
  // ---------------------------------------
  int loopCount30sec = 0;
  // Wait for SmartConfig packet from mobile
  dbPrtln("\nWaiting for SmartConfig packet");
  while (!WiFi.smartConfigDone())
  {
    delay(500);
    prt("#");
    // 30秒以上 smartConfigDoneできなかったら false
    // 60秒以上 smartConfigDoneできなかったら false
    if (loopCount30sec++ > 30 * 4)
    {
      dbPrtln("\nSmartConfig not recieved ,Wifi faile!");
      return false;
    }
  }
  dbPrtln("\nSmartConfig packet received.");

  // ---------------------------------------
  // Wait for WiFi to connect to AP
  dbPrtln("\nWaiting for WiFi");
  prtln("\n");
  int loopCount60sec = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    prt("*");
    // 60秒以上接続できなかったら false
    if (loopCount60sec++ > 60 * 2)
    {
      return false;
    }
  }
  return true;
}

bool mdnsStart(void)
{
  if (!MDNS.begin(HOST_NAME.c_str()))
  {
    Serial.println("ERR: MDNS cannot start");
    Serial.println("ERR: ServerName = " + HOST_NAME);
    return false;
  }

  Serial.println("mDNS ServerName = " + HOST_NAME);
  return true;
}

String getTmNTP()
{
  struct tm Ldt;
  for (int i = 0; i < 5; i++)
  {
    if (getLocalTime(&Ldt, 1000U))
      return strTmInfo(Ldt);

    delay(10);
  }

  String errStr = "2025/04/01(Tue) 00:00:00";
  return errStr;
}

String strTmInfo(struct tm &timeInfo)
{
  char buf[60];
  static constexpr const char *const wd[7] = {"Sun", "Mon", "Tue", "Wed", "Thr", "Fri", "Sat"};

  sprintf(buf, "%04d/%02d/%02d(%s) %02d:%02d:%02d",
          timeInfo.tm_year + 1900, timeInfo.tm_mon + 1, timeInfo.tm_mday,
          wd[timeInfo.tm_wday], timeInfo.tm_hour, timeInfo.tm_min, timeInfo.tm_sec);

  return String(buf);
}

bool getWiFiSettings(int flType, const String filename)
{
  File fs;

  if (flType == FS_LittleFS)
  {
    if (!LittleFS.exists(filename))
      return false;

    fs = LittleFS.open(filename, FILE_READ);

    if (!fs)
      return false;
  }
  else if (flType == FS_SPIFFS)
  {
    if (!SPIFFS.exists(filename))
      return false;

    fs = SPIFFS.open(filename, FILE_READ);

    if (!fs)
      return false;
  }
  else if (flType == FS_SD)
  {
    if (!SD.exists(filename))
      return false;

    fs = SD.open(filename, FILE_READ);
    if (!fs)
      return false;
  }
  else
  {
    dbPrtln("getWiFiSettings Err: invalid flType");
    return false;
  }

  size_t length = fs.size();
  if (length <= 3) // at least 3bytes size
    return false;

  char buf[length + 1];
  fs.read((uint8_t *)buf, length);
  buf[length] = 0;
  fs.close();

  int x;
  int y = 0;
  int z = 0;
  for (x = 0; x < length; x++)
  {
    if (buf[x] == 0x0a || buf[x] == 0x0d)
      buf[x] = 0;
    else if (!y && x > 0 && !buf[x - 1] && buf[x])
      y = x;
    else if (!z && x > 0 && !buf[x - 1] && buf[x])
      z = x;
  }

  if (y == 0)
    return false;
  SSID = String(buf);
  SSID_PASS = String(&buf[y]);

  if (z == 0)
    return false;
  HOST_NAME = String(&buf[z]);

  if (SSID == "" || SSID_PASS == "" || HOST_NAME == "")
    return false;

  return true;
}

// URLエンコード関数
String urlEncode(const String &input)
{
  String encodedString = "";
  char c;
  char code0;
  char code1;
  for (int i = 0; i < input.length(); i++)
  {
    c = input.charAt(i);
    if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
    {
      encodedString += c;
    }
    else if (c == ' ')
    {
      encodedString += "%20";
    }
    else
    {
      code1 = (c & 0xf) + '0';
      if ((c & 0xf) > 9)
      {
        code1 = (c & 0xf) - 10 + 'A';
      }
      c = (c >> 4) & 0xf;
      code0 = c + '0';
      if (c > 9)
      {
        code0 = c - 10 + 'A';
      }
      encodedString += '%';
      encodedString += code0;
      encodedString += code1;
    }
  }
  return encodedString;
}

// URLデコード関数
String urlDecode(const String &input)
{
  String decodedString = "";
  char c;
  char code0;
  char code1;
  for (int i = 0; i < input.length(); i++)
  {
    c = input.charAt(i);
    if (c == '+')
    { // '+' はスペースとしてデコードする場合もあるが、ここでは%20のみ対応
      decodedString += ' ';
    }
    else if (c == '%')
    {
      i++;
      if (i < input.length())
      {
        code0 = input.charAt(i);
        i++;
        if (i < input.length())
        {
          code1 = input.charAt(i);
          char decodedChar = 0;
          // 16進文字を数値に変換
          if (code0 >= '0' && code0 <= '9')
            decodedChar = (code0 - '0') << 4;
          else if (code0 >= 'A' && code0 <= 'F')
            decodedChar = (code0 - 'A' + 10) << 4;
          else if (code0 >= 'a' && code0 <= 'f')
            decodedChar = (code0 - 'a' + 10) << 4;
          else
          {                       // 不正なエンコード形式
            decodedString += '%'; // '%'をそのまま追加
            i -= 2;               // インデックスを戻す
            continue;
          }

          if (code1 >= '0' && code1 <= '9')
            decodedChar |= (code1 - '0');
          else if (code1 >= 'A' && code1 <= 'F')
            decodedChar |= (code1 - 'A' + 10);
          else if (code1 >= 'a' && code1 <= 'f')
            decodedChar |= (code1 - 'a' + 10);
          else
          { // 不正なエンコード形式
            decodedString += '%';
            decodedString += code0;
            i--;
            continue;
          }
          decodedString += decodedChar;
        }
        else
        { // %XX の形式でない
          decodedString += '%';
          decodedString += code0;
        }
      }
      else
      { // 文字列末尾が %
        decodedString += '%';
      }
    }
    else
    {
      decodedString += c;
    }
  }
  return decodedString;
}

void requestManage()
{

#ifdef M5STACK_DEVICE
  if (RTC_ADJUST_ON && RTC_ENABLE && (millis() - TM_SETUP_DONE > TM_RTC_ADJUST))
  {
    adjustRTC();
    RTC_ADJUST_ON = false;
  }
#endif

  if (REQUEST_NO == REQ_NONE)
    return;

  int req = REQUEST_NO;
  switch (req)
  {
  case REQ_REBOOT:
    REQUEST_NO = REQ_NONE;
    REBOOT();
    return;

#ifdef M5STACK_DEVICE
  case REQ_SHUTDOWN:
    REQUEST_NO = REQ_NONE;
    // SHUTDOWN_TM_SEC = 0;
    POWER_OFF();
    return;
#endif

  default:
    REQUEST_NO = REQ_NONE;
    Serial.println("requeestManage : invalid request get ");
  }
  return;
}

void sendReq(int reqNo)
{
  REQUEST_NO = reqNo;
}

void STOP()
{
  Serial.println(" *** Stop *** fatal error");
  if (SD_ENABLE)
    SD.end();
  if (LittleFS_ENABLE)
    LittleFS.end();
  else if (SPIFFS_ENABLE)
    SPIFFS.end();

  delay(5000);
  for (;;)
  {
    delay(1000);
  }
}

void REBOOT()
{
  Serial.println(" *** Reboot ***");

  if (SD_ENABLE)
    SD.end();

  if (LittleFS_ENABLE)
    LittleFS.end();
  else if (SPIFFS_ENABLE)
    SPIFFS.end();

  delay(SHUTDOWN_TM_SEC * 1000L);
  ESP.restart();
  // *** NEVER RETURN ***
  for (;;)
  {
    delay(1000);
  }
}

bool SPIFFS_begin()
{
  if (!SPIFFS.begin(true))
  {
    Serial.println("ERR: SPIFFS begin erro...");
    return false;
  }
  return true;
}

void SPIFFS_start()
{
  SPIFFS_ENABLE = false;
  if (SPIFFS_USE)
  {
    SPIFFS_ENABLE = SPIFFS_begin();
    if (SPIFFS_ENABLE)
      prtln("SPIFFS  .....  OK");
    else
      prtln("SPIFFS  .....  NG");
  }
}

bool LittleFS_begin()
{
  if (!LittleFS.begin(true))
  {
    Serial.println("ERR: LittleFS begin erro...");
    return false;
  }
  return true;
}

void LittleFS_start()
{
  LittleFS_ENABLE = false;
  if (LittleFS_USE)
  {
    LittleFS_ENABLE = LittleFS_begin();
    if (LittleFS_ENABLE)
      prtln("LittleFS ....  OK");
    else
      prtln("LittleFS ....  NG");
  }
}

void SD_start()
{
  // --- SD start ---
  if (SD_USE)
  {
    if (SD_ENABLE)
      prtln("SD      .....  OK");
    else
      prtln("SD      .....  NG");
  }
}

bool SD_cardInfo(void)
{
  sdcard_type_t cardType = SD.cardType();
  switch (cardType)
  {
  case CARD_MMC:
    Serial.println("MMC detected");
    break;
  case CARD_SD:
    Serial.println("SD detected");
    break;
  case CARD_SDHC:
    Serial.println("SDHC detected");
    break;
  case CARD_NONE:
    Serial.println("ERR: No SD card attached");
    return false;
  case CARD_UNKNOWN:
    Serial.println("ERR: SD card unknown Type");
    return false;
  default:
    Serial.println("ERR: SD cardType is default Type");
    return false;
  }
  return true;
}

bool getWiFiInfo()
{
  // ------- Network Settings Read ---------
  SSID = "";
  SSID_PASS = "";
  HOST_NAME = "";

  if (SD_ENABLE && getWiFiSettings(FS_SD, WIFI_TXT))
    Serial.println("Settings read from SD");

  else if (LittleFS_ENABLE && getWiFiSettings(FS_LittleFS, WIFI_TXT))
    Serial.println("Settings read from LittleFS");

  else if (SPIFFS_ENABLE && getWiFiSettings(FS_SPIFFS, WIFI_TXT))
    Serial.println("Settings read from SPIFFS");

  if (SSID == "")
    SSID = YOUR_SSID;

  if (SSID_PASS == "")
    SSID_PASS = YOUR_SSID_PASS;

  if (HOST_NAME == "")
    HOST_NAME = YOUR_HOST_NAME;

  if (SSID == "" || SSID_PASS == "" || HOST_NAME == "")
  {
    dbPrtln("Wifi SETTINGS read ERROR");
    return false;
  }
  dbPrtln("Wifi SETTINGS.....  OK");
  return true;
}
