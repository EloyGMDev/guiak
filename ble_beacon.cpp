#include "ble_beacon.h"
#include "config.h"
#include "audio_manager.h"
#include "firebase_client.h"
#include "component_detector.h"

#define GUIAK_SERVICE_UUID        "0000FD00-0000-1000-8000-00805F9B34FB"
#define CHAR_TRIGGER_UUID        "0000FD01-0000-1000-8000-00805F9B34FB"
#define CHAR_INFO_UUID           "0000FD02-0000-1000-8000-00805F9B34FB"
#define CHAR_PRESENCE_UUID       "0000FD03-0000-1000-8000-00805F9B34FB"
#define BATTERY_SERVICE_UUID     "0000180F-0000-1000-8000-00805F9B34FB"
#define CHAR_BATTERY_LEVEL_UUID  "00002A19-0000-1000-8000-00805F9B34FB"

#if defined(ESP32)
// ════════════════════════════════════════════════════════════════
//  IMPLEMENTACIÓN NATIVA BLE PARA ESP32 / ESP32-S3
// ════════════════════════════════════════════════════════════════
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <BLEBeacon.h>

static BLEServer* pServer = nullptr;
static BLECharacteristic* pTriggerChar = nullptr;
static BLECharacteristic* pInfoChar = nullptr;
static BLECharacteristic* pPresenceChar = nullptr;
static BLECharacteristic* pBatteryChar = nullptr;
static BLEAdvertising* pAdvertising = nullptr;
static bool deviceConnected = false;

class GuiakServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  }
  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    if (pAdvertising) {
      pAdvertising->start();
    }
  }
};

class PresenceCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pCharacteristic) {
    std::string val = pCharacteristic->getValue();
    if (val.length() > 0) {
      String studentId = String(val.c_str());
      studentId.trim();
      if (studentId.length() > 0) {
        firebaseRecordStudentPresence(studentId, "BLE", -60);
      }
    }
  }
};

class TriggerCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pCharacteristic) {
    std::string val = pCharacteristic->getValue();
    if (val.length() > 0) {
      uint8_t cmd = (uint8_t)val[0];
      if (cmd == 1 || cmd == '1') {
        playAcousticBeacon();
        firebasePushLog("AUDIO", "Baliza acustica activada por la app movil");
        if (val.length() > 2 && val[1] == ':') {
          String stdId = String(val.substr(2).c_str());
          firebaseRecordStudentPresence(stdId, "BLE", -55);
        }
      } else if (cmd == 2 || cmd == '2') {
        playArrivalChime();
      } else if (cmd == 3 || cmd == '3') {
        playOminousAlarm(5000);
      }
    }
  }
};

void bleBeaconInit() {
  BLEDevice::init(nodeConfig.roomCode);
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_P6);

  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new GuiakServerCallbacks());

  BLEService* pSonaService = pServer->createService(GUIAK_SERVICE_UUID);
  pTriggerChar = pSonaService->createCharacteristic(
    CHAR_TRIGGER_UUID,
    BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR
  );
  pTriggerChar->setCallbacks(new TriggerCallbacks());

  pPresenceChar = pSonaService->createCharacteristic(
    CHAR_PRESENCE_UUID,
    BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR
  );
  pPresenceChar->setCallbacks(new PresenceCallbacks());

  pInfoChar = pSonaService->createCharacteristic(
    CHAR_INFO_UUID,
    BLECharacteristic::PROPERTY_READ
  );
  String infoPayload = String(nodeConfig.roomName) + "|" + String(nodeConfig.roomCode) + "|" + String(nodeConfig.floor);
  pInfoChar->setValue(infoPayload.c_str());

  pSonaService->start();

  BLEService* pBatteryService = pServer->createService(BATTERY_SERVICE_UUID);
  pBatteryChar = pBatteryService->createCharacteristic(
    CHAR_BATTERY_LEVEL_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
  );
  pBatteryChar->addDescriptor(new BLE2902());
  uint8_t bLevel = batteryPercent;
  pBatteryChar->setValue(&bLevel, 1);
  pBatteryService->start();

  BLEBeacon myBeacon;
  myBeacon.setManufacturerId(0x4C00);
  BLEUUID beaconUUID(nodeConfig.beaconUUID);
  myBeacon.setProximityUUID(beaconUUID);
  myBeacon.setMajor(nodeConfig.major);
  myBeacon.setMinor(nodeConfig.minor);
  myBeacon.setSignalPower(nodeConfig.measuredPower);

  BLEAdvertisementData oAdvertisementData;
  BLEAdvertisementData oScanResponseData;
  oAdvertisementData.setFlags(0x06);

  std::string beaconData = myBeacon.getData();
  std::string mfgData = "";
  mfgData += (char)0x02;
  mfgData += (char)0x15;
  mfgData += beaconData;
  oAdvertisementData.setManufacturerData(mfgData);

  oScanResponseData.setName(nodeConfig.roomName);
  oScanResponseData.setCompleteServices(BLEUUID(GUIAK_SERVICE_UUID));

  pAdvertising = pServer->getAdvertising();
  pAdvertising->setAdvertisementData(oAdvertisementData);
  pAdvertising->setScanResponseData(oScanResponseData);

  uint16_t advInterval = (nodeConfig.advIntervalMs > 0) ? nodeConfig.advIntervalMs : DEFAULT_ADV_INTERVAL_MS;
  uint16_t intervalUnits = (advInterval * 8) / 5;
  pAdvertising->setMinInterval(intervalUnits);
  pAdvertising->setMaxInterval(intervalUnits + 16);

  pAdvertising->start();
}

