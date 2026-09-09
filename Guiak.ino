/*
 * ════════════════════════════════════════════════════════════════
 *  Guiak System v3.0 (guiak.com) — Nodo de Navegación Asistida para Aulas
 *  Plataforma: ESP32-S3 (Ultra-Low Power a Batería)
 * ════════════════════════════════════════════════════════════════
 *  - Audio Digital I2S de alta definición (MAX98357A con corte a 0 µA)
 *  - Baliza iBeacon + Servidor GATT BLE (activación acústica remota)
 *  - Horario Wi-Fi Escolar Inteligente (Activo 07:00 - 20:30 / Noche apagado)
 *  - Wi-Fi Modem Sleep para reducir consumo durante el día
 *  - Monitorización de batería Li-Ion por ADC
 *  - Lector RFID de proximidad anti-rebote (RC522)
 *  - Almacenamiento persistente en NVS (Preferences)
 * ════════════════════════════════════════════════════════════════
 */

#include "config.h"
#include "version.h"
#include "utils.h"
#include "database.h"
#include "power_manager.h"
#include "audio_manager.h"
#include "ble_beacon.h"
#include "hardware_io.h"
#include "web_server.h"
#include "firebase_client.h"
#include "component_detector.h"

#if defined(ARDUINO_UNOR4_WIFI)
#include <ArduinoBLE.h>
#include <WiFiS3.h>
#endif

static unsigned long lastBatteryCheck = 0;
const unsigned long BATTERY_CHECK_INTERVAL = 30000; // Cada 30 segundos

void setup() {
  Serial.begin(115200);
  delay(300);

#if defined(ARDUINO_UNOR4_WIFI)
  addLog("SISTEMA", "Iniciando Guiak Node v" + String(FW_VERSION) + " en Arduino UNO R4 WiFi...");
#else
  addLog("SISTEMA", "Iniciando Guiak Node v" + String(FW_VERSION) + " en ESP32-S3...");
#endif

  // 1. Inicializar almacenamiento persistente NVS
  dbInit();
  addLog("SISTEMA", "Aula: " + String(nodeConfig.roomName) + " (Codigo: " + String(nodeConfig.roomCode) + ")");
  char schedBuf[32];
  snprintf(schedBuf, sizeof(schedBuf), "%02d:%02d a %02d:%02d", nodeConfig.wifiStartHour, nodeConfig.wifiStartMin, nodeConfig.wifiEndHour, nodeConfig.wifiEndMin);
  addLog("SISTEMA", "Horario Wi-Fi configurado: " + String(schedBuf));

  // 2. Inicializar sistema de energía y lectura de batería
  powerInit();
  addLog("ENERGIA", "Bateria inicial: " + String(batteryPercent) + "% (" + String(batteryMillivolts) + " mV)");

  // 3. Inicializar subsistema de audio digital I2S
  audioInit();

  // 4. Inicializar periféricos (Botón de configuración y Lector RFID)
  hardwareInit();

  // 5. Arrancar baliza Bluetooth BLE (iBeacon continuo)
  bleBeaconInit();
  addLog("BLE", "Baliza iBeacon activa (Intervalo: " + String(nodeConfig.advIntervalMs) + " ms)");

  // 6. Autodiagnóstico y Detección de Componentes Hardware (POST)
  componentDetectorInit();
  ComponentReport compReport = runComponentSelfTest();
  if (!compReport.allCriticalOk) {
    // 1. Dispara la melodía de aviso de 5 segundos (Buzzer -> Altavoz -> LEDs)
    triggerComponentFailureAlarm(compReport);
    // 2. El sistema se adapta automáticamente e inicia el programa con los componentes disponibles
    adaptSystemToAvailableHardware(compReport);
  } else {
    // Si todos los componentes están saludables, emite el tono armónico de bienvenida
    playArrivalChime();
  }

  // 7. Arrancar servicio Wi-Fi según el horario escolar (07:00 a 20:30)
  // Si el botón físico está pulsado en el arranque, forzar encendido manual inmediato
  if (digitalRead(BTN_CONFIG_PIN) == LOW) {
    addLog("SISTEMA", "Boton presionado al arrancar -> Forzando Wi-Fi manual...");
    startWifiService(true);
  } else {
    wifiServiceInit();
  }

  addLog("SISTEMA", "Nodo Guiak iniciado con exito (" + String(isAdaptiveModeActive ? "Modo Adaptado Resiliente" : "Modo Completo") + "). RAM libre: " + String(getFreeHeapBytes() / 1024) + " KB");
}

void loop() {
  // 0. Procesar eventos Bluetooth en tiempo real con latencia cero
  bleLoop();

  // 1. Comprobar pulsación del botón físico (conmutar Wi-Fi manual)
  checkConfigButton();

  // 2. Gestionar servicio Wi-Fi y horario escolar (07:00 a 20:30)
  handleWifiService();

  // 3. Sincronizacion en tiempo real con Firebase
  firebaseLoop();

  // 4. Escuchar tarjetas RFID de paso
  handleRFID();

  // 5. Monitorización periódica del nivel de batería
  if (millis() - lastBatteryCheck > BATTERY_CHECK_INTERVAL) {
    lastBatteryCheck = millis();
    updateBatteryStatus();
    bleUpdateBatteryLevel(batteryPercent);

    if (isBatteryLow()) {
      addLog("BATERIA", "AVISO: Bateria baja (" + String(batteryPercent) + "%)", LOG_WARN);
      firebasePushLog("BATERIA", "AVISO: Bateria baja (" + String(batteryPercent) + "%)", LOG_WARN);
    }
  }

  // 6. Supervisión y detección continua de componentes
  checkComponentHealthLoop();

  // 7. Ceder ciclo de CPU sin retardos perceptibles
  delay(1);
}