#include "firebase_client.h"
#include "config.h"
#include "database.h"
#include "audio_manager.h"
#include "power_manager.h"
#include "utils.h"

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <time.h>

static unsigned long lastSyncCheck = 0;
static unsigned long lastCommandCheck = 0;
const unsigned long SYNC_INTERVAL_MS    = 30000; // Sincronizar config cada 30s
const unsigned long COMMAND_INTERVAL_MS = 5000;  // Comprobar ordenes cada 5s

// Filtro anti-spam de presencia (evita saturar la red si el estudiante se queda quieto delante)
static String lastPresenceStudent = "";
static unsigned long lastPresenceTime = 0;
const unsigned long PRESENCE_COOLDOWN_MS = 15000; // 15 segundos entre actualizaciones del mismo alumno

// ── COLA CIRCULAR OFFLINE ──────────────────────
static QueuedEvent offlineQueue[OFFLINE_QUEUE_SIZE];
static int queueHead  = 0;
static int queueTail  = 0;
static int queueCount = 0;

static void enqueueOfflineLog(const String& tag, const String& msg, uint8_t level) {
  if (queueCount >= OFFLINE_QUEUE_SIZE) {
    queueTail = (queueTail + 1) % OFFLINE_QUEUE_SIZE;
    queueCount--;
  }
  strncpy(offlineQueue[queueHead].tag, tag.c_str(), sizeof(offlineQueue[queueHead].tag) - 1);
  strncpy(offlineQueue[queueHead].msg, msg.c_str(), sizeof(offlineQueue[queueHead].msg) - 1);
  offlineQueue[queueHead].level = level;
  offlineQueue[queueHead].timestamp = millis() / 1000;

  queueHead = (queueHead + 1) % OFFLINE_QUEUE_SIZE;
  queueCount++;
}

static String buildUrl(const String& path) {
  String host = String(nodeConfig.firebaseHost);
  host.trim();
  while (host.endsWith("/")) host.remove(host.length() - 1);
  if (!host.startsWith("http://") && !host.startsWith("https://")) {
    host = "https://" + host;
  }

  String url = host + path + ".json";
  if (strlen(nodeConfig.firebaseAuth) > 0) {
    url += "?auth=" + String(nodeConfig.firebaseAuth);
  }
  return url;
}

void firebaseInit() {
  addLog("FIREBASE", "Cliente Firebase inicializado: " + String(nodeConfig.firebaseHost));
}

void firebasePushLog(const String& tag, const String& message, uint8_t level) {
  if (!nodeConfig.firebaseEnabled) return;

  if (WiFi.status() != WL_CONNECTED) {
    enqueueOfflineLog(tag, message, level);
    return;
  }

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient https;

  String path = "/nodes/" + String(nodeConfig.roomCode) + "/logs";
  String url = buildUrl(path);

  if (https.begin(client, url)) {
    https.addHeader("Content-Type", "application/json");

    const char* lvlStr = "INFO";
    if (level == LOG_DEBUG) lvlStr = "DEBUG";
    else if (level == LOG_WARN)  lvlStr = "WARN";
    else if (level == LOG_ERROR) lvlStr = "ERROR";

    String payload = "{";
    payload += "\"node\":\"" + String(nodeConfig.roomCode) + "\",";
    payload += "\"tag\":\"" + tag + "\",";
    payload += "\"msg\":\"" + message + "\",";
    payload += "\"lvl\":\"" + String(lvlStr) + "\",";
    payload += "\"bat\":" + String(batteryPercent) + ",";
    payload += "\"uptime\":" + String(millis() / 1000);
    payload += "}";

    https.POST(payload);
    https.end();
  }
}

