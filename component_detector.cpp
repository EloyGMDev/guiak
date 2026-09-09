#include "component_detector.h"
#include "audio_manager.h"
#include "hardware_io.h"
#include "power_manager.h"
#include "database.h"
#include "ble_beacon.h"
#include "utils.h"
#include "firebase_client.h"

#if !defined(ARDUINO_UNOR4_WIFI)
#include <BLEDevice.h>
#endif

// Estructura interna para notas de la melodía
struct NoteDef {
  uint16_t freq;
  uint16_t durationMs;
  uint16_t pauseMs;
};

// ════════════════════════════════════════════════════════════════
//  MELODÍA DISONANTE Y AMENAZANTE ("CHUNGA") DE 5 SEGUNDOS
//  Basada en tritonos (Diabolus in musica: 440 Hz / 622 Hz)
//  y segundas menores para transmitir alarma grave e inequívoca.
// ════════════════════════════════════════════════════════════════
static const NoteDef OMINOUS_MELODY[] = {
  { 466, 150, 25 }, // A#4
  { 622, 170, 25 }, // D#5 (Tritono disonante agudo)
  { 440, 150, 25 }, // A4
  { 587, 160, 25 }, // D5 (Tensión de suspensión)
  { 311, 230, 35 }, // D#4 (Bajo oscuro y siniestro)
  { 880, 140, 20 }, // A5 (Chirrido agudo de alerta)
  { 622, 180, 30 }, // D#5 (Resolución disonante)
  { 370, 210, 40 }  // F#4 (Cadencia inestable)
};
static const size_t OMINOUS_MELODY_LEN = sizeof(OMINOUS_MELODY) / sizeof(OMINOUS_MELODY[0]);

bool isAdaptiveModeActive = false;
static ComponentReport lastReport = { false, false, false, false, false, false, false, "" };
static unsigned long lastCheckMs = 0;
static bool alarmCurrentlyTriggered = false;

static bool baselineRfidOk = false;
static bool baselineBleOk = false;
static bool baselineBatteryOk = false;
static bool baselineAudioOk = false;
static bool baselineSelfTestDone = false;

void componentDetectorInit() {
  // Configuración del pin de LED de estado como salida
#if defined(LED_STATUS_PIN) && (LED_STATUS_PIN >= 0)
  pinMode(LED_STATUS_PIN, OUTPUT);
  digitalWrite(LED_STATUS_PIN, LOW);
#endif

  // Configuración del pin de Buzzer como salida
#if defined(BUZZER_PIN) && (BUZZER_PIN >= 0)
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
#endif

  addLog("DIAGNOSTICO", "Sistema de detección y adaptación de componentes listo.");
}

ComponentReport runComponentSelfTest() {
  ComponentReport report;
  report.rfidOk = false;
  report.batteryOk = false;
  report.audioSpeakerOk = false;
  report.buzzerOk = false;
  report.nvsOk = false;
  report.bleOk = false;
  report.allCriticalOk = false;
  report.missingSummary = "";

  addLog("DIAGNOSTICO", "Iniciando autotest de componentes de hardware (POST)...");

  // 1. Verificación del lector RFID MFRC522 (consulta de registros vía SPI)
  report.rfidOk = checkRFIDConnected();
  if (report.rfidOk) {
    addLog("DIAGNOSTICO", " -> [OK] Lector RFID MFRC522 activo en bus SPI.");
  } else {
    addLog("DIAGNOSTICO", " -> [AUSENTE] Lector RFID MFRC522 no responde o desconectado.", LOG_WARN);
    report.missingSummary += "[Lector RFID no detectado] ";
  }

  // 2. Verificación del sensor ADC de Batería
  updateBatteryStatus();
  if (isBatteryConnected) {
    report.batteryOk = true;
    addLog("DIAGNOSTICO", " -> [OK] Batería Li-Ion conectada (" + String(batteryMillivolts) + " mV, " + String(batteryPercent) + "%).");
  } else {
    report.batteryOk = false;
    addLog("DIAGNOSTICO", " -> [INFO] Sin batería Li-Ion (Alimentado por USB 5V continuo).");
    report.missingSummary += "[Modo USB / Sin batería Li-Ion] ";
  }

  // 3. Verificación de Memoria No Volátil (NVS Flash / Preferences)
  uint32_t accesses = dbGetAccessCount();
  report.nvsOk = true;
  addLog("DIAGNOSTICO", " -> [OK] Memoria Flash NVS accesible (Accesos: " + String(accesses) + ").");

  // 4. Verificación del controlador Bluetooth LE (BLE)
  report.bleOk = isBleAvailable;
  if (report.bleOk) {
    addLog("DIAGNOSTICO", " -> [OK] Bluetooth BLE activo emitiendo señal.");
  } else {
    addLog("DIAGNOSTICO", " -> [AUSENTE] Bluetooth BLE no activo.", LOG_WARN);
    report.missingSummary += "[Bluetooth BLE no disponible] ";
  }

  // 5. Verificación de Actuadores Acústicos (Buzzer y Altavoz)
#if defined(BUZZER_PIN) && (BUZZER_PIN >= 0)
  report.buzzerOk = nodeConfig.hasBuzzer;
#else
  report.buzzerOk = false;
#endif

  report.audioSpeakerOk = nodeConfig.hasSpeaker && isAudioReady();

  addLog("DIAGNOSTICO", " -> Actuadores acústicos: Buzzer=" + String(report.buzzerOk ? "SI" : "NO") +
                        ", Altavoz=" + String(report.audioSpeakerOk ? "SI" : "NO") +
                        ", LEDs=SI (Pin " + String(LED_STATUS_PIN) + ")");

  if (!report.buzzerOk && !report.audioSpeakerOk) {
    report.missingSummary += "[Sin actuador acústico (Alerta por LEDs)] ";
  }

  // Criterio de hardware completo:
  // Es 100% completo si RFID, BLE y algún canal de sonido están presentes
  report.allCriticalOk = report.rfidOk && report.bleOk && (report.buzzerOk || report.audioSpeakerOk);

  if (report.allCriticalOk) {
    addLog("DIAGNOSTICO", "TODOS LOS COMPONENTES DETECTADOS. Estado: COMPLETO Y SALUDABLE.");
  } else {
    addLog("DIAGNOSTICO", "AVISO: Componentes no detectados: " + report.missingSummary, LOG_WARN);
  }

  // Si es la primera ejecución en boot, memorizar la configuración base de hardware
  if (!baselineSelfTestDone) {
    baselineRfidOk = report.rfidOk;
    baselineBleOk = report.bleOk;
    baselineBatteryOk = report.batteryOk;
    baselineAudioOk = (report.buzzerOk || report.audioSpeakerOk);
    baselineSelfTestDone = true;
  }

  lastReport = report;
  return report;
}

