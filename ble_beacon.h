#ifndef BLE_BEACON_H
#define BLE_BEACON_H

#include <Arduino.h>

// Inicializa el subsistema Bluetooth BLE (iBeacon + Servidor GATT de interacción)
void bleBeaconInit();

// Actualiza el nivel de batería en la característica BLE
void bleUpdateBatteryLevel(uint8_t percent);

// Detiene temporalmente la radio BLE (para ahorro extremo o inicio de Wi-Fi)
void bleStop();

// Reinicia la publicidad BLE tras parar Wi-Fi
void bleStart();

// Comprueba si hay peticiones pendientes
void bleLoop();

#endif // BLE_BEACON_H
