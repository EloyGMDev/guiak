#ifndef FIREBASE_CLIENT_H
#define FIREBASE_CLIENT_H

#include <Arduino.h>

// Inicializa el cliente Firebase Realtime Database
void firebaseInit();

// Bucle periodico de sincronizacion (consulta cambios remotos y actualiza estado)
void firebaseLoop();

// Comprueba si hay cambios de configuracion en Firebase y los aplica en local
void firebaseSyncConfig();

// Envia un evento o traza de log a la coleccion de auditoria en Firebase
void firebasePushLog(const String& tag, const String& message, uint8_t level = 1);

// Actualiza el estado del nodo (bateria, conectividad, ultima conexion) en Firebase
void firebaseUpdateStatus();

#endif // FIREBASE_CLIENT_H