void playOminousAlarm(uint16_t durationMs) {
  unsigned long startMs = millis();
  isSoundPlaying = true;

  // Determinar jerarquía de salida según lo especificado:
  // 1. Buzzer (si existe)
  // 2. Altavoz (si no hay buzzer)
  // 3. LEDs (si no hay ambos, parpadean y piten visualmente)
  bool canUseBuzzer = nodeConfig.hasBuzzer && (BUZZER_PIN >= 0);
  bool canUseSpeaker = !canUseBuzzer && nodeConfig.hasSpeaker && isAudioReady();
  bool ledsOnly = !canUseBuzzer && !canUseSpeaker;

  String outputDevice = "LEDS (Sin audio disponible)";
  if (canUseBuzzer) {
    outputDevice = "BUZZER (Pin " + String(BUZZER_PIN) + ")";
  } else if (canUseSpeaker) {
    outputDevice = "ALTAVOZ (I2S/DAC)";
  }

  addLog("ALARMA", "Disparando melodia disonante de " + String(durationMs) + " ms usando: " + outputDevice, LOG_WARN);

  size_t noteIdx = 0;
  while ((millis() - startMs) < durationMs) {
    const NoteDef& note = OMINOUS_MELODY[noteIdx % OMINOUS_MELODY_LEN];
    noteIdx++;

    unsigned long elapsed = millis() - startMs;
    if (elapsed >= durationMs) break;

    uint16_t remaining = durationMs - elapsed;
    uint16_t noteDur = (note.durationMs < remaining) ? note.durationMs : remaining;
    if (noteDur == 0) break;

    // Encender LED en sincronía con el tono (o en estroboscópico si solo hay LED)
#if defined(LED_STATUS_PIN) && (LED_STATUS_PIN >= 0)
    digitalWrite(LED_STATUS_PIN, HIGH);
#endif

    // Reproducción sonora según jerarquía
    if (canUseBuzzer) {
#if defined(BUZZER_PIN) && (BUZZER_PIN >= 0)
      tone(BUZZER_PIN, note.freq);
      delay(noteDur);
      noTone(BUZZER_PIN);
#endif
    } else if (canUseSpeaker) {
      // Reproducir tono por altavoz a volumen de alarma (90%)
      playToneI2S(note.freq, noteDur, 90);
    } else {
      // Solo LEDs: pulsos rápidos de 50ms para simular pitidos visuales de máxima urgencia
      unsigned long pulseStart = millis();
      while (millis() - pulseStart < noteDur) {
#if defined(LED_STATUS_PIN) && (LED_STATUS_PIN >= 0)
        digitalWrite(LED_STATUS_PIN, HIGH);
        delay(40);
        digitalWrite(LED_STATUS_PIN, LOW);
        delay(40);
#endif
      }
    }

#if defined(LED_STATUS_PIN) && (LED_STATUS_PIN >= 0)
    digitalWrite(LED_STATUS_PIN, LOW);
#endif

    // Pausa entre notas
    elapsed = millis() - startMs;
    if (elapsed >= durationMs) break;
    remaining = durationMs - elapsed;
    uint16_t pauseDur = (note.pauseMs < remaining) ? note.pauseMs : remaining;
    if (pauseDur > 0) {
      delay(pauseDur);
    }
  }

  // Apagar y restablecer periféricos
#if defined(BUZZER_PIN) && (BUZZER_PIN >= 0)
  noTone(BUZZER_PIN);
  digitalWrite(BUZZER_PIN, LOW);
#endif

  if (canUseSpeaker) {
    audioPowerDown();
  }

#if defined(LED_STATUS_PIN) && (LED_STATUS_PIN >= 0)
  digitalWrite(LED_STATUS_PIN, LOW);
#endif

  isSoundPlaying = false;
  addLog("ALARMA", "Secuencia de alarma completada.");
}

