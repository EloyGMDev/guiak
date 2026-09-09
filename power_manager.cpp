#include "power_manager.h"
#include "config.h"

#if defined(ESP32)
#include "esp_sleep.h"
#include "esp_adc_cal.h"
#endif

bool isBatteryConnected = false;

void powerInit() {
  // Configuración del ADC para lectura de batería
  analogReadResolution(12); // 0 a 4095
#if defined(ESP32)
  analogSetAttenuation(ADC_11db); // Rango de hasta ~3.1V en el pin
#endif
  pinMode(BATTERY_ADC_PIN, INPUT);
  
  // Primera lectura de arranque
  updateBatteryStatus();
}

uint16_t readBatteryMillivolts() {
  // Tomar varias muestras para estabilizar la lectura
  const int SAMPLES = 10;
  uint32_t rawSum = 0;
  for (int i = 0; i < SAMPLES; i++) {
    rawSum += analogRead(BATTERY_ADC_PIN);
    delayMicroseconds(100);
  }
  float avgRaw = (float)rawSum / SAMPLES;

  // Si el valor ADC es casi nulo (< 80 sobre 4095), no hay divisor/batería conectada
  // (el Arduino está alimentado directamente por cable USB de 5V)
  if (avgRaw < 80) {
    return 0;
  }

#if defined(ARDUINO_UNOR4_WIFI)
  float pinMillivolts = (avgRaw / 4095.0f) * 5000.0f;
#else
  float pinMillivolts = (avgRaw / 4095.0f) * 3100.0f;
#endif

  // Aplicar factor del divisor resistivo
  float batMv = pinMillivolts * ADC_DIVIDER_RATIO;
  
  if (batMv < 1000) return 0;
  if (batMv > 4350) batMv = 4350;

  return (uint16_t)batMv;
}

void updateBatteryStatus() {
  batteryMillivolts = readBatteryMillivolts();

  if (batteryMillivolts < 2000) {
    // Modo alimentación externa continua (USB / Fuente 5V) sin batería Li-Ion
    isBatteryConnected = false;
    batteryPercent = 100;
  } else {
    // Batería Li-Ion presente: calcular porcentaje real
    isBatteryConnected = true;
    if (batteryMillivolts <= BATTERY_MIN_MV) {
      batteryPercent = 0;
    } else if (batteryMillivolts >= BATTERY_MAX_MV) {
      batteryPercent = 100;
    } else {
      batteryPercent = (uint8_t)(((batteryMillivolts - BATTERY_MIN_MV) * 100UL) / (BATTERY_MAX_MV - BATTERY_MIN_MV));
    }
  }
}

bool isBatteryLow() {
  // En modo USB continuo nunca hay aviso de batería baja
  if (!isBatteryConnected) return false;
  return (batteryPercent <= BATTERY_LOW_THRESH);
}

void enterLightSleep(uint32_t durationMs) {
  if (durationMs == 0) return;
  
#if defined(ESP32)
  // Configurar timer de despertar
  esp_sleep_enable_timer_wakeup((uint64_t)durationMs * 1000ULL);
  
  // Configurar despertar por botón si se pulsa durante el sueño
  esp_sleep_enable_ext0_wakeup((gpio_num_t)BTN_CONFIG_PIN, 0); // Despertar si se pulsa (LOW)

  // Entrar en Light Sleep (mantiene memoria RAM y variables)
  esp_light_sleep_start();
#else
  delay(durationMs);
#endif
}
