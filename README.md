# Guiak (guiak.com)

Assistive indoor beacon and navigation ecosystem for visually impaired orientation in educational institutions.
Sistema de balizas inteligentes y navegacion asistida en interiores para personas con discapacidad visual en centros educativos.
Sistema de balises intelligets i navegacio assistida en interiors per a persones amb discapacitat visual en centres educatius.

---

## English

### 1. Overview
Guiak is an embedded IoT system designed for classroom door frames and educational facilities. Using Bluetooth Low Energy (iBeacon and GATT) combined with high-definition digital audio, it allows visually impaired students to identify classroom entrances via their smartphone or through on-demand acoustic orientation pulses.

By offloading device management, schedules, and logging to Firebase Realtime Database, the hardware no longer requires a monolithic on-board web server, enabling robust deployment on ESP32-S3 as well as compatibility with Arduino UNO R4 WiFi platforms.

### 2. Hardware Architecture & Platform Support
* ESP32-S3 DevKit-C: Dual-Core 240 MHz, 16 MB Flash, 8 MB PSRAM. Ultra-low-power battery operation with I2S digital audio (MAX98357A, 0 uA sleep).
* Arduino UNO R4 WiFi: Renesas RA4M1 48 MHz + ESP32-S3 bridge. Supported for lab evaluation, mains/power bank operation, and DAC/tone acoustic output.
* Radio Connectivity: Bluetooth 5.0 LE (iBeacon with 1-meter calibrated TX power) and Wi-Fi 802.11 b/g/n.
* Access & Identification: MFRC522 RFID reader via SPI.
* Power Management: Battery monitoring via ADC resistive divider. Active Wi-Fi Modem Sleep during school hours and radio shutdown at night.

### 3. Pin Connection Mapping (ESP32-S3)
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

### 4. Firebase Cloud Synchronization
* Dynamic Remote Updates: Nodes periodically poll Firebase. When administrators update classroom names, floors, or volume levels in the cloud, nodes automatically adapt their local configuration and persist changes to non-volatile storage (NVS).
* Real-Time Telemetry & Audit Logs: Critical events (RFID scans, acoustic beacon triggers, low-battery alerts, and boot events) are streamed to Firebase audit collections with timestamps and node metrics.

### 5. Energy Management & Wi-Fi Operating Hours
* Operational Hours (07:00 to 20:30): Wi-Fi remains active using Wi-Fi Modem Sleep (~15-20 mA average consumption), handling cloud sync and status reporting.
* Night Standby (20:30 to 07:00): Wi-Fi radio powers down completely (< 1 mA consumption) to maximize battery longevity.
* Manual Override: Holding the physical button for 2.5 seconds temporarily enables Wi-Fi for 5 minutes during maintenance windows.

---

## Castellano

### 1. Descripcion General
Guiak es un sistema embebido disenado para instalarse en los marcos de las puertas de aulas e instalaciones educativas. Mediante tecnologia Bluetooth Low Energy (iBeacon y GATT) y audio digital de alta definicion, permite a los alumnos con discapacidad visual identificar la ubicacion exacta de las aulas a traves de su telefono movil o mediante balizas acusticas de orientacion activadas bajo demanda.

Al centralizar la gestion, los horarios y los registros en Firebase Realtime Database, el dispositivo ya no requiere un servidor web local monolitico, permitiendo un despliegue optimizado en ESP32-S3 y la reutilizacion viable de placas Arduino UNO R4 WiFi.

### 2. Arquitectura de Hardware y Soporte de Plataformas
* ESP32-S3 DevKit-C: Doble nucleo a 240 MHz, 16 MB Flash, 8 MB PSRAM. Maxima eficiencia a bateria con audio I2S digital (MAX98357A, 0 uA en reposo).
* Arduino UNO R4 WiFi: Renesas RA4M1 48 MHz + puente ESP32-S3. Compatible para prototipado y alimentacion por red o bateria externa con salida DAC/buzzer.
* Conectividad: Bluetooth 5.0 LE (iBeacon con potencia calibrada a 1 metro) y Wi-Fi 802.11 b/g/n.
* Sensor de Proximidad / Registro: Lector RFID RC522 por bus SPI.
* Gestion Energetica: Lectura de tension de bateria por ADC con divisor resistivo. Wi-Fi Modem Sleep diurno y apagado total nocturno.

### 3. Mapa de Conexion de Pines (ESP32-S3)
* I2S BCLK (Reloj de bits): GPIO 15
* I2S LRC (Seleccion de canal): GPIO 16
* I2S DIN (Datos de audio): GPIO 17
* I2S SD (Corte de alimentacion de audio): GPIO 18
* RFID SS / SDA: GPIO 10
* RFID RST: GPIO 9
* RFID SCK: GPIO 12
* RFID MISO: GPIO 13
* RFID MOSI: GPIO 11
* Divisor de Bateria (ADC1_CH3): GPIO 4
* Boton de Configuracion: GPIO 0 (o GPIO 7)
* LED de Estado: GPIO 48

