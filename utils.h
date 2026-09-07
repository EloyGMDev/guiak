#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>

// Niveles de log
#define LOG_DEBUG  0
#define LOG_INFO   1
#define LOG_WARN   2
#define LOG_ERROR  3

void addLog(const String& type, const String& msg, uint8_t level = LOG_INFO);
String uptimeFormatted();
uint32_t getFreeHeapBytes();
uint32_t getFreePsramBytes();

#endif // UTILS_H
