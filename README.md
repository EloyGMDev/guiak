<div align="center">

<p align="center">
  <img src="logo.png" alt="Guiak" width="360" />
</p>

# Guiak
### *Assistive Indoor Beacon & Navigation Ecosystem for Educational Institutions*
**[guiak.com](https://guiak.com)**

<p align="center">
  <a href="https://guiak.com"><img src="https://img.shields.io/badge/Website-guiak.com-00C49F?style=for-the-badge" alt="Website" /></a>
  &nbsp;&nbsp;
  <a href="./apk/guiak-app-v3.0.apk"><img src="https://img.shields.io/badge/Android_App-Download_APK-3DDC84?style=for-the-badge&logo=android&logoColor=white" alt="Download Android APK" /></a>
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
> **English:** **Guiak** is the direct evolution and official successor of the legacy **SONA** project. The original SONA codebase was initially conceived as an educational prototype for the **mSchools** competition. Following that phase, the author evolved the architecture into an independent, personal initiative to support visually impaired navigation not only in schools and institutes, but also across public buildings and facilities. Consequently, the legacy SONA competition repository is permanently discontinued and will receive no further updates or bug fixes. All active development is maintained exclusively at [guiak.com](https://guiak.com).
> 
> **Castellano:** **Guiak** es la evolucion directa y sucesor oficial del proyecto heredado **SONA**. El repositorio original de SONA nacio como prototipo educativo para presentarse al concurso **mSchools**. Tras esa etapa inicial, su autor ha evolucionado el sistema hacia un proyecto personal e independiente con una meta mas amplia: ayudar a personas con discapacidad visual a orientarse tanto en institutos y centros educativos como en edificios publicos. Por ello, el repositorio inicial de SONA queda permanentemente descontinuado y el desarrollo activo continua exclusivamente dentro del ecosistema Guiak en [guiak.com](https://guiak.com).
> 
> **Catala:** **Guiak** es l evolucio directa i successor oficial del projecte heretat **SONA**. El repositori original de SONA va neixer com a prototip educatiu per a presentar-se al concurs **mSchools**. Despres d aquella etapa inicial, el seu autor ha evolucionat el sistema cap a un projecte personal i independent amb una meta mes amplia: ajudar persones amb discapacitat visual a orientar-se tant a instituts i centres educatius com a edificis publics. Per aixo, el repositori inicial de SONA queda permanentment descontinuat i el desenvolupament actiu continua exclusivament dins l ecosistema Guiak a [guiak.com](https://guiak.com).

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
- **Smart Operational Schedule (06:30 - 20:30):** Automatic Wi-Fi Modem Sleep during school hours and deep peripheral shutdown at night to conserve power.
- **Hardware Watchdog Guard:** Automatic hardware recovery in the event of peripheral timeouts or network stack hangs.
- **Automated Component Detection & Failover Alarm:** POST self-test and continuous background watchdog verifying all modules (RFID, battery ADC, BLE, NVS). If any component is missing or fails to respond, it triggers a **5-second dissonant alarm melody** prioritizing: **Buzzer** &rarr; **Speaker** (I2S/DAC) &rarr; **Status LEDs** (high-visibility strobe).

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
| | Piezo Buzzer (Alarm) | **GPIO 47** | Diagnostic failover alert output |

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

---

### 5. Guiak Mobile App for Android (Download APK)
**Guiak** includes an official Android companion application designed specifically for blind and visually impaired students navigating educational centers.

- **Direct Download:** [`apk/guiak-app-v3.0.apk`](./apk/guiak-app-v3.0.apk) (Ready to install on Android 8.0+).
- **Language Note:** *Currently, the mobile application user interface, synthetic voice prompts, and accessibility announcements are available exclusively in **Spanish (Castellano)**. Multilingual support (Catalan and English) is planned for upcoming releases.*

#### Key Mobile Features:
1. **100% TalkBack Native:** Complete semantic accessibility labels on every card, button, and indicator, designed for intuitive one-handed navigation while holding a white cane.
2. **"¿Dónde estoy?" Voice Assistance:** Tap anywhere on the high-contrast button to hear your current location, nearest classroom, floor, and estimated distance read aloud via text-to-speech.
3. **Giant "Hacer sonar puerta" Trigger:** A high-contrast yellow button that instantly triggers the physical node's acoustic orientation pulse on the doorframe via Bluetooth GATT with sub-second latency.
4. **Proximity Audio Radar:** Progressive acoustic sonar beeps that increase in tempo as the student approaches the classroom door.
5. **Classroom Arrival Confirmation:** Send attendance confirmation to the door node either via the app button or automatic beacon proximity.

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
- **Horario Inteligente (06:30 a 20:30):** Modo Wi-Fi Modem Sleep activo durante la jornada lectiva y apagado profundo nocturno para maximizar la autonomia.
- **Watchdog de Hardware:** Reinicio preventivo automatico ante posibles excepciones en la pila de red o perifericos.
- **Sistema de Deteccion de Componentes y Alarma Failover:** Autodiagnostico POST en el arranque y vigilancia continua en bucle de todos los modulos (RFID MFRC522, ADC de bateria, BLE y NVS). Si falta un componente o no responde, dispara una **alarma de 5 segundos con melodia disonante ("chunga")** con jerarquia: **Buzzer** &rarr; **Altavoz** (I2S/DAC) &rarr; **LEDs de estado** (destello de emergencia).

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

---

### 5. Aplicacion Movil Guiak para Android (Descarga de APK)
El ecosistema **Guiak** incluye una aplicacion movil oficial para Android desarrollada especificamente para estudiantes con ceguera o baja vision.

- **Descarga directa:** [`apk/guiak-app-v3.0.apk`](./apk/guiak-app-v3.0.apk) (Archivo APK listo para instalar en Android 8.0 o superior).
- **Nota de idioma:** *Actualmente, la aplicacion movil, sus locuciones por voz (TTS) y todos los textos de la interfaz estan disponibles **unicamente en castellano**. En futuras versiones se incorporaran traducciones al catalan y al ingles.*

#### Funcionalidades Principales de la App:
1. **Accesibilidad Total con TalkBack:** Botones gigantes de alto contraste (negro puro, amarillo trafico `#FFD700` y cyan `#00E5FF`) y etiquetas semanticas completas, optimizadas para su manejo con una sola mano mientras se camina con baston.
2. **Boton "¿Donde estoy?" (Repetir voz):** Pulsa la pantalla para que la sintesis de voz te anuncie al instante el aula mas cercana, la planta del edificio y los metros de distancia estimados.
3. **Boton gigante "HACER SONAR PUERTA":** Activa de forma inmediata el zumbador o altavoz fisico instalado en el marco de la puerta del aula mediante Bluetooth GATT para orientarse por ecolocalizacion.
4. **Radar Sonoro de Proximidad:** Emite pitidos de sonar que aumentan su cadencia a medida que la persona se acerca a la puerta.
5. **Confirmacion de Llegada:** Permite confirmar la entrada al aula y registrar la asistencia directamente en Firebase.
6. **Emisor de Baliza del Alumno:** Convierte el smartphone en una baliza emisora para que las puertas detecten la llegada del alumno automaticamente.

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
- **Horari Intelligent (06:30 a 20:30):** Wi-Fi actiu en horari lectiu i desconnexio profunda nocturna per a la preservacio de la bateria.
- **Watchdog de Maquinari:** Recuperacio automatica davant fallades de xarxa o periferics.
- **Sistema de Deteccio de Components i Alarma Failover:** Autodiagnosi POST a l arrancada i vigilancia continua en bucle de tots els moduls (RFID MFRC522, ADC de bateria, BLE i NVS). Si falta un component o no respon, dispara una **alarma de 5 segons amb melodia dissonant ("xunga")** amb jerarquia: **Buzzer** &rarr; **Altveu** (I2S/DAC) &rarr; **LEDs d estat** (parpelleig d emergencia).

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

---

### 5. Aplicacio Mobil Guiak per a Android (Descarrega d'APK)
L'ecosistema **Guiak** inclou una aplicacio mobil oficial per a Android desenvolupada especificament per a estudiants amb discapacitat visual o baixa visio.

- **Descarrega directa:** [`apk/guiak-app-v3.0.apk`](./apk/guiak-app-v3.0.apk) (Fitxer APK llest per a installar en Android 8.0 o superior).
- **Nota d'idioma:** *Actualment, l'aplicacio mobil, les seves locucions per veu (TTS) i tots els textos de la interficie estan disponibles **unicament en castella**. En futures versions s'incorporaran traduccions al catala i a l'angles.*

#### Funcionalitats Principals de l'App:
1. **Accessibilitat Total amb TalkBack:** Botons gegants d'alt contrast (negre pur, groc transit `#FFD700` i cian `#00E5FF`) i etiquetes semantiques completes pensades per a utilitzar amb una sola ma mentre es camina amb basto.
2. **Boto "¿Donde estoy?" (Repetir veu):** Toca la pantalla per a escoltar per sintesi de veu l'aula mes propera, la planta i la distancia estimada.
3. **Boto gegant "HACER SONAR PUERTA":** Activa a l'instant el pols acustic del marc de la porta via Bluetooth GATT per a orientar-se per ecolocalitzacio.
4. **Radar Sonor de Proximitat:** Emet xiulets continus que acceleren com mes a prop estiguis de la porta.
5. **Confirmacio d'Arribada:** Confirma l'entrada a l'aula i registra la presencia a Firebase.

</details>

---

### License / Licencia / Llicencia
Copyright (c) 2026 **EloyGM** ([guiak.com](https://guiak.com)). All rights reserved / Todos los derechos reservados / Tots els drets reservats.  
Consult [`LICENSE`](./LICENSE) for full legal terms and conditions.
