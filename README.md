# Guiak (guiak.com)

Assistive indoor beacon and navigation ecosystem for visually impaired orientation in educational institutions.
Sistema de balizas inteligentes y navegacion asistida en interiores para personas con discapacidad visual en centros educativos.
Sistema de balises intelligets i navegacio assistida en interiors per a persones amb discapacitat visual en centres educatius.

---

## Project Continuity Notice / Aviso de Continuidad / Avis de Continuitat

### English
Guiak is the direct evolution and official successor of the legacy SONA project. The original SONA repository is permanently deprecated, discontinued, and will receive no further updates or bug fixes. All active research, architectural enhancements, cloud integrations, and firmware releases are developed exclusively within the Guiak ecosystem at guiak.com.

### Castellano
Guiak es la evolucion directa y sucesor oficial del proyecto heredado SONA. El repositorio original de SONA queda permanentemente descontinuado, obsoleto y no recibira mas actualizaciones ni parches. Todo el desarrollo activo, mejoras de arquitectura, integraciones en la nube y nuevas versiones del firmware se realizan exclusivamente dentro del ecosistema Guiak en guiak.com.

### Catala
Guiak es l evolucio directa i successor oficial del projecte heretat SONA. El repositori original de SONA queda permanentment descontinuat, obsolet i no rebra mes actualitzacions ni pegats. Tot el desenvolupament actiu, millores d arquitectura, integracions al nuvol i noves versions del microprogramari es desenvolupen exclusivament dins l ecosistema Guiak a guiak.com.

---

## English

### 1. Overview
Guiak is an embedded IoT accessibility system designed for classroom door frames and educational facilities. Using Bluetooth Low Energy (iBeacon and GATT) combined with high-definition digital audio, it allows visually impaired students to identify classroom entrances via their smartphone or through on-demand acoustic orientation pulses.

By offloading device management, schedules, and logging to Firebase Realtime Database, the hardware no longer requires a monolithic on-board web server, enabling dual-platform support: deployment on ESP32-S3 as well as native compatibility with Arduino UNO R4 WiFi.

### 2. Key System Capabilities
* Dual-Platform Hardware Abstraction: Single codebase capable of compiling for either ESP32-S3 (ultra-low-power battery mode with I2S audio) or Arduino UNO R4 WiFi (mains/power bank with analog DAC/buzzer).
* Firebase Real-Time Cloud Synchronization: Classrooms, schedules, volume levels, and access permissions are configured centrally. When cloud data updates, nodes adapt instantly without re-flashing.
* Offline Resilience Buffer: Access events and RFID transactions are stored in a local circular queue if Wi-Fi connection drops, automatically flushing to Firebase upon reconnection.
* Remote Cloud Command Dispatcher: Administrators can remotely trigger the acoustic beacon, enforce emergency center lockdowns, or query node telemetry directly from Firebase.
* Acoustic Wayfinding Pulses: Multi-tone harmonic chime envelopes optimized for spatial human echolocation without harsh distortion.
* Smart Operational Schedule (07:00 to 20:30): Active Wi-Fi Modem Sleep during school hours and deep radio shutdown at night to conserve battery.
* Hardware Watchdog Timer: Automatic self-recovery if network connectivity or peripheral loops experience hardware exceptions.

### 3. Pin Connection Mapping

#### ESP32-S3 DevKit-C
* I2S BCLK (Bit Clock): GPIO 15
* I2S LRC (Word Select): GPIO 16
* I2S DIN (Audio Data): GPIO 17
* I2S SD (Power Shutdown): GPIO 18
* RFID SS / SDA: GPIO 10
* RFID RST: GPIO 9
* RFID SCK: GPIO 12
* RFID MISO: GPIO 13
* RFID MOSI: GPIO 11
* Battery ADC Divider: GPIO 4 (ADC1_CH3)
* Configuration / Test Button: GPIO 0 (or GPIO 7)
* Status LED: GPIO 48

#### Arduino UNO R4 WiFi
* Audio Output: Pin A0 (Analog DAC 12-bit) / Pin D5 (Tone Buzzer)
* RFID SS / SDA: Pin D10
* RFID RST: Pin D9
* RFID SCK: Pin D13
* RFID MISO: Pin D12
* RFID MOSI: Pin D11
* Manual Button: Pin D7

### 4. Firebase Cloud Architecture
* /nodes/{roomCode}/config.json: Remote configuration (room name, floor, volume, operational hours).
* /nodes/{roomCode}/status.json: Live heartbeat, battery percentage, cell millivolts, Wi-Fi RSSI, uptime.
* /nodes/{roomCode}/commands.json: Cloud-to-node action dispatcher (remote sound trigger, lockdown toggle).
* /nodes/{roomCode}/logs.json: Historical transaction logs (RFID accesses, acoustic beacon events, alerts).

---

## Castellano