### 4. Sincronizacion con Firebase
* Actualizacion Dinamica: Los nodos consultan periodicamente la coleccion de configuracion en Firebase. Si se modifica el nombre del aula, la planta o el volumen en la nube, el nodo adapta su configuracion local en memoria Flash (NVS) de forma inmediata.
* Telemetria y Registro de Logs: Cada evento critico (fichaje RFID, disparo acustico, alerta de bateria y arranque) se transmite a Firebase con su correspondiente marca de tiempo.

### 5. Gestion Energetica y Horario Wi-Fi
* Horario Escolar (07:00 a 20:30): La radio Wi-Fi permanece activa con Wi-Fi Modem Sleep (15 a 20 mA de media), gestionando la sincronizacion con la nube.
* Periodo Nocturno (20:30 a 07:00): La radio Wi-Fi se desconecta por completo (< 1 mA) para preservar la bateria.
* Sobrescritura Manual: Mantener presionado el boton fisico durante 2.5 segundos activa el Wi-Fi temporalmente durante 5 minutos para mantenimiento.

---

## Catala

### 1. Descripcio General
Guiak es un sistema encastat dissenyat per a ser installat als marcs de les portes d aules i installacions educatives. Mitjancant tecnologia Bluetooth Low Energy (iBeacon i GATT) i audio digital d alta definicio, permet a l alumnat amb discapacitat visual identificar la ubicacio exacta de les aules a traves del seu telefon mobil o mitjancant balises acustiques d orientacio activades sota demanda.

En centralitzar la gestio, els horaris i els registres a Firebase Realtime Database, el dispositiu ja no requereix un servidor web local monolititc, permetent un desplegament optimitzat en ESP32-S3 i la reutilitzacio viable de plaques Arduino UNO R4 WiFi.

### 2. Arquitectura de Maquinari i Suport de Plataformes
* ESP32-S3 DevKit-C: Doble nucli a 240 MHz, 16 MB Flash, 8 MB PSRAM. Maxima eficiencia a bateria amb audio I2S digital (MAX98357A, 0 uA en repos).
* Arduino UNO R4 WiFi: Renesas RA4M1 48 MHz + pont ESP32-S3. Compatible per a prototipatge i alimentacio per xarxa o bateria externa amb sortida DAC/buzzer.
* Connectivitat: Bluetooth 5.0 LE (iBeacon amb potencia calibrada a 1 metre) i Wi-Fi 802.11 b/g/n.
* Sensor de Proximitat / Registre: Lector RFID RC522 per bus SPI.
* Gestio Energetica: Lectura de tensio de bateria per ADC amb divisor resistiu. Wi-Fi Modem Sleep diurn i apagada total nocturna.

### 3. Mapa de Connexio de Pins (ESP32-S3)
* I2S BCLK (Rellotge de bits): GPIO 15
* I2S LRC (Seleccio de canal): GPIO 16
* I2S DIN (Dades d audio): GPIO 17
* I2S SD (Tall d alimentacio d audio): GPIO 18
* RFID SS / SDA: GPIO 10
* RFID RST: GPIO 9
* RFID SCK: GPIO 12
* RFID MISO: GPIO 13
* RFID MOSI: GPIO 11
* Divisor de Bateria (ADC1_CH3): GPIO 4
* Boto de Configuracio: GPIO 0 (o GPIO 7)
* LED d Estat: GPIO 48

### 4. Sincronitzacio amb Firebase
* Actualitzacio Dinamica: Els nodes consulten periodicament la configuracio a Firebase. Si es modifica el nom de l aula, la planta o el volum al nuvol, el node adapta la seva configuracio local a la memoria Flash (NVS) d immediat.
* Telemetria i Registre de Logs: Cada esdeveniment critic (lectura RFID, activacio acustica, alerta de bateria i arrencada) es transmet a Firebase amb la seva marca de temps.

### 5. Gestio Energetica i Horari Wi-Fi
* Horari Escolar (07:00 a 20:30): La radio Wi-Fi roman activa amb Wi-Fi Modem Sleep (15 a 20 mA de mitjana), gestionant la sincronitzacio amb el nuvol.
* Periode Nocturn (20:30 a 07:00): La radio Wi-Fi es desconnecta completament (< 1 mA) per preservar la bateria.
* Sobreescriptura Manual: Mantenir premut el boto fisic durant 2.5 segons activa el Wi-Fi temporalment durant 5 minuts per a manteniment.

---

## License / Licencia / Llicencia
Copyright (c) 2026 EloyGM. All rights reserved / Todos los derechos reservados / Tots els drets reservats.
Consult LICENSE for details / Consultar LICENSE para mas detalles / Consultar LICENSE per a mes detalls.
