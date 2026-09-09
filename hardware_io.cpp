#include "hardware_io.h"
#include "config.h"
#include "database.h"
#include "audio_manager.h"
#include "web_server.h"
#include "firebase_client.h"
#include "utils.h"
#include <SPI.h>

MFRC522 mfrc522(RFID_SS_PIN, RFID_RST_PIN);

static String lastUID = "";
static unsigned long lastUIDTime = 0;
const unsigned long UID_DEBOUNCE_MS = 3000;

static unsigned long btnPressStart = 0;
static bool btnWasPressed = false;

void hardwareInit() {
  // Configuración de pines de entrada
  pinMode(BTN_CONFIG_PIN, INPUT_PULLUP);

  // Inicializar bus SPI para lector RFID en los pines asignados
  #if defined(ARDUINO_UNOR4_WIFI)
  SPI.begin();
#else
  SPI.begin(RFID_SCK_PIN, RFID_MISO_PIN, RFID_MOSI_PIN, RFID_SS_PIN);
#endif
  mfrc522.PCD_Init();
  mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_max);
}

void checkConfigButton() {
  bool btnState = (digitalRead(BTN_CONFIG_PIN) == LOW); // Activo a nivel bajo con PULLUP

  if (btnState && !btnWasPressed) {
    btnPressStart = millis();
    btnWasPressed = true;
  } else if (btnState && btnWasPressed) {
    // Si se mantiene pulsado más de 2.5 segundos, alternar modo configuración
    if (millis() - btnPressStart > 2500) {
      btnWasPressed = false; // Evitar disparos múltiples
      if (!isWifiActive) {
        startWifiService(true);
      } else {
        stopWifiService();
      }
    }
  } else if (!btnState && btnWasPressed) {
    // Pulsación corta (< 2.5 s): Probar baliza acústica
    unsigned long pressDuration = millis() - btnPressStart;
    btnWasPressed = false;
    if (pressDuration > 50 && pressDuration < 2000) {
      playAcousticBeacon();
    }
  }
}

void handleRFID() {
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  String uid = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (mfrc522.uid.uidByte[i] < 0x10) uid += "0";
    uid += String(mfrc522.uid.uidByte[i], HEX);
  }
  uid.toUpperCase();

  // Filtro anti-rebote para no procesar la misma tarjeta varias veces consecutivas
  if (uid == lastUID && (millis() - lastUIDTime < UID_DEBOUNCE_MS)) {
    mfrc522.PICC_HaltA();
    return;
  }

  lastUID = uid;
  lastUIDTime = millis();

  // Comprobar si el aula esta en modo bloqueo de emergencia (Lockdown)
  if (lockdownMode) {
    playWarningChime();
    firebasePushLog("BLOQUEO", "Acceso DENEGADO por LOCKDOWN: Tarjeta " + uid, LOG_WARN);
    mfrc522.PICC_HaltA();
    return;
  }

  // Registrar acceso
  dbIncrementAccessCount();

  // Respuesta acústica de confirmación
  playSuccessChime();
  firebasePushLog("ACCESO", "Tarjeta UID: " + uid + " leida en aula " + String(nodeConfig.roomCode));

  // Registrar presencia de estudiante/docente en Firebase
  firebaseRecordStudentPresence(uid, "RFID", 0);

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}

bool checkRFIDConnected() {
  byte v = mfrc522.PCD_ReadRegister(MFRC522::PCD_Register::VersionReg);
  // Si el lector está desconectado o el bus SPI no responde, devuelve 0x00 o 0xFF
  if (v == 0x00 || v == 0xFF) {
    return false;
  }
  return true;
}

