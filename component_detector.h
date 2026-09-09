#ifndef COMPONENT_DETECTOR_H
#define COMPONENT_DETECTOR_H

#include <Arduino.h>
#include "config.h"

// ════════════════════════════════════════════════════════════════
//  ESTRUCTURA DEL REPORTE DE SALUD DE COMPONENTES
// ════════════════════════════════════════════════════════════════
struct ComponentReport {
  bool rfidOk;           // Lector RFID MFRC522 (comunicación SPI y VersionReg)
  bool batteryOk;        // Sensor ADC divisor de tensión de batería
  bool audioSpeakerOk;   // Altavoz (I2S MAX98357A en ESP32-S3 o DAC en UNO R4)
  bool buzzerOk;         // Zumbador/Buzzer piezoeléctrico
  bool nvsOk;            // Almacenamiento no volátil NVS (Preferences)
  bool bleOk;            // Baliza Bluetooth Low Energy (iBeacon / GATT)
  bool allCriticalOk;    // True si todos los componentes requeridos están OK
  String missingSummary; // Resumen textual con los componentes ausentes o fallidos
};

// Inicializa pines y subsistemas asociados a la detección y diagnóstico
void componentDetectorInit();

// Ejecuta el autotest completo de componentes en el arranque (POST - Power-On Self-Test)
ComponentReport runComponentSelfTest();

// Supervisión continua no bloqueante durante loop() para detectar caídas de componentes
void checkComponentHealthLoop();

// Reproduce la alarma de 5 segundos con melodía disonante/chunga (Buzzer -> Altavoz -> LEDs)
void playOminousAlarm(uint16_t durationMs = COMPONENT_ALARM_DURATION_MS);

// Dispara la alarma de fallo de componentes con reporte a Serial y Firebase
void triggerComponentFailureAlarm(const ComponentReport& report);

// Consulta del último reporte obtenido
ComponentReport getLastComponentReport();

#endif // COMPONENT_DETECTOR_H
