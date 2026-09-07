#include "web_server.h"
#include "config.h"
#include "database.h"
#include "audio_manager.h"
#include "power_manager.h"
#include "ble_beacon.h"
#include "ntp_sync.h"
#include "utils.h"

#include <WiFi.h>
#include <WebServer.h>
#include <esp_wifi.h>
#include <time.h>

static WebServer server(80);
static bool serverRunning = false;
static unsigned long lastScheduleCheck = 0;
const unsigned long SCHEDULE_CHECK_INTERVAL = 15000; // Comprobar cada 15 segundos

static const char HTML_CONFIG_PAGE[] PROGMEM = R"rawhtml(
<!DOCTYPE html>
<html lang="es">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Guiak - Portal de Configuración (guiak.com)</title>
  <link href="https://fonts.googleapis.com/css2?family=Outfit:wght@300;400;600;700&display=swap" rel="stylesheet">
  <style>
    :root {
      --bg: #090b10;
      --card: rgba(255, 255, 255, 0.04);
      --border: rgba(255, 255, 255, 0.08);
      --accent: #00d2ff;
      --accent-glow: rgba(0, 210, 255, 0.25);
      --success: #00ff88;
      --warning: #ffb703;
      --text: #f0f4f8;
      --text-dim: #8892b0;
    }
    * { box-sizing: border-box; margin: 0; padding: 0; }
    body {
      background-color: var(--bg);
      color: var(--text);
      font-family: 'Outfit', sans-serif;
      min-height: 100vh;
      display: flex;
      flex-direction: column;
      align-items: center;
      padding: 24px 16px;
    }
    .container {
      width: 100%;
      max-width: 480px;
    }
    header {
      text-align: center;
      margin-bottom: 24px;
    }
    .badge-bar {
      display: flex;
      justify-content: center;
      gap: 10px;
      margin-top: 8px;
    }
    .badge {
      font-size: 11px;
      padding: 4px 10px;
      border-radius: 20px;
      background: rgba(0, 210, 255, 0.1);
      border: 1px solid var(--accent);
      color: var(--accent);
      font-weight: 600;
    }
    .badge-battery {
      background: rgba(0, 255, 136, 0.1);
      border-color: var(--success);
      color: var(--success);
    }
    .card {
      background: var(--card);
      border: 1px solid var(--border);
      border-radius: 16px;
      padding: 24px;
      margin-bottom: 20px;
      backdrop-filter: blur(12px);
      box-shadow: 0 10px 30px rgba(0, 0, 0, 0.4);
    }
    .card-title {
      font-size: 16px;
      font-weight: 600;
      color: var(--accent);
      margin-bottom: 16px;
      text-transform: uppercase;
      letter-spacing: 0.8px;
    }
    .field {
      margin-bottom: 16px;
    }
    label {
      display: block;
      font-size: 13px;
      color: var(--text-dim);
      margin-bottom: 6px;
    }
    input, select {
      width: 100%;
      background: rgba(0, 0, 0, 0.35);
      border: 1px solid var(--border);
      color: #fff;
      padding: 12px;
      border-radius: 10px;
      font-family: inherit;
      font-size: 14px;
      outline: none;
      transition: all 0.2s;
    }
    input:focus {
      border-color: var(--accent);
      box-shadow: 0 0 12px var(--accent-glow);
    }
    .row {
      display: flex;
      gap: 12px;
    }
    .row > div {
      flex: 1;
    }
    .slider-row {
      display: flex;
      align-items: center;
      gap: 12px;
    }
    .slider-row input[type=range] {
      flex: 1;
      padding: 0;
    }
    .btn {
      width: 100%;
      padding: 14px;
      border-radius: 10px;
      font-family: inherit;
      font-weight: 700;
      font-size: 14px;
      cursor: pointer;
      border: none;
      transition: transform 0.15s, box-shadow 0.15s;
    }
    .btn-primary {
      background: linear-gradient(135deg, #00d2ff, #0072ff);
      color: #000;
      box-shadow: 0 4px 20px var(--accent-glow);
      margin-bottom: 12px;
    }
    .btn-primary:active { transform: scale(0.98); }
    .btn-secondary {
      background: rgba(255, 255, 255, 0.08);
      color: #fff;
      border: 1px solid var(--border);
      margin-bottom: 12px;
    }
    .btn-danger {
      background: rgba(255, 75, 75, 0.15);
      color: #ff6b6b;
      border: 1px solid rgba(255, 75, 75, 0.3);
    }
    .timer-info {
      text-align: center;
      font-size: 12px;
      color: var(--text-dim);
      margin-top: 10px;
    }
  </style>
</head>
<body>
  <div class="container">
    <header>
      <h2>Guiak Node v3.0</h2>
      <div class="badge-bar">
        <span class="badge" id="badge-code">AULA 104</span>
        <span class="badge badge-battery" id="badge-bat">Batería: 95%</span>
      </div>
    </header>

    <div class="card">
      <div class="card-title">Información del Aula</div>
      <div class="field">
        <label>Nombre Descriptivo</label>
        <input type="text" id="roomName" placeholder="Ej: Aula 104 - Informática">
      </div>
      <div class="field">
        <label>Código Corto</label>
        <input type="text" id="roomCode" placeholder="Ej: 104">
      </div>
      <div class="row">
        <div>
          <label>Planta / Piso</label>
          <input type="number" id="floor" min="0" max="10">
        </div>
        <div>
          <label>Edificio</label>
          <input type="text" id="building" placeholder="Ej: Central">
        </div>
      </div>
    </div>

    <div class="card">
      <div class="card-title">Horario Wi-Fi Escolar</div>
      <div class="row">
        <div>
          <label>Hora Inicio (Mañana)</label>
          <input type="time" id="wifiStart" value="07:00">
        </div>
        <div>
          <label>Hora Fin (Tarde)</label>
          <input type="time" id="wifiEnd" value="20:30">
        </div>
      </div>
      <div style="margin-top: 12px; display: flex; align-items: center; gap: 8px;">
        <input type="checkbox" id="wifiSchedEn" style="width: auto;" checked>
        <label for="wifiSchedEn" style="margin: 0; color: #fff;">Activar Wi-Fi automáticamente en horario escolar</label>
      </div>
      <div class="timer-info" style="text-align: left; margin-top: 8px;">
        El Wi-Fi se apagará de noche (20:30 a 07:00) para preservar la batería.
      </div>
    </div>

    <div class="card">
      <div class="card-title">Audio y Baliza de Navegación</div>
      <div class="field">
        <label>Volumen del Altavoz (0 - 100%)</label>
        <div class="slider-row">
          <input type="range" id="volume" min="10" max="100" value="80" oninput="document.getElementById('volVal').innerText = this.value + '%'">
          <span id="volVal" style="font-size: 13px; min-width: 40px;">80%</span>
        </div>
      </div>
      <button class="btn btn-secondary" onclick="testSound()">🔊 Probar Sonido en Puerta</button>
    </div>

    <button class="btn btn-primary" onclick="saveData()">Guardar Cambios</button>
    <button class="btn btn-danger" onclick="exitPortal()">Cerrar Sesión Web</button>
  </div>

  <script>
    fetch('/api/status').then(r => r.json()).then(data => {
      document.getElementById('roomName').value = data.roomName || '';
      document.getElementById('roomCode').value = data.roomCode || '';
      document.getElementById('floor').value = data.floor || 1;
      document.getElementById('building').value = data.building || '';
      document.getElementById('volume').value = data.volume || 80;
      document.getElementById('volVal').innerText = (data.volume || 80) + '%';
      
      let sH = String(data.wStartH !== undefined ? data.wStartH : 7).padStart(2, '0');
      let sM = String(data.wStartM !== undefined ? data.wStartM : 0).padStart(2, '0');
      let eH = String(data.wEndH !== undefined ? data.wEndH : 20).padStart(2, '0');
      let eM = String(data.wEndM !== undefined ? data.wEndM : 30).padStart(2, '0');
      document.getElementById('wifiStart').value = `${sH}:${sM}`;
      document.getElementById('wifiEnd').value = `${eH}:${eM}`;
      document.getElementById('wifiSchedEn').checked = data.wSchedEn !== undefined ? data.wSchedEn : true;

      document.getElementById('badge-code').innerText = data.roomCode || 'SONA';
      document.getElementById('badge-bat').innerText = 'Batería: ' + (data.battery || 100) + '% (' + (data.mv || 4000) + 'mV)';
    });

    function testSound() {
      fetch('/api/test_sound', { method: 'POST' });
    }

    function saveData() {
      const sVal = document.getElementById('wifiStart').value.split(':');
      const eVal = document.getElementById('wifiEnd').value.split(':');

      const payload = {
        roomName: document.getElementById('roomName').value,
        roomCode: document.getElementById('roomCode').value,
        floor: parseInt(document.getElementById('floor').value),
        building: document.getElementById('building').value,
        volume: parseInt(document.getElementById('volume').value),
        wStartH: parseInt(sVal[0]),
        wStartM: parseInt(sVal[1]),
        wEndH: parseInt(eVal[0]),
        wEndM: parseInt(eVal[1]),
        wSchedEn: document.getElementById('wifiSchedEn').checked
      };

      fetch('/api/save', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(payload)
      }).then(r => r.json()).then(res => {
        if(res.ok) alert('¡Configuración guardada correctamente!');
      });
    }

    function exitPortal() {
      alert('Sesión cerrada. El nodo mantendrá el Wi-Fi según su programación horaria.');
    }
  </script>
