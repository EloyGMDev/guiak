#include "ble_beacon.h"
#include "config.h"
#include "audio_manager.h"

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <BLEBeacon.h>

// UUIDs de Servicio y Características SONA
#define GUIAK_SERVICE_UUID        "0000FD00-0000-1000-8000-00805F9B34FB"
#define CHAR_TRIGGER_UUID        "0000FD01-0000-1000-8000-00805F9B34FB"
#define CHAR_INFO_UUID           "0000FD02-0000-1000-8000-00805F9B34FB"

// Servicio Estándar de Batería Bluetooth SIG
#define BATTERY_SERVICE_UUID     "0000180F-0000-1000-8000-00805F9B34FB"
#define CHAR_BATTERY_LEVEL_UUID  "00002A19-0000-1000-8000-00805F9B34FB"

static BLEServer* pServer = nullptr;
static BLECharacteristic* pTriggerChar = nullptr;
static BLECharacteristic* pInfoChar = nullptr;
static BLECharacteristic* pBatteryChar = nullptr;
static BLEAdvertising* pAdvertising = nullptr;
static bool deviceConnected = false;

// Callbacks del Servidor BLE
class GuiakServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  }
  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    // Reiniciar publicidad al desconectar para que otros alumnos puedan interactuar
    if (pAdvertising) {
      pAdvertising->start();
    }
  }
};

// Callback para activación de sonido bajo demanda desde la app móvil
class TriggerCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pCharacteristic) {
    std::string val = pCharacteristic->getValue();
    if (val.length() > 0) {
      uint8_t cmd = (uint8_t)val[0];
      if (cmd == 1 || cmd == '1') {
        // Disparo de baliza acústica estándar
        playAcousticBeacon();
      } else if (cmd == 2 || cmd == '2') {
        // Disparo de confirmación de llegada
        playArrivalChime();
      }
    }
  }
};

void bleBeaconInit() {
  // Inicializar dispositivo BLE con el nombre del aula
  BLEDevice::init(nodeConfig.roomCode);

  // Configurar potencia de transmisión Bluetooth óptima (equilibrio alcance/batería)
  // En ESP32-S3: ESP_PWR_LVL_P3 (+3 dBm) o ESP_PWR_LVL_P6 (+6 dBm)
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_P6);

  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new GuiakServerCallbacks());

  // 1. Crear Servicio SONA
  BLEService* pSonaService = pServer->createService(GUIAK_SERVICE_UUID);

  // Característica "Trigger Sound" (Escritura desde el móvil)
  pTriggerChar = pSonaService->createCharacteristic(
    CHAR_TRIGGER_UUID,
    BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR
  );
  pTriggerChar->setCallbacks(new TriggerCallbacks());

  // Característica "Room Info" (Lectura de metadatos)
  pInfoChar = pSonaService->createCharacteristic(
    CHAR_INFO_UUID,
    BLECharacteristic::PROPERTY_READ
  );
  String infoPayload = String(nodeConfig.roomName) + "|" + String(nodeConfig.roomCode) + "|" + String(nodeConfig.floor);
  pInfoChar->setValue(infoPayload.c_str());

  pSonaService->start();

  // 2. Crear Servicio de Batería Estándar
  BLEService* pBatteryService = pServer->createService(BATTERY_SERVICE_UUID);
  pBatteryChar = pBatteryService->createCharacteristic(
    CHAR_BATTERY_LEVEL_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
  );
  pBatteryChar->addDescriptor(new BLE2902());
  uint8_t bLevel = batteryPercent;
  pBatteryChar->setValue(&bLevel, 1);
  pBatteryService->start();

  // 3. Configurar Trama iBeacon
  BLEBeacon myBeacon;
  myBeacon.setManufacturerId(0x4C00); // Identificador Apple iBeacon

  BLEUUID beaconUUID(nodeConfig.beaconUUID);
  myBeacon.setProximityUUID(beaconUUID);
  myBeacon.setMajor(nodeConfig.major);
  myBeacon.setMinor(nodeConfig.minor);
  myBeacon.setSignalPower(nodeConfig.measuredPower);

  BLEAdvertisementData oAdvertisementData;
  BLEAdvertisementData oScanResponseData;

  oAdvertisementData.setFlags(0x06); // General Discoverable + BR/EDR Not Supported

  std::string beaconData = myBeacon.getData();
  std::string mfgData = "";
  mfgData += (char)0x02; // Longitud del subtipo
  mfgData += (char)0x15; // Subtipo iBeacon
  mfgData += beaconData;
  oAdvertisementData.setManufacturerData(mfgData);

  // En la respuesta de escaneo (Scan Response), publicar el nombre del aula
  oScanResponseData.setName(nodeConfig.roomName);
  oScanResponseData.setCompleteServices(BLEUUID(GUIAK_SERVICE_UUID));

  pAdvertising = pServer->getAdvertising();
  pAdvertising->setAdvertisementData(oAdvertisementData);
  pAdvertising->setScanResponseData(oScanResponseData);

  // Intervalo de anuncio (500 ms por defecto para ahorrar batería)
  uint16_t advInterval = (nodeConfig.advIntervalMs > 0) ? nodeConfig.advIntervalMs : DEFAULT_ADV_INTERVAL_MS;
  uint16_t intervalUnits = (advInterval * 8) / 5; // 0.625 ms units
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
  // El stack BLE del ESP32 funciona de forma asíncrona mediante eventos FreeRTOS
}