void firebaseRecordStudentPresence(const String& studentId, const String& method, int rssi) {
  if (!nodeConfig.firebaseEnabled) return;

  String cleanId = studentId;
  cleanId.trim();
  cleanId.replace(" ", "_");
  cleanId.replace("/", "_");
  cleanId.replace(".", "_");
  if (cleanId.length() == 0) return;

  // Evitar sobrecarga en Firebase si el móvil o tarjeta sigue cerca de la puerta
  if (cleanId == lastPresenceStudent && (millis() - lastPresenceTime < PRESENCE_COOLDOWN_MS)) {
    return;
  }
  lastPresenceStudent = cleanId;
  lastPresenceTime = millis();

  time_t now = 0;
  time(&now);
  unsigned long timestamp = (now > 1000000) ? (unsigned long)now : (millis() / 1000);

  addLog("PRESENCIA", "Estudiante detectado [" + method + "]: " + cleanId + " en " + String(nodeConfig.roomCode), LOG_INFO);

  if (WiFi.status() != WL_CONNECTED) {
    enqueueOfflineLog("PRESENCE", cleanId + "|" + method + "|" + String(nodeConfig.roomCode), LOG_INFO);
    return;
  }

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient https;

  // 1. Actualizar ficha del estudiante: /students/{cleanId}/lastSeen.json
  String studentPath = "/students/" + cleanId + "/lastSeen";
  String studentUrl  = buildUrl(studentPath);

  if (https.begin(client, studentUrl)) {
    https.addHeader("Content-Type", "application/json");

    String payload = "{";
    payload += "\"studentId\":\"" + cleanId + "\",";
    payload += "\"roomCode\":\"" + String(nodeConfig.roomCode) + "\",";
    payload += "\"roomName\":\"" + String(nodeConfig.roomName) + "\",";
    payload += "\"floor\":" + String(nodeConfig.floor) + ",";
    payload += "\"building\":\"" + String(nodeConfig.building) + "\",";
    payload += "\"timestamp\":" + String(timestamp) + ",";
    payload += "\"method\":\"" + method + "\",";
    payload += "\"rssi\":" + String(rssi);
    payload += "}";

    https.PATCH(payload);
    https.end();
  }

  // 2. Actualizar registro local del aula: /nodes/{roomCode}/presence/{cleanId}.json
  String roomPath = "/nodes/" + String(nodeConfig.roomCode) + "/presence/" + cleanId;
  String roomUrl  = buildUrl(roomPath);

  if (https.begin(client, roomUrl)) {
    https.addHeader("Content-Type", "application/json");

    String payload = "{";
    payload += "\"studentId\":\"" + cleanId + "\",";
    payload += "\"timestamp\":" + String(timestamp) + ",";
    payload += "\"method\":\"" + method + "\",";
    payload += "\"rssi\":" + String(rssi);
    payload += "}";

    https.PATCH(payload);
    https.end();
  }

  // 3. Registrar el evento en el histórico del aula
  firebasePushLog("PRESENCIA", "Estudiante " + cleanId + " registrado mediante " + method, LOG_INFO);
}

void firebaseFlushOfflineQueue() {
  if (queueCount == 0 || WiFi.status() != WL_CONNECTED) return;

  addLog("FIREBASE", "Volcando " + String(queueCount) + " eventos acumulados offline...", LOG_INFO);

  while (queueCount > 0) {
    QueuedEvent ev = offlineQueue[queueTail];
    queueTail = (queueTail + 1) % OFFLINE_QUEUE_SIZE;
    queueCount--;

    firebasePushLog(String(ev.tag) + "_OFFLINE", String(ev.msg), ev.level);
    delay(50);
  }
}

void firebaseUpdateStatus() {
  if (!nodeConfig.firebaseEnabled || WiFi.status() != WL_CONNECTED) return;

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient https;

  String path = "/nodes/" + String(nodeConfig.roomCode) + "/status";
  String url = buildUrl(path);

  if (https.begin(client, url)) {
    https.addHeader("Content-Type", "application/json");

    String payload = "{";
    payload += "\"online\":true,";
    payload += "\"roomName\":\"" + String(nodeConfig.roomName) + "\",";
    payload += "\"floor\":" + String(nodeConfig.floor) + ",";
    payload += "\"battery\":" + String(batteryPercent) + ",";
    payload += "\"mv\":" + String(batteryMillivolts) + ",";
    payload += "\"rssi\":" + String(WiFi.RSSI()) + ",";
    payload += "\"lockdown\":" + String(lockdownMode ? "true" : "false") + ",";
    payload += "\"uptime\":" + String(millis() / 1000);
    payload += "}";

    https.PATCH(payload);
    https.end();
  }
}