</body>
</html>
)rawhtml";

bool isWithinWifiSchedule() {
  if (!nodeConfig.wifiScheduleEnabled) return true; // Si el horario está desactivado, Wi-Fi siempre permitido

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo, 100)) {
    // Si aún no hemos sincronizado la hora por NTP tras arrancar, asumimos que estamos dentro de horario
    // para poder conectar y sincronizar.
    return true;
  }

  int curMin   = timeinfo.tm_hour * 60 + timeinfo.tm_min;
  int startMin = nodeConfig.wifiStartHour * 60 + nodeConfig.wifiStartMin;
  int endMin   = nodeConfig.wifiEndHour * 60 + nodeConfig.wifiEndMin;

  return (curMin >= startMin && curMin <= endMin);
}

void handleRoot() {
  server.send_P(200, "text/html", HTML_CONFIG_PAGE);
}

void handleStatus() {
  updateBatteryStatus();
  String json = "{";
  json += "\"roomName\":\"" + String(nodeConfig.roomName) + "\",";
  json += "\"roomCode\":\"" + String(nodeConfig.roomCode) + "\",";
  json += "\"floor\":" + String(nodeConfig.floor) + ",";
  json += "\"building\":\"" + String(nodeConfig.building) + "\",";
  json += "\"beaconUUID\":\"" + String(nodeConfig.beaconUUID) + "\",";
  json += "\"volume\":" + String(nodeConfig.volume) + ",";
  json += "\"battery\":" + String(batteryPercent) + ",";
  json += "\"mv\":" + String(batteryMillivolts) + ",";
  json += "\"wStartH\":" + String(nodeConfig.wifiStartHour) + ",";
  json += "\"wStartM\":" + String(nodeConfig.wifiStartMin) + ",";
  json += "\"wEndH\":" + String(nodeConfig.wifiEndHour) + ",";
  json += "\"wEndM\":" + String(nodeConfig.wifiEndMin) + ",";
  json += "\"wSchedEn\":" + String(nodeConfig.wifiScheduleEnabled ? "true" : "false");
  json += "}";
  server.send(200, "application/json", json);
}

