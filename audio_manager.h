#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <Arduino.h>

// Inicializa el bus I2S y configura el pin de Shutdown del amplificador MAX98357A
void audioInit();

// Reproduce la baliza acústica de localización (pulso armónico optimizado para orientación 3D)
void playAcousticBeacon();

// Reproduce un acorde de confirmación de llegada
void playArrivalChime();

// Reproduce un tono de confirmación de acceso / RFID
void playSuccessChime();

// Reproduce un tono de aviso / error / batería baja
void playWarningChime();

// Genera y reproduce un tono senoidal puro con envolvente suave a través de I2S
void playToneI2S(uint16_t freqHz, uint16_t durationMs, uint8_t volumePercent = 80);

// Apaga manualmente el amplificador a 0 µA
void audioPowerDown();

#endif // AUDIO_MANAGER_H
