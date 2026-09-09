#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include "version.h"

// ════════════════════════════════════════════════════════════════
//  ABSTRACCIÓN DE HARDWARE (ESP32-S3 vs Arduino UNO R4 WiFi)
// ════════════════════════════════════════════════════════════════
#if defined(ARDUINO_UNOR4_WIFI)
  // ── PINES ARDUINO UNO R4 WIFI ──────────────
  #define AUDIO_PIN          A0    // Pin DAC 12-bit / Salida analógica (o Pin 5 Buzzer)
  #define BUZZER_PIN         5     // Pin alternativo para buzzer piezoeléctrico
  #define RFID_SS_PIN        10    // Slave Select (SDA)
  #define RFID_RST_PIN       9     // Reset
  #define RFID_SCK_PIN       13    // SPI Clock (D13)
  #define RFID_MISO_PIN      12    // SPI MISO (D12)
  #define RFID_MOSI_PIN      11    // SPI MOSI (D11)
  #define BTN_CONFIG_PIN     7     // Botón manual (D7 con pull-up)
  #define BATTERY_ADC_PIN    A1    // Entrada analógica batería
  #define LED_STATUS_PIN     13    // LED integrado
#else
  // ── PINES NATIVOS ESP32-S3 DEVKIT ──────────
  #define I2S_BCLK_PIN       15    // Bit Clock (BCLK)
  #define I2S_LRC_PIN        16    // Word Select (LRC)
  #define I2S_DIN_PIN        17    // Serial Data In (DIN)
  #define I2S_SD_PIN         18    // Shutdown amplificador (LOW = 0µA Sleep)
  #define BUZZER_PIN         47    // Pin alternativo para buzzer piezoeléctrico
  #define RFID_SS_PIN        10    // Slave Select (SDA)
  #define RFID_RST_PIN       9     // Reset
  #define RFID_SCK_PIN       12    // SPI Clock
  #define RFID_MISO_PIN      13    // SPI MISO
  #define RFID_MOSI_PIN      11    // SPI MOSI
  #define BTN_CONFIG_PIN     0     // Botón físico (GPIO 0 / Boot o GPIO 7)
  #define BATTERY_ADC_PIN    4     // Entrada analógica divisor resistivo (ADC1_CH3)
  #define LED_STATUS_PIN     48    // LED RGB integrado o externo
#endif

// ════════════════════════════════════════════════════════════════
//  PARÁMETROS DE ENERGÍA Y BATERÍA
// ════════════════════════════════════════════════════════════════
#define BATTERY_MIN_MV     3300  // 3.3V = 0% batería
#define BATTERY_MAX_MV     4200  // 4.2V = 100% batería
#define BATTERY_LOW_THRESH 20    // 20% umbral de aviso batería baja
#define ADC_DIVIDER_RATIO  2.0f  // Divisor resistivo 100k/100k

// ════════════════════════════════════════════════════════════════
//  PARÁMETROS DE BALIZA BLUETOOTH (iBeacon / Guiak)
// ════════════════════════════════════════════════════════════════
#define DEFAULT_BEACON_UUID "FDA50693-A4E2-4FB1-AFCF-C6EB07647825"
#define DEFAULT_ADV_INTERVAL_MS 100  // Anuncio a alta frecuencia (100 ms = 10 Hz) para respuesta instantánea sin delay
#define BEACON_MEASURED_POWER   -58  // Calibración estándar RSSI a 1 metro

// ════════════════════════════════════════════════════════════════
//  COLA DE EVENTOS OFFLINE (RESILIENCIA SIN CONEXIÓN)
// ════════════════════════════════════════════════════════════════
#define OFFLINE_QUEUE_SIZE 25

struct QueuedEvent {
  char     tag[16];
  char     msg[64];
  uint8_t  level;
  uint32_t timestamp;
};

// ════════════════════════════════════════════════════════════════
//  ESTRUCTURA DE CONFIGURACIÓN DEL NODO
// ════════════════════════════════════════════════════════════════
struct NodeConfig {
  char     roomName[48];        // Ej: "Aula 104 - Informatica"
  char     roomCode[12];        // Ej: "104"
  uint16_t floor;               // Planta (ej: 1)
  char     building[32];        // Ej: "Edificio Principal"
  char     beaconUUID[37];      // UUID de la baliza iBeacon
  uint16_t major;               // Major iBeacon (Planta)
  uint16_t minor;               // Minor iBeacon (Número de Aula)
  int8_t   measuredPower;       // Potencia de referencia a 1m (-59 dBm)
  uint8_t  volume;              // Nivel de volumen (0 a 100%)
  uint16_t advIntervalMs;       // Intervalo de publicidad BLE en ms (ej: 500)
  char     wifiSSID[32];        // SSID Wi-Fi
  char     wifiPassword[64];    // Contraseña Wi-Fi
  char     adminPassword[32];   // Contraseña panel web ("admin" por defecto)
  uint16_t configTimeoutSec;    // Tiempo para auto-apagar Wi-Fi en modo manual (ej: 300 s = 5 min)

  // ── PROGRAMACIÓN HORARIA DE WI-FI ──────────
  uint8_t  wifiStartHour;       // Hora de encendido matutino (ej: 7 = 07:00)
  uint8_t  wifiStartMin;        // Minuto de encendido (ej: 0)
  uint8_t  wifiEndHour;         // Hora de apagado vespertino (ej: 20 = 20:30)
  uint8_t  wifiEndMin;          // Minuto de apagado (ej: 30)
  bool     wifiScheduleEnabled; // True = respetar horario 07:00 - 20:30

  // ── INTEGRACIÓN FIREBASE ───────────────────
  char     firebaseHost[96];    // URL de Firebase Realtime Database
  char     firebaseAuth[64];    // Auth token / secreto
  bool     firebaseEnabled;     // True = sincronizar con Firebase

  // ── ACTUADORES Y HARDWARE DISPONIBLE ───────
  bool     hasBuzzer;           // True = zumbador/buzzer piezoeléctrico disponible
  bool     hasSpeaker;          // True = altavoz (I2S MAX98357A / DAC) disponible
};

// ════════════════════════════════════════════════════════════════
//  PARÁMETROS DEL SISTEMA DE DETECCIÓN Y DIAGNÓSTICO
// ════════════════════════════════════════════════════════════════
#define COMPONENT_CHECK_INTERVAL_MS  20000 // Comprobación en bucle cada 20 segundos
#define COMPONENT_ALARM_DURATION_MS  5000  // Alarma de 5 segundos si falta algún componente

// ════════════════════════════════════════════════════════════════
//  ESTADO Y VARIABLES GLOBALES
// ════════════════════════════════════════════════════════════════
extern NodeConfig nodeConfig;
extern bool isWifiActive;              // True si la radio Wi-Fi está encendida
extern bool isConfigMode;              // True si el servidor web está activo
extern bool isManualWifiOverride;      // True si se forzó encendido manual fuera de horario
extern unsigned long manualOverrideStart; // Marca de tiempo de encendido manual
extern uint8_t batteryPercent;         // 0 - 100%
extern uint16_t batteryMillivolts;     // Ej: 3850 mV
extern bool isSoundPlaying;            // True mientras el amplificador reproduce audio
extern bool lockdownMode;              // True si el centro ordenó bloqueo de emergencia remoto

#endif // CONFIG_H
