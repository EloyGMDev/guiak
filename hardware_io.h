#ifndef HARDWARE_IO_H
#define HARDWARE_IO_H

#include <Arduino.h>
#include <MFRC522.h>

extern MFRC522 mfrc522;

// Inicializa periféricos de entrada y salida (RFID, Botones, Pines)
void hardwareInit();

// Comprueba si el botón de configuración fue presionado/mantenido
void checkConfigButton();

// Comprueba de forma no bloqueante si hay una tarjeta RFID presente
void handleRFID();

// Comprueba si el lector RFID MFRC522 está presente y respondiendo en el bus SPI
bool checkRFIDConnected();

#endif // HARDWARE_IO_H
