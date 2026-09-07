#include "config.h"

// Instancia de configuración del nodo
NodeConfig nodeConfig;

// Variables de estado del sistema
bool isWifiActive = false;
bool isConfigMode = false;
bool isManualWifiOverride = false;
unsigned long manualOverrideStart = 0;
uint8_t batteryPercent = 100;
uint16_t batteryMillivolts = 4100;
bool isSoundPlaying = false;