void handleTestSound() {
  playAcousticBeacon();
  server.send(200, "application/json", "{\"ok\":true}");
}

void handleSave() {
  if (server.hasArg("plain")) {
    String body = server.arg("plain");

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
      while (end < src.length() && (isdigit(src[end]))) end++;
      return src.substring(start, end).toInt();
    };

    String newName = extractStr(body, "roomName");
    String newCode = extractStr(body, "roomCode");
    String newBld  = extractStr(body, "building");
    int newFloor   = extractInt(body, "floor");
    int newVol     = extractInt(body, "volume");
    int sH         = extractInt(body, "wStartH");
    int sM         = extractInt(body, "wStartM");
    int eH         = extractInt(body, "wEndH");
    int eM         = extractInt(body, "wEndM");
    bool schedEn   = (body.indexOf("\"wSchedEn\":true") != -1);

    if (newName.length() > 0) newName.toCharArray(nodeConfig.roomName, sizeof(nodeConfig.roomName));
    if (newCode.length() > 0) newCode.toCharArray(nodeConfig.roomCode, sizeof(nodeConfig.roomCode));
    if (newBld.length() > 0)  newBld.toCharArray(nodeConfig.building, sizeof(nodeConfig.building));
    if (newFloor >= 0) nodeConfig.floor = newFloor;
    if (newVol >= 0 && newVol <= 100) nodeConfig.volume = newVol;
    if (sH >= 0 && sH <= 23) nodeConfig.wifiStartHour = sH;
    if (sM >= 0 && sM <= 59) nodeConfig.wifiStartMin = sM;
    if (eH >= 0 && eH <= 23) nodeConfig.wifiEndHour = eH;
    if (eM >= 0 && eM <= 59) nodeConfig.wifiEndMin = eM;
    nodeConfig.wifiScheduleEnabled = schedEn;

    dbSaveConfig();
    server.send(200, "application/json", "{\"ok\":true}");
    return;
  }
  server.send(400, "application/json", "{\"error\":\"Invalid payload\"}");
}

