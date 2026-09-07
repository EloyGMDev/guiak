#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>

// Inicializa el servicio de Wi-Fi respetando el horario escolar (07:00 a 20:30)
void wifiServiceInit();

// Gestiona el servidor web, la sincronización horaria y el ciclo de encendido/apagado programado
void handleWifiService();

// Enciende la radio Wi-Fi y levanta el servidor web (por horario o manual)
void startWifiService(bool isManual = false);

// Apaga la radio Wi-Fi y el servidor para ahorro de batería (fuera de horario)
void stopWifiService();

// Comprueba si la hora actual está dentro del intervalo programado (ej: 07:00 - 20:30)
bool isWithinWifiSchedule();

#endif // WEB_SERVER_H
