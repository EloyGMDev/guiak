#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include "version.h"

// ════════════════════════════════════════════════════════════════
//  ASIGNACIÓN DE PINES (ESP32-S3 DevKit)
// ════════════════════════════════════════════════════════════════

// ── AUDIO DIGITAL I2S (MAX98357A) ───────────
#define I2S_BCLK_PIN       15    // Bit Clock (BCLK)
#define I2S_LRC_PIN        16    // Word Select / Left-Right Clock (LRC)
#define I2S_DIN_PIN        17    // Serial Data In (DIN)
#define I2S_SD_PIN         18    // Shutdown control (LOW = 0µA Sleep, HIGH = Activo)

// ── LECTOR RFID RC522 (Bus SPI) ─────────────
#define RFID_SS_PIN        10    // Slave Select (SDA)
#define RFID_RST_PIN       9     // Reset
#define RFID_SCK_PIN       12    // SPI Clock
#define RFID_MISO_PIN      13    // SPI MISO
#define RFID_MOSI_PIN      11    // SPI MOSI

// ── ENTRADAS Y SENSORES ──────────────────────
#define BTN_CONFIG_PIN     0     // Botón físico (GPIO 0 / Boot o pulsador a GND)
#define BATTERY_ADC_PIN    4     // Entrada analógica divisor resistivo (ADC1_CH3)
#define LED_STATUS_PIN     48    // LED RGB de estado (GPIO 48 / integrado o externo)

// ════════════════════════════════════════════════════════════════
//  PARÁMETROS DE ENERGÍA Y BATERÍA (Li-Ion 3.7V)
// ════════════════════════════════════════════════════════════════
#define BATTERY_MIN_MV     3300  // 3.3V = 0% batería
#define BATTERY_MAX_MV     4200  // 4.2V = 100% batería
#define BATTERY_LOW_THRESH 20    // 20% umbral de aviso batería baja
#define ADC_R1_OHMS        100000.0f // Divisor resistivo R1 (100k)
#define ADC_R2_OHMS        100000.0f // Divisor resistivo R2 (100k)
#define ADC_DIVIDER_RATIO  ((ADC_R1_OHMS + ADC_R2_OHMS) / ADC_R2_OHMS) // 2.0x

// ════════════════════════════════════════════════════════════════
//  PARÁMETROS DE BALIZA BLUETOOTH (iBeacon / SONA)
// ════════════════════════════════════════════════════════════════
#define DEFAULT_BEACON_UUID "FDA50693-A4E2-4FB1-AFCF-C6EB07647825"
#define DEFAULT_ADV_INTERVAL_MS 500  // Anuncio cada 500 ms (balance ideal batería / respuesta)
#define BEACON_MEASURED_POWER   -59  // Calibración RSSI a 1 metro

// ════════════════════════════════════════════════════════════════
//  ESTRUCTURA DE CONFIGURACIÓN DEL NODO (Almacenada en NVS)
// ════════════════════════════════════════════════════════════════
struct NodeConfig {
  char     roomName[48];        // Ej: "Aula 104 - Informática"
  char     roomCode[12];        // Ej: "104"
  uint16_t floor;               // Planta (ej: 1)
  char     building[32];        // Ej: "Edificio Principal"
  char     beaconUUID[37];      // UUID de la baliza iBeacon
  uint16_t major;               // Major iBeacon (típicamente Planta)
  uint16_t minor;               // Minor iBeacon (típicamente Número de Aula)
  int8_t   measuredPower;       // Potencia de referencia a 1m (-59 dBm)
  uint8_t  volume;              // Nivel de volumen (0 a 100%)
  uint16_t advIntervalMs;       // Intervalo de publicidad BLE en ms (ej: 500)
  char     wifiSSID[32];        // SSID Wi-Fi para modo configuración
  char     wifiPassword[64];    // Contraseña Wi-Fi
  char     adminPassword[32];   // Contraseña panel web ("admin" por defecto)
  uint16_t configTimeoutSec;    // Tiempo para auto-apagar Wi-Fi en modo manual (ej: 300 s = 5 min)

  // ── PROGRAMACIÓN HORARIA DE WI-FI ──────────
  uint8_t  wifiStartHour;       // Hora de encendido matutino (ej: 7 = 07:00)
  uint8_t  wifiStartMin;        // Minuto de encendido (ej: 0)
  uint8_t  wifiEndHour;         // Hora de apagado vespertino (ej: 20 = 20:30)
  uint8_t  wifiEndMin;          // Minuto de apagado (ej: 30)
  bool     wifiScheduleEnabled; // True = respetar horario 07:00 - 20:30
};

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

#endif // CONFIG_H
