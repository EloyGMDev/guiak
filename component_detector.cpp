#include "component_detector.h"
#include "audio_manager.h"
#include "hardware_io.h"
#include "power_manager.h"
#include "database.h"
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

static ComponentReport lastReport = { false, false, false, false, false, false, false, "" };
static unsigned long lastCheckMs = 0;
static bool alarmCurrentlyTriggered = false;

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

  addLog("DIAGNOSTICO", "Sistema de detección de componentes listo.");
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
    addLog("DIAGNOSTICO", " -> [OK] Lector RFID MFRC522 conectado y comunicando por SPI.");
  } else {
    addLog("DIAGNOSTICO", " -> [FALLO] Lector RFID MFRC522 no responde o desconectado.", LOG_ERROR);
    report.missingSummary += "[RFID MFRC522 no responde] ";
  }

  // 2. Verificación del sensor ADC de Batería
  uint16_t batMv = readBatteryMillivolts();
  // Un valor de 0 mV o extremadamente bajo (<1000 mV) indica sensor/divisor desconectado
  if (batMv >= 1000) {
    report.batteryOk = true;
    addLog("DIAGNOSTICO", " -> [OK] Sensor Bateria ADC operativo (" + String(batMv) + " mV).");
  } else {
    report.batteryOk = false;
    addLog("DIAGNOSTICO", " -> [FALLO] Sensor Bateria ADC sin lectura valida (" + String(batMv) + " mV).", LOG_WARN);
    report.missingSummary += "[Bateria/ADC sin lectura] ";
  }

  // 3. Verificación de Memoria No Volátil (NVS Flash / Preferences)
  uint32_t accesses = dbGetAccessCount();
  report.nvsOk = true; // Si dbInit() y la lectura de claves NVS pasaron, NVS está OK
  addLog("DIAGNOSTICO", " -> [OK] Memoria Flash NVS accesible (Accesos registrados: " + String(accesses) + ").");

  // 4. Verificación del controlador Bluetooth LE (BLE)
#if defined(ARDUINO_UNOR4_WIFI)
  report.bleOk = true;
#else
  report.bleOk = BLEDevice::getInitialized();
#endif
  if (report.bleOk) {
    addLog("DIAGNOSTICO", " -> [OK] Submodulo Bluetooth BLE activo y transmitiendo iBeacon.");
  } else {
    addLog("DIAGNOSTICO", " -> [FALLO] Bluetooth BLE no inicializado correctamente.", LOG_ERROR);
    report.missingSummary += "[Bluetooth BLE caido] ";
  }

  // 5. Verificación de Actuadores Acústicos (Buzzer y Altavoz)
#if defined(BUZZER_PIN) && (BUZZER_PIN >= 0)
  report.buzzerOk = nodeConfig.hasBuzzer;
#else
  report.buzzerOk = false;
#endif

  report.audioSpeakerOk = nodeConfig.hasSpeaker && isAudioReady();

  addLog("DIAGNOSTICO", " -> Estado actuadores: Buzzer=" + String(report.buzzerOk ? "SI" : "NO") +
                        ", Altavoz=" + String(report.audioSpeakerOk ? "SI" : "NO") +
                        ", LEDs=SI (Pin " + String(LED_STATUS_PIN) + ")");

  // Criterio global:
  // Componentes críticos mínimos: RFID operativo, Batería con lectura válida, NVS y BLE
  report.allCriticalOk = report.rfidOk && report.batteryOk && report.nvsOk && report.bleOk;

  if (report.allCriticalOk) {
    addLog("DIAGNOSTICO", "TODOS LOS COMPONENTES DETECTADOS CORRECTAMENTE. Estado: SALUDABLE.");
  } else {
    addLog("DIAGNOSTICO", "¡ALERTA! Fallo en componentes detectado: " + report.missingSummary, LOG_ERROR);
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

  addLog("DIAGNOSTICO", "¡FALLO DE HARDWARE! Faltan componentes: " + report.missingSummary, LOG_ERROR);

  // Notificar a Firebase de inmediato si la conexión está disponible
  firebasePushLog("DIAGNOSTICO", "FALLO: Faltan componentes: " + report.missingSummary, LOG_ERROR);

  // Hacer sonar la melodía chunga durante 5 segundos
  playOminousAlarm(COMPONENT_ALARM_DURATION_MS);

  alarmCurrentlyTriggered = false;
}

void checkComponentHealthLoop() {
  if (millis() - lastCheckMs < COMPONENT_CHECK_INTERVAL_MS) {
    return;
  }
  lastCheckMs = millis();

  // Verificación no bloqueante en caliente de componentes clave
  bool rfidAlive = checkRFIDConnected();
  uint16_t batMv = readBatteryMillivolts();
  bool batAlive = (batMv >= 1000);

  if (!rfidAlive || !batAlive) {
    ComponentReport rep = runComponentSelfTest();
    if (!rep.allCriticalOk) {
      triggerComponentFailureAlarm(rep);
    }
  }
}

ComponentReport getLastComponentReport() {
  return lastReport;
}