void startWifiService(bool isManual) {
  if (isWifiActive && serverRunning) return;

  addLog("WIFI", isManual ? "Iniciando Wi-Fi (Manual)" : "Iniciando Wi-Fi (Horario 07:00 - 20:30)...");

  // Modo AP + STA para permitir conexión a red del instituto y punto de acceso de respaldo
  String apSSID = "Guiak-Node-" + String(nodeConfig.roomCode);
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(apSSID.c_str(), "guiak1234");

  if (strlen(nodeConfig.wifiSSID) > 0) {
    WiFi.begin(nodeConfig.wifiSSID, nodeConfig.wifiPassword);
  }

  // Activar Wi-Fi Modem Sleep para reducir el consumo hasta 15-20 mA mientras está conectado
  esp_wifi_set_ps(WIFI_PS_MIN_MODEM);

  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/status", HTTP_GET, handleStatus);
  server.on("/api/test_sound", HTTP_POST, handleTestSound);
  server.on("/api/save", HTTP_POST, handleSave);

  server.begin();
  serverRunning = true;
  isWifiActive = true;
  isConfigMode = true;

  if (isManual) {
    isManualWifiOverride = true;
    manualOverrideStart = millis();
  }

  // Sincronizar reloj por NTP si hay conexión a internet
  ntpSync();
}

void stopWifiService() {
  if (!isWifiActive) return;

  addLog("WIFI", "Apagando Wi-Fi por horario nocturno (Ahorro de bateria)");

  if (serverRunning) {
    server.stop();
    serverRunning = false;
  }

  WiFi.softAPdisconnect(true);
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);

  isWifiActive = false;
  isConfigMode = false;
  isManualWifiOverride = false;
}

void wifiServiceInit() {
  // Comprobar si debemos arrancar el Wi-Fi según el horario escolar
  if (isWithinWifiSchedule()) {
    startWifiService(false);
  } else {
    stopWifiService();
  }
}

void handleWifiService() {
  if (isWifiActive && serverRunning) {
    server.handleClient();
  }

  // Revisión periódica de la ventana horaria (07:00 a 20:30)
  if (millis() - lastScheduleCheck > SCHEDULE_CHECK_INTERVAL) {
    lastScheduleCheck = millis();

    if (isManualWifiOverride) {
      // Si se encendió manualmente por botón, comprobar temporizador (ej: 5 min)
      unsigned long timeoutMs = (unsigned long)nodeConfig.configTimeoutSec * 1000UL;
      if (millis() - manualOverrideStart > timeoutMs) {
        isManualWifiOverride = false;
        // Volver a evaluar el horario
        if (!isWithinWifiSchedule()) {
          stopWifiService();
        }
      }
    } else if (nodeConfig.wifiScheduleEnabled) {
      bool shouldBeOn = isWithinWifiSchedule();
      if (shouldBeOn && !isWifiActive) {
        startWifiService(false);
      } else if (!shouldBeOn && isWifiActive) {
        stopWifiService();
      }
    }
  }
}
