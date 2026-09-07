#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include <Arduino.h>

// Inicializa ADC y configuración de bajo consumo
void powerInit();

// Lee el voltaje actual de la batería en milivoltios
uint16_t readBatteryMillivolts();

// Actualiza las variables globales batteryMillivolts y batteryPercent
void updateBatteryStatus();

// Comprueba si la batería está por debajo del umbral de aviso
bool isBatteryLow();

// Duerme la CPU en modo Light Sleep manteniendo la memoria RAM y el estado de BLE
void enterLightSleep(uint32_t durationMs);

#endif // POWER_MANAGER_H
