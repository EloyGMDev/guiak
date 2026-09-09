#include "database.h"
#include <Preferences.h>

static Preferences prefs;
#define NVS_NAMESPACE "guiak_node"

void dbInit() {
  prefs.begin(NVS_NAMESPACE, false);
  
  // Si no está inicializado, guardar valores por defecto
  if (!prefs.isKey("initialized")) {
    dbResetDefaults();
  } else {
    dbLoadConfig();
  }
}

void dbResetDefaults() {
  prefs.clear();

  strncpy(nodeConfig.roomName, "Aula 104 - Informatica", sizeof(nodeConfig.roomName) - 1);
  strncpy(nodeConfig.roomCode, "104", sizeof(nodeConfig.roomCode) - 1);
  nodeConfig.floor = 1;
  strncpy(nodeConfig.building, "Edificio Principal", sizeof(nodeConfig.building) - 1);
  strncpy(nodeConfig.beaconUUID, DEFAULT_BEACON_UUID, sizeof(nodeConfig.beaconUUID) - 1);
  nodeConfig.major = 1;
  nodeConfig.minor = 104;
  nodeConfig.measuredPower = BEACON_MEASURED_POWER;
  nodeConfig.volume = 80;
  nodeConfig.advIntervalMs = DEFAULT_ADV_INTERVAL_MS;
  strncpy(nodeConfig.wifiSSID, "Guiak-WiFi", sizeof(nodeConfig.wifiSSID) - 1);
  strncpy(nodeConfig.wifiPassword, "", sizeof(nodeConfig.wifiPassword) - 1);
  strncpy(nodeConfig.adminPassword, "admin", sizeof(nodeConfig.adminPassword) - 1);
  nodeConfig.configTimeoutSec = 300; // 5 minutos
  nodeConfig.wifiStartHour = 7;      // 07:00
  nodeConfig.wifiStartMin  = 0;
  nodeConfig.wifiEndHour   = 20;     // 20:30
  nodeConfig.wifiEndMin    = 30;
  nodeConfig.wifiScheduleEnabled = true;
  strncpy(nodeConfig.firebaseHost, "https://guiak-default-rtdb.firebaseio.com", sizeof(nodeConfig.firebaseHost) - 1);
  strncpy(nodeConfig.firebaseAuth, "", sizeof(nodeConfig.firebaseAuth) - 1);
  nodeConfig.firebaseEnabled = true;
  nodeConfig.hasBuzzer = true;
  nodeConfig.hasSpeaker = true;

  dbSaveConfig();
  prefs.putBool("initialized", true);
}

void dbLoadConfig() {
  prefs.getString("roomName", nodeConfig.roomName, sizeof(nodeConfig.roomName));
  prefs.getString("roomCode", nodeConfig.roomCode, sizeof(nodeConfig.roomCode));
  nodeConfig.floor = prefs.getUShort("floor", 1);
  prefs.getString("building", nodeConfig.building, sizeof(nodeConfig.building));
  prefs.getString("beaconUUID", nodeConfig.beaconUUID, sizeof(nodeConfig.beaconUUID));
  nodeConfig.major = prefs.getUShort("major", 1);
  nodeConfig.minor = prefs.getUShort("minor", 104);
  nodeConfig.measuredPower = (int8_t)prefs.getChar("measPower", BEACON_MEASURED_POWER);
  nodeConfig.volume = prefs.getUChar("volume", 80);
  nodeConfig.advIntervalMs = prefs.getUShort("advInt", DEFAULT_ADV_INTERVAL_MS);
  prefs.getString("wifiSSID", nodeConfig.wifiSSID, sizeof(nodeConfig.wifiSSID));
  prefs.getString("wifiPass", nodeConfig.wifiPassword, sizeof(nodeConfig.wifiPassword));
  prefs.getString("adminPass", nodeConfig.adminPassword, sizeof(nodeConfig.adminPassword));
  nodeConfig.configTimeoutSec = prefs.getUShort("cfgTimeout", 300);
  nodeConfig.wifiStartHour = prefs.getUChar("wStartH", 7);
  nodeConfig.wifiStartMin  = prefs.getUChar("wStartM", 0);
  nodeConfig.wifiEndHour   = prefs.getUChar("wEndH", 20);
  nodeConfig.wifiEndMin    = prefs.getUChar("wEndM", 30);
  nodeConfig.wifiScheduleEnabled = prefs.getBool("wSchedEn", true);
  prefs.getString("fbHost", nodeConfig.firebaseHost, sizeof(nodeConfig.firebaseHost));
  prefs.getString("fbAuth", nodeConfig.firebaseAuth, sizeof(nodeConfig.firebaseAuth));
  nodeConfig.firebaseEnabled = prefs.getBool("fbEn", true);
  nodeConfig.hasBuzzer = prefs.getBool("hasBuzzer", true);
  nodeConfig.hasSpeaker = prefs.getBool("hasSpeaker", true);
}

void dbSaveConfig() {
  prefs.putString("roomName", nodeConfig.roomName);
  prefs.putString("roomCode", nodeConfig.roomCode);
  prefs.putUShort("floor", nodeConfig.floor);
  prefs.putString("building", nodeConfig.building);
  prefs.putString("beaconUUID", nodeConfig.beaconUUID);
  prefs.putUShort("major", nodeConfig.major);
  prefs.putUShort("minor", nodeConfig.minor);
  prefs.putChar("measPower", (int8_t)nodeConfig.measuredPower);
  prefs.putUChar("volume", nodeConfig.volume);
  prefs.putUShort("advInt", nodeConfig.advIntervalMs);
  prefs.putString("wifiSSID", nodeConfig.wifiSSID);
  prefs.putString("wifiPass", nodeConfig.wifiPassword);
  prefs.putString("adminPass", nodeConfig.adminPassword);
  prefs.putUShort("cfgTimeout", nodeConfig.configTimeoutSec);
  prefs.putUChar("wStartH", nodeConfig.wifiStartHour);
  prefs.putUChar("wStartM", nodeConfig.wifiStartMin);
  prefs.putUChar("wEndH", nodeConfig.wifiEndHour);
  prefs.putUChar("wEndM", nodeConfig.wifiEndMin);
  prefs.putBool("wSchedEn", nodeConfig.wifiScheduleEnabled);
  prefs.putString("fbHost", nodeConfig.firebaseHost);
  prefs.putString("fbAuth", nodeConfig.firebaseAuth);
  prefs.putBool("fbEn", nodeConfig.firebaseEnabled);
  prefs.putBool("hasBuzzer", nodeConfig.hasBuzzer);
  prefs.putBool("hasSpeaker", nodeConfig.hasSpeaker);
}

void dbIncrementAccessCount() {
  uint32_t count = prefs.getUInt("accessCount", 0);
  count++;
  prefs.putUInt("accessCount", count);
}

uint32_t dbGetAccessCount() {
  return prefs.getUInt("accessCount", 0);
}

bool dbIsCardAuthorized(const String& uid) {
  String key = "card_" + uid;
  return prefs.isKey(key.c_str());
}

void dbAuthorizeCard(const String& uid, const String& ownerName) {
  String key = "card_" + uid;
  prefs.putString(key.c_str(), ownerName);
}
