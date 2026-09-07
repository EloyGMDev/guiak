#include "audio_manager.h"
#include "config.h"

#if defined(ARDUINO_UNOR4_WIFI)
// ════════════════════════════════════════════════════════════════
//  AUDIO PARA ARDUINO UNO R4 WIFI (Buzzer / DAC A0)
// ════════════════════════════════════════════════════════════════
void audioInit() {
  pinMode(BUZZER_PIN, OUTPUT);
}

void audioPowerDown() {
  noTone(BUZZER_PIN);
  isSoundPlaying = false;
}

void playToneI2S(uint16_t freqHz, uint16_t durationMs, uint8_t volumePercent) {
  isSoundPlaying = true;
  tone(BUZZER_PIN, freqHz, durationMs);
  delay(durationMs);
  audioPowerDown();
}

#else
// ════════════════════════════════════════════════════════════════
//  AUDIO NATIVO ESP32-S3 (I2S Digital MAX98357A con corte a 0 µA)
// ════════════════════════════════════════════════════════════════
#include "driver/i2s.h"
#include <math.h>

#define I2S_PORT          I2S_NUM_0
#define I2S_SAMPLE_RATE   22050

static bool i2sInstalled = false;

void audioInit() {
  pinMode(I2S_SD_PIN, OUTPUT);
  digitalWrite(I2S_SD_PIN, LOW); // 0 µA en reposo

  if (!i2sInstalled) {
    i2s_config_t i2s_config = {
      .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
      .sample_rate = I2S_SAMPLE_RATE,
      .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
      .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
      .communication_format = I2S_COMM_FORMAT_STAND_I2S,
      .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
      .dma_buf_count = 4,
      .dma_buf_len = 256,
      .use_apll = false,
      .tx_desc_auto_clear = true,
      .fixed_mclk = 0
    };

    i2s_pin_config_t pin_config = {
      .bck_io_num = I2S_BCLK_PIN,
      .ws_io_num = I2S_LRC_PIN,
      .data_out_num = I2S_DIN_PIN,
      .data_in_num = I2S_PIN_NO_CHANGE
    };

    if (i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL) == ESP_OK) {
      i2s_set_pin(I2S_PORT, &pin_config);
      i2sInstalled = true;
    }
  }

  audioPowerDown();
}

void audioPowerDown() {
  digitalWrite(I2S_SD_PIN, LOW);
  isSoundPlaying = false;
}

void playToneI2S(uint16_t freqHz, uint16_t durationMs, uint8_t volumePercent) {
  if (!i2sInstalled || freqHz == 0 || durationMs == 0) return;

  isSoundPlaying = true;
  digitalWrite(I2S_SD_PIN, HIGH);
  delay(10);

  volumePercent = constrain(volumePercent, 0, 100);
  int16_t maxAmplitude = (int16_t)((32767.0f * (volumePercent / 100.0f)) * 0.7f);

  size_t totalSamples = (I2S_SAMPLE_RATE * durationMs) / 1000;
  size_t attackSamples = totalSamples / 10;
  if (attackSamples > 500) attackSamples = 500;
  size_t decaySamples = attackSamples;

  const size_t CHUNK_SIZE = 128;
  int16_t buffer[CHUNK_SIZE * 2];

  float phase = 0.0f;
  float phaseStep = (2.0f * (float)M_PI * freqHz) / (float)I2S_SAMPLE_RATE;

  size_t samplesGenerated = 0;
  while (samplesGenerated < totalSamples) {
    size_t chunk = CHUNK_SIZE;
    if (samplesGenerated + chunk > totalSamples) {
      chunk = totalSamples - samplesGenerated;
    }

    for (size_t i = 0; i < chunk; i++) {
      size_t currentSample = samplesGenerated + i;
      float env = 1.0f;
      if (currentSample < attackSamples) {
        env = (float)currentSample / (float)attackSamples;
      } else if (currentSample > totalSamples - decaySamples) {
        env = (float)(totalSamples - currentSample) / (float)decaySamples;
      }

      int16_t sampleVal = (int16_t)(sinf(phase) * maxAmplitude * env);
      phase += phaseStep;
      if (phase >= 2.0f * (float)M_PI) phase -= 2.0f * (float)M_PI;

      buffer[i * 2]     = sampleVal;
      buffer[i * 2 + 1] = sampleVal;
    }

    size_t bytesWritten = 0;
    i2s_write(I2S_PORT, buffer, chunk * 2 * sizeof(int16_t), &bytesWritten, portMAX_DELAY);
    samplesGenerated += chunk;
  }

  delay(25);
  audioPowerDown();
}
#endif

// ════════════════════════════════════════════════════════════════
//  PATRONES ACÚSTICOS COMUNES DE ORIENTACIÓN ESPACIAL
// ════════════════════════════════════════════════════════════════
void playAcousticBeacon() {
  uint8_t vol = (nodeConfig.volume > 0) ? nodeConfig.volume : 80;
  playToneI2S(880,  120, vol);  // A5
  delay(50);
  playToneI2S(1320, 220, vol);  // E6 (quinta armónica)
  audioPowerDown();
}

void playArrivalChime() {
  uint8_t vol = (nodeConfig.volume > 0) ? nodeConfig.volume : 80;
  playToneI2S(523, 100, vol); // C5
  delay(30);
  playToneI2S(659, 100, vol); // E5
  delay(30);
  playToneI2S(784, 180, vol); // G5
  audioPowerDown();
}

void playSuccessChime() {
  uint8_t vol = (nodeConfig.volume > 0) ? nodeConfig.volume : 80;
  playToneI2S(1046, 80, vol); // C6
  delay(40);
  playToneI2S(1318, 140, vol); // E6
  audioPowerDown();
}

void playWarningChime() {
  uint8_t vol = (nodeConfig.volume > 0) ? nodeConfig.volume : 80;
  playToneI2S(440, 150, vol);
  delay(80);
  playToneI2S(349, 250, vol);
  audioPowerDown();
}
