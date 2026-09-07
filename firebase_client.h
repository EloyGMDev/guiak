#ifndef FIREBASE_CLIENT_H
#define FIREBASE_CLIENT_H

#include <Arduino.h>

// Inicializa el cliente Firebase Realtime Database
void firebaseInit();

// Bucle principal: sincroniza config, revisa comandos remotos, vacía cola offline y actualiza estado
void firebaseLoop();

// Sincroniza la configuración remota (si el aula cambia en la nube, se adapta en caliente)
void firebaseSyncConfig();

// Comprueba y ejecuta órdenes remotas del centro (sonar aula, modo lockdown, etc.)
void firebaseCheckCommands();

// Envía un evento a Firebase (si no hay Wi-Fi, lo encola en memoria offline)
void firebasePushLog(const String& tag, const String& message, uint8_t level = 1);

// Registra la presencia y última ubicación de un estudiante (vía BLE de proximidad o tarjeta RFID)
void firebaseRecordStudentPresence(const String& studentId, const String& method, int rssi = 0);

// Vacía los eventos pendientes acumulados durante caídas de Wi-Fi
void firebaseFlushOfflineQueue();

// Actualiza el estado en vivo (batería, RSSI, uptime, online)
void firebaseUpdateStatus();

#endif // FIREBASE_CLIENT_H
