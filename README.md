<div align="center">

# 🧭 Guiak
### *Assistive Indoor Beacon & Navigation Ecosystem for Educational Institutions*
**[guiak.com](https://guiak.com)**

<p align="center">
  <a href="https://guiak.com"><img src="https://img.shields.io/badge/Website-guiak.com-00C49F?style=for-the-badge&logo=firefox-browser&logoColor=white" alt="Website" /></a>
  <img src="https://img.shields.io/badge/Firmware-v3.0.0%20Production-blue?style=for-the-badge&logo=cplusplus&logoColor=white" alt="Version" />
  <img src="https://img.shields.io/badge/Hardware-ESP32--S3%20%7C%20UNO%20R4%20WiFi-blueviolet?style=for-the-badge&logo=arduino&logoColor=white" alt="Hardware" />
  <img src="https://img.shields.io/badge/Cloud-Firebase%20Realtime%20DB-FFCA28?style=for-the-badge&logo=firebase&logoColor=black" alt="Firebase" />
  <img src="https://img.shields.io/badge/Wireless-BLE%205.0%20%2B%20Wi--Fi-0082FC?style=for-the-badge&logo=bluetooth&logoColor=white" alt="BLE" />
  <img src="https://img.shields.io/badge/Audio-I2S%20HD%20%2F%2012--bit%20DAC-FF5722?style=for-the-badge&logo=soundcharts&logoColor=white" alt="Audio" />
  <img src="https://img.shields.io/badge/License-EloyGM%20Proprietary-success?style=for-the-badge" alt="License" />
</p>

---

### 🌐 Select Language / Selecciona Idioma / Tria Llengua

<p align="center">
  <a href="#-english-documentation"><img src="https://img.shields.io/badge/Language-English-007ACC?style=flat-square&logo=google-translate&logoColor=white" alt="English" /></a>
  &nbsp;&nbsp;
  <a href="#-documentación-en-castellano"><img src="https://img.shields.io/badge/Idioma-Castellano-EA4335?style=flat-square&logo=google-translate&logoColor=white" alt="Castellano" /></a>
  &nbsp;&nbsp;
  <a href="#-documentació-en-català"><img src="https://img.shields.io/badge/Llengua-Català-FBBC05?style=flat-square&logo=google-translate&logoColor=white" alt="Català" /></a>
</p>

*(Click any language button above or expand the interactive sections below)*

</div>

> [!IMPORTANT]
> ### 📢 Project Continuity Notice / Aviso de Continuidad / Avís de Continuïtat
> **English:** **Guiak** is the direct evolution and official successor of the legacy **SONA** project. The original SONA repository is permanently deprecated, discontinued, and will receive no further updates or bug fixes. All active research, architectural enhancements, cloud integrations, and firmware releases are developed exclusively within the Guiak ecosystem at [guiak.com](https://guiak.com).
> 
> **Castellano:** **Guiak** es la evolución directa y sucesor oficial del proyecto heredado **SONA**. El repositorio original de SONA queda permanentemente descontinuado, obsoleto y no recibirá más actualizaciones ni parches. Todo el desarrollo activo, mejoras de arquitectura, integraciones en la nube y nuevas versiones del firmware se realizan exclusivamente dentro del ecosistema Guiak en [guiak.com](https://guiak.com).
> 
> **Català:** **Guiak** és l'evolució directa i successor oficial del projecte heretat **SONA**. El repositori original de SONA queda permanentment descontinuat, obsolet i no rebrà més actualitzacions ni pegats. Tot el desenvolupament actiu, millores d'arquitectura, integracions al núvol i noves versions del microprogramari es desenvolupen exclusivament dins l'ecosistema Guiak a [guiak.com](https://guiak.com).

---

<details open>
<summary><h2 id="-english-documentation" style="display:inline-block; cursor:pointer;">🇬🇧 English Documentation (Click to Collapse / Expand)</h2></summary>
<br>

### 1. System Overview
**Guiak** is an embedded IoT accessibility system engineered for classroom door frames and educational facilities. Combining **Bluetooth Low Energy (iBeacon and GATT)** with **high-definition digital audio synthesis**, it enables visually impaired students to detect classroom entrances via their personal smartphones or through on-demand acoustic orientation pulses.

By offloading device management, schedules, and logging directly to **Firebase Realtime Database**, the physical node no longer requires an on-board monolithic web server. This architectural shift enables seamless dual-platform deployment: production deployment on the ultra-low-power **ESP32-S3** as well as full native compatibility with the **Arduino UNO R4 WiFi**.

---

### 2. Key Capabilities
- **Dual-Platform Hardware Abstraction:** Unified codebase with automated compile-time HAL for either **ESP32-S3** (battery powered with I2S digital audio) or **Arduino UNO R4 WiFi** (mains or external battery with 12-bit DAC / buzzer).
- **Firebase Real-Time Cloud Synchronization:** Room names, floor assignments, acoustic volumes, and schedules update dynamically without re-flashing nodes.
- **Offline Resilience Event Buffer:** If institutional Wi-Fi drops, RFID card reads and navigation logs are preserved in an in-memory circular buffer and automatically flushed to Firebase once network connectivity returns.
- **Remote Cloud Command Dispatcher:** Facility administrators can trigger sound pulses remotely, toggle school-wide safety lockdowns, or query node telemetry directly from the Firebase console.
- **Harmonic Acoustic Wayfinding:** Multi-tone harmonic chime envelopes mathematically optimized for spatial human echolocation without jarring acoustic distortion.
- **Smart Operational Schedule (07:00 – 20:30):** Automatic Wi-Fi Modem Sleep during school hours and deep peripheral shutdown at night to conserve power.
- **Hardware Watchdog Guard:** Automatic hardware recovery in the event of peripheral timeouts or network stack hangs.

---

### 3. Pin Connection Mapping

#### ESP32-S3 DevKit-C
| Peripheral | Function | GPIO Pin | Notes |
| :--- | :--- | :---: | :--- |
| **I2S DAC / Amp** | BCLK (Bit Clock) | **GPIO 15** | MAX98357A / PCM5102 |
| | LRC / WSEL (Word Select) | **GPIO 16** | Left / Right audio clock |
| | DIN (Audio Data) | **GPIO 17** | Digital serial audio |
| | SD_MODE (Power Shutdown) | **GPIO 18** | Ultra-low-power mute control |
| **MFRC522 RFID** | SS / SDA (Chip Select) | **GPIO 10** | SPI Chip Select |
| | SCK (SPI Clock) | **GPIO 12** | SPI Clock |
| | MOSI (Data In) | **GPIO 11** | SPI MOSI |
| | MISO (Data Out) | **GPIO 13** | SPI MISO |
| | RST (Reset) | **GPIO 9** | Hardware reset |
| **Sensors & I/O** | Battery Voltage Divider | **GPIO 4** | ADC1_CH3 (3.3V scaled) |
| | Hardware Test Button | **GPIO 0** | Pulled-up active LOW |
| | Status Indicator LED | **GPIO 48** | On-board addressable RGB / LED |

#### Arduino UNO R4 WiFi
| Peripheral | Function | Board Pin | Notes |
| :--- | :--- | :---: | :--- |
| **Audio Output** | Analog DAC | **Pin A0** | 12-bit true analog DAC output |
| | Tone Buzzer (Fallback) | **Pin D5** | PWM acoustic wave generator |
| **MFRC522 RFID** | SS / SDA | **Pin D10** | Hardware SPI CS |
| | SCK | **Pin D13** | Hardware SPI SCK |
| | MOSI | **Pin D11** | Hardware SPI MOSI |
| | MISO | **Pin D12** | Hardware SPI MISO |
| | RST | **Pin D9** | Hardware reset |
| **Controls** | Test / Config Button | **Pin D7** | Internal pull-up |

---

### 4. Firebase Cloud Architecture

```json
/nodes/{roomCode}/
  ├── config.json       <-- Classroom name, floor, volume, schedule
  ├── status.json       <-- Heartbeat, battery %, mV, Wi-Fi RSSI, uptime
  ├── command.json      <-- Cloud dispatcher (remote ping, lockdown toggle)
  └── logs/             <-- Timestamped event logs (RFID badges, wayfinding triggers)
```

</details>

---

<details>
<summary><h2 id="-documentación-en-castellano" style="display:inline-block; cursor:pointer;">🇪🇸 Documentación en Castellano (Haz clic para Desplegar / Contraer)</h2></summary>
<br>

### 1. Descripción General
**Guiak** es un sistema embebido de accesibilidad diseñado específicamente para instalarse en los marcos de las puertas de aulas e instalaciones educativas. Mediante tecnología **Bluetooth Low Energy (iBeacon y GATT)** y **audio digital de alta definición**, permite a los alumnos con discapacidad visual identificar la ubicación exacta de las aulas mediante su teléfono móvil o a través de pulsos acústicos de orientación activados bajo demanda.

Al centralizar la gestión, los horarios y los registros en **Firebase Realtime Database**, el dispositivo ya no requiere un servidor web local monolítico. Esto permite un soporte dual transparente: despliegue optimizado en **ESP32-S3** (batería y audio I2S) y compatibilidad directa con placas **Arduino UNO R4 WiFi**.

---

### 2. Capacidades Principales
- **Soporte Dual de Plataformas:** Mismo código fuente con capa de abstracción HAL para compilar en **ESP32-S3** (modo ultra-bajo consumo con amplificador I2S) o en **Arduino UNO R4 WiFi** (salida analógica DAC de 12 bits / zumbador).
- **Sincronización en Tiempo Real con Firebase:** Aulas, plantas, niveles de volumen y horarios se configuran de forma centralizada en la nube y se aplican en caliente sin reprogramar los nodos.
- **Cola de Eventos Fuera de Línea (Buffer Offline):** Si la red Wi-Fi escolar se interrumpe temporalmente, los fichajes RFID y las activaciones acústicas se conservan en un buffer circular en memoria y se transmiten automáticamente a Firebase al recuperar la conexión.
- **Despachador Remoto de Órdenes:** El equipo directivo puede activar la baliza acústica a distancia, activar el modo de confinamiento escolar de emergencia (lockdown) o consultar la telemetría en tiempo real desde Firebase.
- **Pulsos de Ecolocalización Armónica:** Secuencias de tonos con envolvente suave diseñadas para una óptima orientación espacial tridimensional sin provocar sobresaltos auditivos.
- **Horario Inteligente (07:00 a 20:30):** Modo Wi-Fi Modem Sleep activo durante la jornada lectiva y apagado profundo nocturno para maximizar la autonomía.
- **Watchdog de Hardware:** Reinicio preventivo automático ante posibles excepciones en la pila de red o periféricos.

---

### 3. Mapa de Conexión de Pines
*(Consultar la tabla de conexiones detallada en la sección en inglés para ESP32-S3 y Arduino UNO R4 WiFi).*

---

### 4. Estructura de Datos en Firebase
- `/nodes/{roomCode}/config.json`: Configuración remota (nombre del aula, planta, volumen y horario).
- `/nodes/{roomCode}/status.json`: Telemetría en vivo (latido, porcentaje de batería, RSSI Wi-Fi y tiempo activo).
- `/nodes/{roomCode}/command.json`: Recepción de comandos remotos (hacer sonar la baliza, activar confinamiento).
- `/nodes/{roomCode}/logs/`: Histórico cronológico de accesos RFID y eventos de navegación.

</details>

---

<details>
<summary><h2 id="-documentació-en-català" style="display:inline-block; cursor:pointer;">🏴 Documentació en Català (Fes clic per Desplegar / Contraure)</h2></summary>
<br>

### 1. Descripció General
**Guiak** és un sistema encastat d'accessibilitat dissenyat per a ser instal·lat als marcs de les portes d'aules i instal·lacions educatives. Mitjançant tecnologia **Bluetooth Low Energy (iBeacon i GATT)** i **àudio digital d'alta definició**, permet a l'alumnat amb discapacitat visual identificar la ubicació exacta de les aules a través del seu telèfon mòbil o mitjançant balises acústiques d'orientació activades sota demanda.

En centralitzar la gestió, els horaris i els registres a **Firebase Realtime Database**, el dispositiu ja no requereix un servidor web local monolític. Aquest canvi d'arquitectura permet un suport dual: desplegament optimitzat en **ESP32-S3** i compatibilitat directa amb plaques **Arduino UNO R4 WiFi**.

---

### 2. Capacitats Principals
- **Suport Dual de Plataformes:** El mateix codi font pot compilar-se tant en **ESP32-S3** com en **Arduino UNO R4 WiFi**.
- **Sincronització en Temps Real amb Firebase:** Les aules, horaris, volum i paràmetres es configuren al núvol i s'apliquen a l'instant.
- **Cua d'Esdeveniments Fora de Línia (Buffer Offline):** Si el Wi-Fi cau, els accessos RFID es guarden a la memòria local i es transmeten automàticament a Firebase en recuperar la connexió.
- **Despatxador Remot d'Ordres:** Possibilitat d'activar el so de l'aula a distància o ordenar un tancament d'emergència des de la consola central.
- **Polsos d'Ecolocalització Acústica:** Sons harmònics suaus optimitzats per a orientació auditiva espacial sense estridències.
- **Horari Intel·ligent (07:00 a 20:30):** Wi-Fi actiu en horari lectiu i desconnexió profunda nocturna per a la preservació de la bateria.
- **Watchdog de Maquinari:** Recuperació automàtica davant fallades de xarxa o perifèrics.

---

### 3. Mapa de Connexions
*(Consulteu la taula de connexions detallada a la secció en anglès per a ESP32-S3 i Arduino UNO R4 WiFi).*

---

### 4. Estructura al Núvol (Firebase)
- `/nodes/{roomCode}/config.json`: Configuració remota de l'aula, planta i volum.
- `/nodes/{roomCode}/status.json`: Estat en viu, bateria restant, RSSI Wi-Fi i temps en funcionament.
- `/nodes/{roomCode}/command.json`: Recepció d'ordres remotes (activació acústica, mode confinament).
- `/nodes/{roomCode}/logs/`: Registre cronològic d'accessos RFID i balises de navegació.

</details>

---

### ⚖️ License / Licencia / Llicència
Copyright © 2026 **EloyGM** ([guiak.com](https://guiak.com)). All rights reserved / Todos los derechos reservados / Tots els drets reservats.  
Consult [`LICENSE`](./LICENSE) for full legal terms and conditions.
