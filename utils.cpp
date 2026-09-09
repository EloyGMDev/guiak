#include "utils.h"

#if defined(ESP32)
#include <esp_system.h>
#endif

void addLog(const String& type, const String& msg, uint8_t level) {
  unsigned long ms = millis();
  unsigned long s = ms / 1000;
  char ts[16];
  snprintf(ts, sizeof(ts), "%02lu:%02lu:%02lu", (s / 3600), (s % 3600) / 60, s % 60);

  const char* lvlStr = "INFO";
  if (level == LOG_DEBUG) lvlStr = "DEBUG";
  else if (level == LOG_WARN)  lvlStr = "WARN";
  else if (level == LOG_ERROR) lvlStr = "ERROR";

  Serial.print("[");
  Serial.print(ts);
  Serial.print("][");
  Serial.print(lvlStr);
  Serial.print("][");
  Serial.print(type);
  Serial.print("] ");
  Serial.println(msg);
}

String uptimeFormatted() {
  unsigned long s = millis() / 1000;
  char buf[16];
  snprintf(buf, sizeof(buf), "%02lu:%02lu:%02lu", s / 3600, (s % 3600) / 60, s % 60);
  return String(buf);
}

uint32_t getFreeHeapBytes() {
#if defined(ESP32)
  return ESP.getFreeHeap();
#else
  return 32768;
#endif
}

uint32_t getFreePsramBytes() {
#if defined(ESP32)
  return ESP.getFreePsram();
#else
  return 0;
#endif
}
