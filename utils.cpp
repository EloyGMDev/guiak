#include "utils.h"
#include <esp_system.h>

void addLog(const String& type, const String& msg, uint8_t level) {
  unsigned long ms = millis();
  unsigned long s = ms / 1000;
  char ts[16];
  snprintf(ts, sizeof(ts), "%02lu:%02lu:%02lu", (s / 3600), (s % 3600) / 60, s % 60);

  const char* lvlStr = "INFO";
  if (level == LOG_DEBUG) lvlStr = "DEBUG";
  else if (level == LOG_WARN)  lvlStr = "WARN";
  else if (level == LOG_ERROR) lvlStr = "ERROR";

  Serial.printf("[%s][%s][%s] %s\n", ts, lvlStr, type.c_str(), msg.c_str());
}

String uptimeFormatted() {
  unsigned long s = millis() / 1000;
  char buf[16];
  snprintf(buf, sizeof(buf), "%02lu:%02lu:%02lu", s / 3600, (s % 3600) / 60, s % 60);
  return String(buf);
}

uint32_t getFreeHeapBytes() {
  return ESP.getFreeHeap();
}

uint32_t getFreePsramBytes() {
  return ESP.getFreePsram();
}
