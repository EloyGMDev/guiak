#include "ntp_sync.h"
#include "utils.h"

#if defined(ESP32)
#include <WiFi.h>
#include <time.h>
#elif defined(ARDUINO_UNOR4_WIFI)
#include <WiFiS3.h>
#include <time.h>
#else
#include <WiFi.h>
#include <time.h>
#endif

const char* NTP_SERVER = "pool.ntp.org";

bool ntpSync(int gmtOffsetSec, int daylightOffsetSec) {
  if (WiFi.status() != WL_CONNECTED) {
    return false;
  }

#if defined(ESP32)
  configTime(gmtOffsetSec, daylightOffsetSec, NTP_SERVER);

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo, 2000)) {
    addLog("NTP", "Fallo al obtener hora de red", LOG_WARN);
    return false;
  }

  char timeStr[64];
  strftime(timeStr, sizeof(timeStr), "%d/%m/%Y %H:%M:%S", &timeinfo);
  addLog("NTP", "Hora sincronizada: " + String(timeStr), LOG_INFO);
  return true;
#elif defined(ARDUINO_UNOR4_WIFI)
  unsigned long epoch = WiFi.getTime();
  if (epoch == 0) {
    addLog("NTP", "Fallo al obtener hora de red (WiFiS3)", LOG_WARN);
    return false;
  }
  epoch += gmtOffsetSec + daylightOffsetSec;
  time_t t = (time_t)epoch;
  struct tm* timeinfo = gmtime(&t);
  char timeStr[64];
  strftime(timeStr, sizeof(timeStr), "%d/%m/%Y %H:%M:%S", timeinfo);
  addLog("NTP", "Hora sincronizada: " + String(timeStr), LOG_INFO);
  return true;
#else
  return false;
#endif
}