### 1. Descripcion General
Guiak es un sistema embebido de accesibilidad disenado para instalarse en los marcos de las puertas de aulas e instalaciones educativas. Mediante tecnologia Bluetooth Low Energy (iBeacon y GATT) y audio de alta definicion, permite a los alumnos con discapacidad visual identificar la ubicacion exacta de las aulas a traves de su telefono movil o mediante balizas acusticas de orientacion activadas bajo demanda.

Al centralizar la gestion, los horarios y los registros en Firebase Realtime Database, el dispositivo ya no requiere un servidor web local monolitico, permitiendo un soporte dual: despliegue optimizado en ESP32-S3 y compatibilidad directa con placas Arduino UNO R4 WiFi.

### 2. Mejoras y Capacidades Principales
* Soporte Dual de Plataformas: Mismo codigo fuente capaz de compilarse tanto en ESP32-S3 (modo bateria ultra-bajo consumo con audio I2S) como en Arduino UNO R4 WiFi (alimentacion por red/bateria con salida DAC/buzzer).
* Sincronizacion en Tiempo Real con Firebase: Aulas, horarios, volumen y permisos se configuran centralizadamente. Cuando los datos cambian en la nube, los nodos se adaptan al instante sin necesidad de reprogramarlos.
* Cola de Eventos Fuera de Linea (Buffer Offline): Si la conexion Wi-Fi del centro cae temporalmente, los fichajes RFID y accesos se guardan en una cola circular local, subiendose a Firebase automaticamente al restablecerse la red.
* Despachador Remoto de Comandos: El equipo directivo puede activar la baliza acustica en remoto, activar un cierre de emergencia (lockdown) o consultar la telemetria desde Firebase.
* Pulsos de Ecolocalizacion Acustica: Secuencias armonicas de tonos puros con envolvente suave disenadas para orientacion tridimensional sin estridencias.
* Horario Inteligente (07:00 a 20:30): Wi-Fi Modem Sleep activo durante la jornada escolar y desconexion total nocturna para maxima autonomia.
* Watchdog de Hardware: Reinicio preventivo automatico en caso de caidas o excepciones en la pila de red.

### 3. Mapa de Conexion de Pines
Ver seccion en ingles para tablas completas de pines para ESP32-S3 y Arduino UNO R4 WiFi.

### 4. Estructura de Datos en Firebase
* /nodes/{roomCode}/config.json: Configuracion remota del aula, planta y volumen.
* /nodes/{roomCode}/status.json: Estado en vivo, porcentaje de bateria, RSSI Wi-Fi y tiempo activo.
* /nodes/{roomCode}/commands.json: Recepcion de ordenes remotas (sonar aula, modo emergencia).
* /nodes/{roomCode}/logs.json: Registro cronologico de accesos RFID y eventos de navegacion.

---

## Catala

### 1. Descripcio General
Guiak es un sistema encastat d accessibilitat dissenyat per a ser installat als marcs de les portes d aules i installacions educatives. Mitjancant tecnologia Bluetooth Low Energy (iBeacon i GATT) i audio d alta definicio, permet a l alumnat amb discapacitat visual identificar la ubicacio exacta de les aules a traves del seu telefon mobil o mitjancant balises acustiques d orientacio activades sota demanda.

En centralitzar la gestio, els horaris i els registres a Firebase Realtime Database, el dispositiu ja no requereix un servidor web local monolititc, permetent un suport dual: desplegament optimitzat en ESP32-S3 i compatibilitat directa amb plaques Arduino UNO R4 WiFi.

### 2. Millores i Capacitats Principals
* Suport Dual de Plataformes: El mateix codi font pot compilar-se tant en ESP32-S3 com en Arduino UNO R4 WiFi.
* Sincronitzacio en Temps Real amb Firebase: Les aules, horaris, volum i permisos es configuren centralitzadament al nuvol i s apliquen en calent.
* Cua d Esdeveniments Fora de Linia (Buffer Offline): Si el Wi-Fi cau, els accessos es guarden en un buffer local i es transmeten a Firebase automaticament en recuperar la connexio.
* Despatxador Remot d Ordres: Possibilitat d activar el so de l aula o ordenar un tancament d emergencia des de Firebase.
* Polsos d Ecolocalitzacio Acustica: Sons armonics suaus optimitzats per a orientacio auditiva tridimensional.
* Horari Intelligent (07:00 a 20:30): Wi-Fi actiu en horari lectiu i desconnexio total nocturna per a preservacio de bateria.
* Watchdog de Maquinari: Recuperacio automatica davant fallades de xarxa.

---

## License / Licencia / Llicencia
Copyright (c) 2026 EloyGM. All rights reserved / Todos los derechos reservados / Tots els drets reservats.
Consult LICENSE for details / Consultar LICENSE para mas detalles / Consultar LICENSE per a mes detalls.