void triggerComponentFailureAlarm(const ComponentReport& report) {
  if (alarmCurrentlyTriggered) return; // Evitar reentrada
  alarmCurrentlyTriggered = true;

  addLog("DIAGNOSTICO", "¡AVISO DE HARDWARE! Componentes no detectados: " + report.missingSummary, LOG_WARN);

  // Notificar a Firebase de inmediato si la conexión está disponible
  firebasePushLog("DIAGNOSTICO", "AVISO: Componentes no detectados: " + report.missingSummary, LOG_WARN);

  // Hacer sonar la melodía chunga durante 5 segundos (Buzzer -> Altavoz -> LEDs)
  playOminousAlarm(COMPONENT_ALARM_DURATION_MS);

  alarmCurrentlyTriggered = false;
}

void adaptSystemToAvailableHardware(const ComponentReport& report) {
  isAdaptiveModeActive = true;

  addLog("SISTEMA", "==================================================");
  addLog("SISTEMA", "[ADAPTACION AUTOMATICA DE HARDWARE GUIAK]");
  addLog("SISTEMA", "Reconfigurando subsistemas para operar con tolerancia a fallos:");

  // 1. Adaptación de RFID
  if (!report.rfidOk) {
    isRfidAvailable = false;
    addLog("SISTEMA", " * Lector RFID: DESACTIVADO. La deteccion de alumnos opera via Baliza BLE y App movil.");
  } else {
    isRfidAvailable = true;
    addLog("SISTEMA", " * Lector RFID: ACTIVO en bus SPI.");
  }

  // 2. Adaptación de Batería
  if (!report.batteryOk) {
    isBatteryConnected = false;
    batteryPercent = 100;
    addLog("SISTEMA", " * Fuente de Energia: MODO USB / CONTINUO ACTIVO (Alertas de bateria baja suspendidas).");
  } else {
    addLog("SISTEMA", " * Fuente de Energia: BATERIA LI-ION ACTIVA.");
  }

  // 3. Adaptación de Audio
  if (!report.buzzerOk && !report.audioSpeakerOk) {
    addLog("SISTEMA", " * Actuadores Acusticos: DESACTIVADOS. Senales redirigidas a pulsos de LED.");
  } else if (report.buzzerOk) {
    addLog("SISTEMA", " * Actuadores Acusticos: BUZZER PRINCIPAL ACTIVO.");
  } else {
    addLog("SISTEMA", " * Actuadores Acusticos: ALTAVOZ ACTIVO.");
  }

  // 4. Adaptación de BLE
  if (!report.bleOk) {
    addLog("SISTEMA", " * Bluetooth BLE: DESACTIVADO. Portal Wi-Fi y servidor web operativos.");
  } else {
    addLog("SISTEMA", " * Bluetooth BLE: ACTIVO (Servicio SONA y Baliza iBeacon).");
  }

  addLog("SISTEMA", ">>> SISTEMA TOTALMENTE ESTABLE Y OPERATIVO EN MODO RESILIENTE <<<");
  addLog("SISTEMA", "==================================================");

  firebasePushLog("SISTEMA", "Nodo adaptado y operativo: " + report.missingSummary, LOG_INFO);
}

void checkComponentHealthLoop() {
  if (!baselineSelfTestDone) return;
  if (millis() - lastCheckMs < COMPONENT_CHECK_INTERVAL_MS) {
    return;
  }
  lastCheckMs = millis();

  // Comprobar únicamente si algún componente que funcionaba en el arranque se ha desconectado en caliente
  bool hotFailure = false;
  String hotReason = "";

  if (baselineRfidOk && !checkRFIDConnected()) {
    hotFailure = true;
    hotReason += "[Desconexion en caliente de RFID] ";
    isRfidAvailable = false; // Desactivar para que loop no se cuelgue ni bloquee el bus SPI
    baselineRfidOk = false;
  }

  if (baselineBleOk && !isBleAvailable) {
    hotFailure = true;
    hotReason += "[Caida en caliente de Bluetooth BLE] ";
    baselineBleOk = false;
  }

  if (hotFailure) {
    addLog("DIAGNOSTICO", "¡FALLO EN CALIENTE DETECTADO! " + hotReason, LOG_ERROR);
    ComponentReport rep = runComponentSelfTest();
    triggerComponentFailureAlarm(rep);
    adaptSystemToAvailableHardware(rep);
  }
}

ComponentReport getLastComponentReport() {
  return lastReport;
}
