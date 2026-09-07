#ifndef NTP_SYNC_H
#define NTP_SYNC_H

#include <Arduino.h>

// Sincroniza la hora del ESP32 mediante SNTP si hay conexión Wi-Fi activa
bool ntpSync(int gmtOffsetSec = 3600, int daylightOffsetSec = 3600);

#endif // NTP_SYNC_H
