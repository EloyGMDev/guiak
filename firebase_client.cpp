#include "firebase_client.h"
#include "config.h"
#include "database.h"
#include "power_manager.h"
#include "utils.h"

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

static unsigned long lastSyncCheck = 0;
const unsigned long SYNC_CHECK_INTERVAL = 30000; // Comprobar cambios cada 30 segundos
static bool isSyncing = false;

void firebaseInit() {
  addLog("FIREBASE", "Cliente Firebase inicializado para: " + String(nodeConfig.firebaseHost));
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

void firebasePushLog(const String& tag, const String& message, uint8_t level) {
  if (!nodeConfig.firebaseEnabled || WiFi.status() != WL_CONNECTED) {
    return;
  }

  WiFiClientSecure client;
  client.setInsecure(); // No validar cadena de certificados SSL para conexion directa ligera
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
    payload += ""node":"" + String(nodeConfig.roomCode) + "",";
    payload += ""tag":"" + tag + "",";
    payload += ""msg":"" + message + "",";
    payload += ""lvl":"" + String(lvlStr) + "",";
    payload += ""bat":" + String(batteryPercent) + ",";
    payload += ""uptime":" + String(millis() / 1000);
    payload += "}";

    int code = https.POST(payload);
    https.end();
  }
}

void firebaseUpdateStatus() {
  if (!nodeConfig.firebaseEnabled || WiFi.status() != WL_CONNECTED) {
    return;
  }

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient https;

  String path = "/nodes/" + String(nodeConfig.roomCode) + "/status";
  String url = buildUrl(path);

  if (https.begin(client, url)) {
    https.addHeader("Content-Type", "application/json");

    String payload = "{";
    payload += ""online":true,";
    payload += ""roomName":"" + String(nodeConfig.roomName) + "",";
    payload += ""floor":" + String(nodeConfig.floor) + ",";
    payload += ""battery":" + String(batteryPercent) + ",";
    payload += ""mv":" + String(batteryMillivolts) + ",";
    payload += ""rssi":" + String(WiFi.RSSI()) + ",";
    payload += ""uptime":" + String(millis() / 1000);
    payload += "}";

    https.PATCH(payload);
    https.end();
  }
}

void firebaseSyncConfig() {
  if (!nodeConfig.firebaseEnabled || WiFi.status() != WL_CONNECTED || isSyncing) {
    return;
  }

  isSyncing = true;
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
  isSyncing = false;
}

void firebaseLoop() {
  if (!isWifiActive || WiFi.status() != WL_CONNECTED || !nodeConfig.firebaseEnabled) {
    return;
  }

  if (millis() - lastSyncCheck > SYNC_CHECK_INTERVAL) {
    lastSyncCheck = millis();
    firebaseSyncConfig();
    firebaseUpdateStatus();
  }
}
