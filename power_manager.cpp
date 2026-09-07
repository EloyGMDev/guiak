#include "power_manager.h"
#include "config.h"
#include "esp_sleep.h"
#include "esp_adc_cal.h"

void powerInit() {
  // Configuración del ADC para lectura de batería en ESP32-S3
  analogReadResolution(12); // 0 a 4095
  analogSetAttenuation(ADC_11db); // Rango de hasta ~3.1V en el pin
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

  // En ESP32 con atenuación de 11dB, 4095 equivale a ~3100 mV en el pin físico
  float pinMillivolts = (avgRaw / 4095.0f) * 3100.0f;

  // Aplicar factor del divisor resistivo (ej. R1=100k, R2=100k -> x2.0)
  float batMv = pinMillivolts * ADC_DIVIDER_RATIO;
  
  // Limitar a rangos razonables de celda Li-ion (3.0V a 4.3V)
  if (batMv < 2500) batMv = 2500;
  if (batMv > 4350) batMv = 4350;

  return (uint16_t)batMv;
}

void updateBatteryStatus() {
  batteryMillivolts = readBatteryMillivolts();

  // Calcular porcentaje con curva de descarga estimada para Li-Ion (3300mV a 4200mV)
  if (batteryMillivolts <= BATTERY_MIN_MV) {
    batteryPercent = 0;
  } else if (batteryMillivolts >= BATTERY_MAX_MV) {
    batteryPercent = 100;
  } else {
    // Estimación lineal escalada
    batteryPercent = (uint8_t)(((batteryMillivolts - BATTERY_MIN_MV) * 100UL) / (BATTERY_MAX_MV - BATTERY_MIN_MV));
  }
}

bool isBatteryLow() {
  return (batteryPercent <= BATTERY_LOW_THRESH);
}

void enterLightSleep(uint32_t durationMs) {
  if (durationMs == 0) return;
  
  // Configurar timer de despertar
  esp_sleep_enable_timer_wakeup((uint64_t)durationMs * 1000ULL);
  
  // Configurar despertar por botón si se pulsa durante el sueño
  esp_sleep_enable_ext0_wakeup((gpio_num_t)BTN_CONFIG_PIN, 0); // Despertar si se pulsa (LOW)

  // Entrar en Light Sleep (mantiene memoria RAM y variables)
  esp_light_sleep_start();
}