void bleUpdateBatteryLevel(uint8_t percent) {
  if (pBatteryChar) {
    pBatteryChar->setValue(&percent, 1);
    if (deviceConnected) {
      pBatteryChar->notify();
    }
  }
}

void bleStop() {
  if (pAdvertising) {
    pAdvertising->stop();
  }
  BLEDevice::deinit(false);
}

void bleStart() {
  bleBeaconInit();
}

void bleLoop() {
}

#elif defined(ARDUINO_UNOR4_WIFI) || __has_include(<ArduinoBLE.h>)
// ════════════════════════════════════════════════════════════════
//  IMPLEMENTACIÓN BLE PARA ARDUINO UNO R4 WIFI (ArduinoBLE)
// ════════════════════════════════════════════════════════════════
#include <ArduinoBLE.h>

static BLEService r4SonaService(GUIAK_SERVICE_UUID);
static BLECharacteristic r4TriggerChar(CHAR_TRIGGER_UUID, BLEWrite | BLEWriteWithoutResponse, 32);
static BLECharacteristic r4InfoChar(CHAR_INFO_UUID, BLERead, 64);
static BLECharacteristic r4PresenceChar(CHAR_PRESENCE_UUID, BLEWrite | BLEWriteWithoutResponse, 32);
static BLEService r4BatteryService(BATTERY_SERVICE_UUID);
static BLEUnsignedCharCharacteristic r4BatteryChar(CHAR_BATTERY_LEVEL_UUID, BLERead | BLENotify);

void bleBeaconInit() {
  if (!BLE.begin()) {
    return;
  }
  BLE.setLocalName(nodeConfig.roomCode);
  BLE.setAdvertisedService(r4SonaService);

  r4SonaService.addCharacteristic(r4TriggerChar);
  r4SonaService.addCharacteristic(r4InfoChar);
  r4SonaService.addCharacteristic(r4PresenceChar);
  BLE.addService(r4SonaService);

  r4BatteryService.addCharacteristic(r4BatteryChar);
  BLE.addService(r4BatteryService);

  r4BatteryChar.writeValue(batteryPercent);

  String infoPayload = String(nodeConfig.roomName) + "|" + String(nodeConfig.roomCode) + "|" + String(nodeConfig.floor);
  r4InfoChar.writeValue(infoPayload.c_str());

  BLE.advertise();
}

void bleUpdateBatteryLevel(uint8_t percent) {
  r4BatteryChar.writeValue(percent);
}

void bleStop() {
  BLE.stopAdvertise();
  BLE.end();
}

void bleStart() {
  bleBeaconInit();
}

void bleLoop() {
  BLEDevice central = BLE.central();
  if (central && central.connected()) {
    if (r4TriggerChar.written()) {
      int len = r4TriggerChar.valueLength();
      const uint8_t* val = r4TriggerChar.value();
      if (len > 0) {
        if (val[0] == 1 || val[0] == '1') {
          playAcousticBeacon();
          firebasePushLog("AUDIO", "Baliza acustica activada por BLE");
          if (len > 2 && val[1] == ':') {
            String stdId = "";
            for (int i = 2; i < len; i++) stdId += (char)val[i];
            firebaseRecordStudentPresence(stdId, "BLE", -55);
          }
        } else if (val[0] == 2 || val[0] == '2') {
          playArrivalChime();
        } else if (val[0] == 3 || val[0] == '3') {
          playOminousAlarm(5000);
        }
      }
    }
    if (r4PresenceChar.written()) {
      int len = r4PresenceChar.valueLength();
      const uint8_t* val = r4PresenceChar.value();
      if (len > 0) {
        String stdId = "";
        for (int i = 0; i < len; i++) stdId += (char)val[i];
        stdId.trim();
        if (stdId.length() > 0) {
          firebaseRecordStudentPresence(stdId, "BLE", -60);
        }
      }
    }
  }
}

#else
// Fallback para placas sin BLE
void bleBeaconInit() {}
void bleUpdateBatteryLevel(uint8_t percent) {}
void bleStop() {}
void bleStart() {}
void bleLoop() {}
#endif