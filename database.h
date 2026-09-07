#ifndef DATABASE_H
#define DATABASE_H

#include <Arduino.h>
#include "config.h"

// Inicializa el sistema de almacenamiento no volátil (NVS)
void dbInit();

// Carga la configuración del nodo desde la memoria Flash NVS
void dbLoadConfig();

// Guarda la configuración del nodo en la memoria Flash NVS
void dbSaveConfig();

// Restablece la configuración del nodo a valores de fábrica
void dbResetDefaults();

// Incrementa y consulta el contador total de accesos/visitas
void dbIncrementAccessCount();
uint32_t dbGetAccessCount();

// Comprobación y registro de tarjetas RFID
bool dbIsCardAuthorized(const String& uid);
void dbAuthorizeCard(const String& uid, const String& ownerName);

#endif // DATABASE_H
