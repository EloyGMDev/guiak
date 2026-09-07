<div align="center">

# Guiak
### *Assistive Indoor Beacon & Navigation Ecosystem for Educational Institutions*
**[guiak.com](https://guiak.com)**

<p align="center">
  <a href="https://guiak.com"><img src="https://img.shields.io/badge/Website-guiak.com-00C49F?style=for-the-badge" alt="Website" /></a>
</p>

---

### Select Language / Selecciona Idioma / Tria Llengua

<p align="center">
  <a href="#english-documentation"><img src="https://img.shields.io/badge/Language-English-007ACC?style=flat-square" alt="English" /></a>
  &nbsp;&nbsp;
  <a href="#documentacion-en-castellano"><img src="https://img.shields.io/badge/Idioma-Castellano-EA4335?style=flat-square" alt="Castellano" /></a>
  &nbsp;&nbsp;
  <a href="#documentacio-en-catala"><img src="https://img.shields.io/badge/Llengua-Catala-FBBC05?style=flat-square" alt="Catala" /></a>
</p>

*(Click any language button above or expand the interactive sections below)*

</div>

> [!IMPORTANT]
> ### Project Continuity Notice / Aviso de Continuidad / Avis de Continuitat
> **English:** **Guiak** is the direct evolution and official successor of the legacy **SONA** project. The original SONA repository is permanently deprecated, discontinued, and will receive no further updates or bug fixes. All active research, architectural enhancements, cloud integrations, and firmware releases are developed exclusively within the Guiak ecosystem at [guiak.com](https://guiak.com).
> 
> **Castellano:** **Guiak** es la evolucion directa y sucesor oficial del proyecto heredado **SONA**. El repositorio original de SONA queda permanentemente descontinuado, obsoleto y no recibira mas actualizaciones ni parches. Todo el desarrollo activo, mejoras de arquitectura, integraciones en la nube y nuevas versiones del firmware se realizan exclusivamente dentro del ecosistema Guiak en [guiak.com](https://guiak.com).
> 
> **Catala:** **Guiak** es l evolucio directa i successor oficial del projecte heretat **SONA**. El repositori original de SONA queda permanentment descontinuat, obsolet i no rebra mes actualitzacions ni pegats. Tot el desenvolupament actiu, millores d arquitectura, integracions al nuvol i noves versions del microprogramari es desenvolupen exclusivament dins l ecosistema Guiak a [guiak.com](https://guiak.com).

---

<details open>
<summary><h2 id="english-documentation" style="display:inline-block; cursor:pointer;">English Documentation (Click to Collapse / Expand)</h2></summary>
<br>

### 1. System Overview
**Guiak** is an embedded IoT accessibility system engineered for classroom door frames and educational facilities. Combining **Bluetooth Low Energy (iBeacon and GATT)** with **high-definition digital audio synthesis**, it enables visually impaired students to detect classroom entrances via their personal smartphones or through on-demand acoustic orientation pulses.

By offloading device management, schedules, and logging directly to **Firebase Realtime Database**, the physical node no longer requires an on-board monolithic web server. This architectural shift enables seamless dual-platform deployment: production deployment on the ultra-low-power **ESP32-S3** as well as full native compatibility with the **Arduino UNO R4 WiFi**.

---

### 2. Key Capabilities
- **Dual-Platform Hardware Abstraction:** Unified codebase with automated compile-time HAL for either **ESP32-S3** (battery powered with I2S digital audio) or **Arduino UNO R4 WiFi** (mains or external battery with 12-bit DAC / buzzer).
- **Student Proximity & Presence Tracking:** Real-time location telemetry registering the last classroom doorway where visually impaired students were detected (via hands-free BLE proximity or physical RFID badge tap).
- **Firebase Real-Time Cloud Synchronization:** Room names, floor assignments, acoustic volumes, and schedules update dynamically without re-flashing nodes.
- **Offline Resilience Event Buffer:** If institutional Wi-Fi drops, RFID card reads and navigation logs are preserved in an in-memory circular buffer and automatically flushed to Firebase once network connectivity returns.
- **Remote Cloud Command Dispatcher:** Facility administrators can trigger sound pulses remotely, toggle school-wide safety lockdowns, or query node telemetry directly from the Firebase console.
- **Harmonic Acoustic Wayfinding:** Multi-tone harmonic chime envelopes mathematically optimized for spatial human echolocation without jarring acoustic distortion.
- **Smart Operational Schedule (07:00 - 20:30):** Automatic Wi-Fi Modem Sleep during school hours and deep peripheral shutdown at night to conserve power.
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
/
├── nodes/{roomCode}/
│     ├── config.json         <-- Classroom name, floor, volume, operational schedule
│     ├── status.json         <-- Heartbeat, battery %, mV, Wi-Fi RSSI, uptime
│     ├── command.json        <-- Cloud dispatcher (remote chime, lockdown mode)
│     ├── presence/{student}  <-- Students currently in range of this doorway
│     └── logs/               <-- Event audit trail (RFID taps, wayfinding pulses)
└── students/{studentId}/
      └── lastSeen.json       <-- Last detected room, floor, timestamp, and method (BLE/RFID)
```

</details>

---

<details>
<summary><h2 id="documentacion-en-castellano" style="display:inline-block; cursor:pointer;">Documentacion en Castellano (Haz clic para Desplegar / Contraer)</h2></summary>
<br>

### 1. Descripcion General
**Guiak** es un sistema embebido de accesibilidad disenado especificamente para instalarse en los marcos de las puertas de aulas e instalaciones educativas. Mediante tecnologia **Bluetooth Low Energy (iBeacon y GATT)** y **audio digital de alta definicion**, permite a los alumnos con discapacidad visual identificar la ubicacion exacta de las aulas mediante su telefono movil o a traves de pulsos acusticos de orientacion activados bajo demanda.

Al centralizar la gestion, los horarios y los registros en **Firebase Realtime Database**, el dispositivo ya no requiere un servidor web local monolitico. Esto permite un soporte dual transparente: despliegue optimizado en **ESP32-S3** (bateria y audio I2S) y compatibilidad directa con placas **Arduino UNO R4 WiFi**.

---

### 2. Capacidades Principales
- **Soporte Dual de Plataformas:** Mismo codigo fuente con capa de abstraccion HAL para compilar en **ESP32-S3** (modo ultra-bajo consumo con amplificador I2S) o en **Arduino UNO R4 WiFi** (salida analogica DAC de 12 bits / zumbador).
- **Registro y Seguimiento de Presencia de Alumnado:** Telemetria en tiempo real de la ultima aula donde estuvo cerca la alumna (tanto en modo manos libres por proximidad BLE como mediante confirmacion por tarjeta RFID).
- **Sincronizacion en Tiempo Real con Firebase:** Aulas, plantas, niveles de volumen y horarios se configuran de forma centralizada en la nube y se aplican en caliente sin reprogramar los nodos.
- **Cola de Eventos Fuera de Linea (Buffer Offline):** Si la red Wi-Fi escolar se interrumpe temporalmente, los fichajes RFID y las activaciones acusticas se conservan en un buffer circular en memoria y se transmiten automaticamente a Firebase al recuperar la conexion.
- **Despachador Remoto de Ordenes:** El equipo directivo puede activar la baliza acustica a distancia, activar el modo de confinamiento escolar de emergencia (lockdown) o consultar la telemetria en tiempo real desde Firebase.
- **Pulsos de Ecolocalizacion Armonica:** Secuencias de tonos con envolvente suave disenadas para una optima orientacion espacial tridimensional sin provocar sobresaltos auditivos.
- **Horario Inteligente (07:00 a 20:30):** Modo Wi-Fi Modem Sleep activo durante la jornada lectiva y apagado profundo nocturno para maximizar la autonomia.
- **Watchdog de Hardware:** Reinicio preventivo automatico ante posibles excepciones en la pila de red o perifericos.

---

### 3. Mapa de Conexion de Pines
*(Consultar la tabla de conexiones detallada en la seccion en ingles para ESP32-S3 y Arduino UNO R4 WiFi).*

---

### 4. Estructura de Datos en Firebase
- `/nodes/{roomCode}/config.json`: Configuracion remota (nombre del aula, planta, volumen y horario).
- `/nodes/{roomCode}/status.json`: Telemetria en vivo (latido, porcentaje de bateria, RSSI Wi-Fi y tiempo activo).
- `/nodes/{roomCode}/command.json`: Recepcion de comandos remotos (hacer sonar la baliza, activar confinamiento).
- `/nodes/{roomCode}/presence/{studentId}.json`: Registro de estudiantes detectados en el aula.
- `/nodes/{roomCode}/logs/`: Historico cronologico de accesos RFID y eventos de navegacion.
- `/students/{studentId}/lastSeen.json`: Ultima aula detectada para el alumno, planta, fecha/hora y metodo (BLE/RFID).

</details>

---

<details>
<summary><h2 id="documentacio-en-catala" style="display:inline-block; cursor:pointer;">Documentacio en Catala (Fes clic per Desplegar / Contraure)</h2></summary>
<br>

### 1. Descripcio General
**Guiak** es un sistema encastat d accessibilitat dissenyat per a ser installat als marcs de les portes d aules i installacions educatives. Mitjancant tecnologia **Bluetooth Low Energy (iBeacon i GATT)** i **audio digital d alta definicio**, permet a l alumnat amb discapacitat visual identificar la ubicacio exacta de les aules a traves del seu telefon mobil o mitjancant balises acustiques d orientacio activades sota demanda.

En centralitzar la gestio, els horaris i els registres a **Firebase Realtime Database**, el dispositiu ja no requereix un servidor web local monolititc. Aquest canvi d arquitectura permet un suport dual: desplegament optimitzat en **ESP32-S3** i compatibilitat directa amb plaques **Arduino UNO R4 WiFi**.

---

### 2. Capacitats Principals
- **Suport Dual de Plataformes:** El mateix codi font pot compilar-se tant en **ESP32-S3** com en **Arduino UNO R4 WiFi**.
- **Seguiment i Presencia de l Alumnat:** Telemetria en temps real de l ultima aula on s ha detectat l alumna (mitjancant mans lliures BLE o amb targeta RFID).
- **Sincronitzacio en Temps Real amb Firebase:** Les aules, horaris, volum i parametres es configuren al nuvol i s apliquen a l instant.
- **Cua d Esdeveniments Fora de Linia (Buffer Offline):** Si el Wi-Fi cau, els accessos RFID es guarden a la memoria local i es transmeten automaticament a Firebase en recuperar la connexio.
- **Despatxador Remot d Ordres:** Possibilitat d activar el so de l aula a distancia o ordenar un tancament d emergencia des de la consola central.
- **Polsos d Ecolocalitzacio Acustica:** Sons harmonics suaus optimitzats per a orientacio auditiva espacial sense estridencies.
- **Horari Intelligent (07:00 a 20:30):** Wi-Fi actiu en horari lectiu i desconnexio profunda nocturna per a la preservacio de la bateria.
- **Watchdog de Maquinari:** Recuperacio automatica davant fallades de xarxa o periferics.

---

### 3. Mapa de Connexions
*(Consulteu la taula de connexions detallada a la seccio en angles per a ESP32-S3 i Arduino UNO R4 WiFi).*

---

### 4. Estructura al Nuvol (Firebase)
- `/nodes/{roomCode}/config.json`: Configuracio remota de l aula, planta i volum.
- `/nodes/{roomCode}/status.json`: Estat en viu, bateria restant, RSSI Wi-Fi i temps en funcionament.
- `/nodes/{roomCode}/command.json`: Recepcio d ordres remotes (activacio acustica, mode confinament).
- `/nodes/{roomCode}/presence/{studentId}.json`: Estudiants detectats a l aula.
- `/nodes/{roomCode}/logs/`: Registre cronologic d accessos RFID i balises de navegacio.
- `/students/{studentId}/lastSeen.json`: Ultima aula detectada per a l alumne, planta, data/hora i metode (BLE/RFID).

</details>

---

### License / Licencia / Llicencia
Copyright (c) 2026 **EloyGM** ([guiak.com](https://guiak.com)). All rights reserved / Todos los derechos reservados / Tots els drets reservats.  
Consult [`LICENSE`](./LICENSE) for full legal terms and conditions.