void firebaseSyncConfig() {
  if (!nodeConfig.firebaseEnabled || WiFi.status() != WL_CONNECTED) return;

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient https;

  String path = "/nodes/" + String(nodeConfig.roomCode) + "/config";
  String url = buildUrl(path);

  if (https.begin(client, url)) {
    int code = https.GET();
    if (code == 200) {
      String json = https.getString();
      if (json.length() > 5 && json != "null") {
        auto extractStr = [](const String& src, const String& key) -> String {
          int idx = src.indexOf("\"" + key + "\":");
          if (idx == -1) return "";
          int start = src.indexOf("\"", idx + key.length() + 2) + 1;
          int end = src.indexOf("\"", start);
          if (start <= 0 || end == -1) return "";
          return src.substring(start, end);
        };

        auto extractInt = [](const String& src, const String& key) -> int {
          int idx = src.indexOf("\"" + key + "\":");
          if (idx == -1) return -1;
          int start = idx + key.length() + 2;
          while (start < src.length() && (src[start] == ' ' || src[start] == ':')) start++;
          int end = start;
          while (end < src.length() && isdigit(src[end])) end++;
          return src.substring(start, end).toInt();
        };

        String newName = extractStr(json, "roomName");
        String newBld  = extractStr(json, "building");
        int newFloor   = extractInt(json, "floor");
        int newVol     = extractInt(json, "volume");

        bool changed = false;
        if (newName.length() > 0 && String(nodeConfig.roomName) != newName) {
          newName.toCharArray(nodeConfig.roomName, sizeof(nodeConfig.roomName));
          changed = true;
        }
        if (newBld.length() > 0 && String(nodeConfig.building) != newBld) {
          newBld.toCharArray(nodeConfig.building, sizeof(nodeConfig.building));
          changed = true;
        }
        if (newFloor >= 0 && nodeConfig.floor != newFloor) {
          nodeConfig.floor = newFloor;
          changed = true;
        }
        if (newVol >= 0 && newVol <= 100 && nodeConfig.volume != newVol) {
          nodeConfig.volume = newVol;
          changed = true;
        }

        if (changed) {
          dbSaveConfig();
          addLog("FIREBASE", "Configuracion remota adaptada con exito", LOG_INFO);
          firebasePushLog("CONFIG", "Nodo adaptado a nueva configuracion remota");
        }
      }
    }
    https.end();
  }
}

void firebaseCheckCommands() {
  if (!nodeConfig.firebaseEnabled || WiFi.status() != WL_CONNECTED) return;

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient https;

  String path = "/nodes/" + String(nodeConfig.roomCode) + "/command";
  String url = buildUrl(path);

  if (https.begin(client, url)) {
    int code = https.GET();
    if (code == 200) {
      String cmd = https.getString();
      cmd.trim();
      cmd.replace("\"", "");

      if (cmd.length() > 0 && cmd != "null" && cmd != "idle") {
        addLog("COMANDO", "Orden remota recibida: " + cmd, LOG_WARN);

        if (cmd == "trigger_sound" || cmd == "sound") {
          playAcousticBeacon();
          firebasePushLog("COMANDO", "Baliza acustica ejecutada por orden remota");
        } else if (cmd == "lockdown_on") {
          lockdownMode = true;
          playWarningChime();
          firebasePushLog("SEGURIDAD", "Bloqueo de emergencia (Lockdown) ACTIVADO", LOG_WARN);
        } else if (cmd == "lockdown_off") {
          lockdownMode = false;
          playSuccessChime();
          firebasePushLog("SEGURIDAD", "Bloqueo de emergencia DESACTIVADO", LOG_INFO);
        } else if (cmd == "reboot") {
          firebasePushLog("SISTEMA", "Reinicio remoto solicitado");
          delay(500);
          ESP.restart();
        }

        https.PUT("\"idle\"");
      }
    }
    https.end();
  }
}

void firebaseLoop() {
  if (!isWifiActive || WiFi.status() != WL_CONNECTED || !nodeConfig.firebaseEnabled) {
    return;
  }

  // 1. Volcar eventos pendientes offline
  if (queueCount > 0) {
    firebaseFlushOfflineQueue();
  }

  // 2. Comprobar ordenes remotas cada 5s
  if (millis() - lastCommandCheck > COMMAND_INTERVAL_MS) {
    lastCommandCheck = millis();
    firebaseCheckCommands();
  }

  // 3. Comprobar actualizacion de configuracion y estado cada 30s
  if (millis() - lastSyncCheck > SYNC_INTERVAL_MS) {
    lastSyncCheck = millis();
    firebaseSyncConfig();
    firebaseUpdateStatus();
  }
}
