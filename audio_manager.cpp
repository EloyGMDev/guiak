#include "audio_manager.h"
#include "config.h"
#include "driver/i2s.h"
#include <math.h>

#define I2S_PORT          I2S_NUM_0
#define I2S_SAMPLE_RATE   22050

static bool i2sInstalled = false;

void audioInit() {
  // Configurar pin de Shutdown del amplificador MAX98357A
  pinMode(I2S_SD_PIN, OUTPUT);
  digitalWrite(I2S_SD_PIN, LOW); // Iniciar apagado (0 µA)

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

  // Asegurar amplificador apagado en reposo
  audioPowerDown();
}

void audioPowerDown() {
  digitalWrite(I2S_SD_PIN, LOW);
  isSoundPlaying = false;
}

void playToneI2S(uint16_t freqHz, uint16_t durationMs, uint8_t volumePercent) {
  if (!i2sInstalled || freqHz == 0 || durationMs == 0) return;

  isSoundPlaying = true;
  // 1. Despertar amplificador (HIGH = activo)
  digitalWrite(I2S_SD_PIN, HIGH);
  delay(10); // Pequeño retardo para estabilizar la alimentación del DAC

  // 2. Parámetros de la forma de onda
  volumePercent = constrain(volumePercent, 0, 100);
  int16_t maxAmplitude = (int16_t)((32767.0f * (volumePercent / 100.0f)) * 0.7f); // Evitar saturación

  size_t totalSamples = (I2S_SAMPLE_RATE * durationMs) / 1000;
  size_t attackSamples = totalSamples / 10;
  if (attackSamples > 500) attackSamples = 500;
  size_t decaySamples = attackSamples;

  const size_t CHUNK_SIZE = 128;
  int16_t buffer[CHUNK_SIZE * 2]; // Estéreo (L + R)

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
      
      // Envolvente de volumen para evitar chasquidos (clicks) al iniciar o parar
      float env = 1.0f;
      if (currentSample < attackSamples) {
        env = (float)currentSample / (float)attackSamples;
      } else if (currentSample > totalSamples - decaySamples) {
        env = (float)(totalSamples - currentSample) / (float)decaySamples;
      }

      int16_t sampleVal = (int16_t)(sinf(phase) * maxAmplitude * env);
      phase += phaseStep;
      if (phase >= 2.0f * (float)M_PI) phase -= 2.0f * (float)M_PI;

      // Duplicar para canal Izquierdo y Derecho (L + R)
      buffer[i * 2]     = sampleVal;
      buffer[i * 2 + 1] = sampleVal;
    }

    size_t bytesWritten = 0;
    i2s_write(I2S_PORT, buffer, chunk * 2 * sizeof(int16_t), &bytesWritten, portMAX_DELAY);
    samplesGenerated += chunk;
  }

  // Dejar que termine de vaciarse el búfer DMA
  delay(25);

  // 3. Volver a apagar el amplificador para mantener 0 µA de consumo
  audioPowerDown();
}

void playAcousticBeacon() {
  // Patrón sonoro dual (baliza acústica de alta localización tridimensional)
  // Dos tonos puros ascendentes con separación rítmica clara
  uint8_t vol = (nodeConfig.volume > 0) ? nodeConfig.volume : 80;
  
  digitalWrite(I2S_SD_PIN, HIGH);
  delay(10);
  
  playToneI2S(880,  120, vol);  // Nota A5
  delay(60);
  playToneI2S(1320, 220, vol);  // Nota E6 (Quinta armónica para ecolocalización precisa)
  
  audioPowerDown();
}

void playArrivalChime() {
  // Acorde agradable de llegada al aula
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
