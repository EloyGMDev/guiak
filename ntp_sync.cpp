#include "ntp_sync.h"
#include "utils.h"
#include <WiFi.h>
#include <time.h>

const char* NTP_SERVER = "pool.ntp.org";

bool ntpSync(int gmtOffsetSec, int daylightOffsetSec) {
  if (WiFi.status() != WL_CONNECTED) {
    return false;
  }

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
}
