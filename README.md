<div align="center">

# Guiak System v3.0 (guiak.com)

**Ecosistema de Balizas Inteligentes y Navegación Asistida para Perguiaks con Discapacidad Visual en Entornos Educativos.**  
Desarrollado para microcontroladores **ESP32-S3** con arquitectura de **Ultra-Bajo Consumo (ULP)** a batería, audio digital I2S de alta definición, balizas **iBeacon / BLE GATT** para localización acústica bajo demanda y portal de configuración Wi-Fi.

[![Version](https://img.shields.io/badge/version-3.0.0--ESP32--ULP-00d2ff?style=flat-square)](#)
[![Platform](https://img.shields.io/badge/platform-ESP32--S3-e056fd?style=flat-square)](#)
[![Power](https://img.shields.io/badge/battery-4--6%20Months-00ff88?style=flat-square)](#)
[![Audio](https://img.shields.io/badge/audio-I2S%20MAX98357A-f0932b?style=flat-square)](#)

---

</div>

## 1. Visión del Proyecto y Experiencia de Usuario

Guiak transforma cada marco de puerta de un instituto en un **faro inteligente de accesibilidad**:

1. **Navegación por el Pasillo (Móvil en el Bolsillo):** El smartphone del alumno escanea en segundo plano las tramas iBeacon de cada aula y le indica mediante **TalkBack** por qué clase va pasando (*"Aula 102 a la derecha, cruce a 10 metros"*).
2. **Localización Acústica Inmediata ("Efecto AirTag"):** Al acercarse al aula de destino, la app móvil o el alumno (con doble clic en el botón de volumen del móvil) manda una orden Bluetooth silenciosa. El altavoz de la puerta emite un **pulso armónico de ecolocalización** (*"Aquí, Aula 104"*), permitiéndole caminar directo al pomo guiado por su propio oído.
3. **Cero Cables (Alimentación a Batería):** Los nodos se instalan en minutos con cinta de doble cara en el marco de la puerta. Una batería Li-Ion 18650 proporciona de **4 a 6 meses de autonomía continua**.
4. **Fichaje de Asistencia:** Lector RFID (RC522) integrado para registro físico de presencia de apoyo.

---

## 2. Mapa de Conexión de Pines (ESP32-S3 DevKit-C)

```
                       ESP32-S3 DevKit-C
                    ┌─────────────────────┐
                    │                     │
      MAX98357A BCLK├──GPIO 15            │
       MAX98357A LRC├──GPIO 16            │
       MAX98357A DIN├──GPIO 17            │
   MAX98357A SD_MODE├──GPIO 18            │
                    │                     │
       RC522 SS(SDA)├──GPIO 10            │
           RC522 RST├──GPIO 9             │
          RC522 MOSI├──GPIO 11            │
          RC522 MISO├──GPIO 13            │
           RC522 SCK├──GPIO 12            │
                    │                     │
   Divisor Batería  ├──GPIO 4  (ADC1_CH3) │
   Botón Config     ├──GPIO 0  (o GPIO 7) │
   LED Estado RGB   ├──GPIO 48            │
                    └─────────────────────┘
```

---

## 3. Arquitectura de Ultra-Bajo Consumo (Batería)

* **Consumo en Reposo (~0,8 mA):** La radio Wi-Fi permanece **100% apagada**. El nodo emite publicidad iBeacon brevemente (3 ms) cada 500 ms y entra en estado de reposo (*Light Sleep*).
* **Amplificador de Audio con Corte de Fuga (0 µA):** El pin `I2S_SD_PIN` (GPIO 18) apaga por hardware el chip MAX98357A cuando no está guiakndo. Solo consume energía los 2 segundos que emite sonido.
* **Portal Wi-Fi Bajo Demanda:** Al mantener presionado el botón físico 2,5 segundos, el nodo levanta un Punto de Acceso (`Guiak-Config-[Aula]`) y servidor web para configurar el aula. Se auto-apaga a los 5 minutos para evitar que la batería se descargue por descuido.
* **Monitorización de Batería:** Conversión por ADC del divisor resistivo (3.3V a 4.2V), reportando el porcentaje de batería por Bluetooth a la app del alumno y del centro.

---

## 4. Módulos del Firmware

| Archivo | Responsabilidad |
| :--- | :--- |
| [`Guiak.ino`](file:///c:/Users/eloyg/Guiak/Guiak.ino) | Bucle principal, máquina de estados (Batería vs Configuración) |
| [`config.h`](file:///c:/Users/eloyg/Guiak/config.h) | Mapa de pines, estructura `NodeConfig` y constantes de energía |
| [`ble_beacon.h`](file:///c:/Users/eloyg/Guiak/ble_beacon.h) / `.cpp` | Emisión iBeacon y Servidor GATT (servicio `0xFD00`, disparo acústico `0xFD01`, batería `0x180F`) |
| [`audio_manager.h`](file:///c:/Users/eloyg/Guiak/audio_manager.h) / `.cpp` | Controlador I2S, síntesis senoidal suave y gestión del pin de corte `SD_MODE` |
| [`power_manager.h`](file:///c:/Users/eloyg/Guiak/power_manager.h) / `.cpp` | Lectura analógica por ADC, cálculo de porcentaje y rutinas de bajo consumo |
| [`database.h`](file:///c:/Users/eloyg/Guiak/database.h) / `.cpp` | Persistencia en memoria Flash NVS (`Preferences`) eliminando la EEPROM estática |
| [`web_server.h`](file:///c:/Users/eloyg/Guiak/web_server.h) / `.cpp` | Servidor HTTP bajo demanda con interfaz web oscura, responsive y prueba de audio |
| [`hardware_io.h`](file:///c:/Users/eloyg/Guiak/hardware_io.h) / `.cpp` | Gestión de pulsaciones de botón y lectura RFID con filtro anti-rebote |
| [`utils.h`](file:///c:/Users/eloyg/Guiak/utils.h) / `.cpp` | Logs formateados y telemetría de memoria RAM/PSRAM |

---

## 5. Portal de Administración Web (Bajo Demanda)

1. Mantén pulsado el botón físico del nodo durante **2,5 segundos**.
2. Conéctate a la red Wi-Fi generada: **`Guiak-Config-104`** (Contraseña: `guiak1234`).
3. Abre tu navegador en **`http://192.168.4.1`**.
4. Podrás:
   * Cambiar el nombre descriptivo del aula, planta y código corto.
   * Ajustar el volumen del altavoz (0–100%) y probar el sonido en directo con el botón **"Probar Sonido en Puerta"**.
   * Comprobar el nivel exacto de batería en milivoltios y porcentaje.
   * Al terminar, pulsar **"Apagar Wi-Fi y Volver a Modo Batería"** (o se apagará solo tras 5 minutos).

---

## 6. Integración con la App Android Guiak

El firmware está preparado para comunicarse nativamente con la app de Android:
* **Escaneo en segundo plano:** La app detecta el UUID de proximidad `FDA50693-A4E2-4FB1-AFCF-C6EB07647825`.
* **Activación por Bluetooth GATT:** Escribiendo un byte `0x01` en la característica `0000FD01-0000-1000-8000-00805F9B34FB`, la puerta suena en menos de **50 milisegundos**.
* **Telemetría de Batería:** La app lee periódicamente el nivel de carga a través de la característica estándar Bluetooth `0x2A19`.

---

## Licencia
Copyright (c) 2026 EloyGM. Todos los derechos reservados. Consulta [LICENSE](file:///c:/Users/eloyg/Guiak/LICENSE) para más detalles.
