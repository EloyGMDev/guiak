# Guiak (guiak.com)

Sistema de balizas inteligentes y navegacion asistida en interiores para personas con discapacidad visual en centros educativos.

---

## Castellano

### 1. Descripcion General
Guiak es un dispositivo embebido disenado para instalarse en los marcos de las puertas de aulas e instalaciones educativas. Mediante tecnologia Bluetooth Low Energy (iBeacon y GATT) y audio digital I2S de alta definicion, permite que los alumnos con discapacidad visual identifiquen la ubicacion exacta de las aulas a traves de su telefono movil o mediante balizas acusticas de ecolocalizacion activadas bajo demanda.

El sistema esta optimizado para operar mediante bateria de litio recargable, apagando la radio Wi-Fi durante la noche y sincronizandose en tiempo real con una base de datos centralizada en Firebase.

### 2. Especificaciones de Hardware (ESP32-S3)
* Microcontrolador: ESP32-S3 DevKit-C (Dual-Core 240 MHz, 16 MB Flash, 8 MB PSRAM).
* Audio Digital: Amplificador I2S MAX98357A (3W mono clase D) con pin de corte de consumo (SD_MODE a 0 uA en reposo).
* Conectividad: Bluetooth 5.0 LE (iBeacon con potencia calibrada a 1 metro) y Wi-Fi 802.11 b/g/n.
* Sensor de Proximidad / Registro: Lector RFID RC522 por bus SPI.
* Alimentacion: Bateria Li-Ion 3.7V (18650 o LiPo) con lectura de tension por divisor resistivo en ADC.
* Autonomia: Modo ULP con Modem Sleep diurno y apagado total nocturno.

### 3. Mapa de Conexion de Pines
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
* Boton de Configuracion: GPIO 0 (o GPIO 7 con pull-up)
* LED de Estado: GPIO 48

### 4. Sincronizacion con Firebase
* Actualizacion Dinamica: Los nodos consultan periodicamente la coleccion de configuracion en Firebase. Si se modifica el nombre del aula, la planta, el volumen o los parametros de acceso en la nube, el nodo adapta su configuracion local en memoria Flash (NVS) de forma inmediata.
* Telemetria y Registro de Logs: Cada evento critico (fichaje RFID, disparo de sonido de baliza, alerta de bateria baja y arranque) se transmite a la ruta de auditoria de Firebase con su correspondiente marca de tiempo.

### 5. Gestion Energetica y Horario Wi-Fi
* Horario Escolar (07:00 a 20:30): La radio Wi-Fi permanece activa con protocolo Wi-Fi Modem Sleep, permitiendo comunicacion con Firebase y acceso al servidor web local con un consumo medio de 15 a 20 mA.
* Periodo Nocturno (20:30 a 07:00): La radio Wi-Fi se desconecta por completo, reduciendo el consumo del dispositivo a menos de 1 mA para maximizar la duracion de la bateria.
* Sobrescritura Manual: La pulsacion del boton fisico durante 2.5 segundos permite encender el Wi-Fi temporalmente durante 5 minutos para tareas de mantenimiento.

---

## Catala

### 1. Descripcio General
Guiak es un dispositiu encastat dissenyat per a ser installat als marcs de les portes d aules i installacions educatives. Mitjancant tecnologia Bluetooth Low Energy (iBeacon i GATT) i audio digital I2S d alta definicio, permet que l alumnat amb discapacitat visual identifiqui la ubicacio exacta de les aules a traves del seu telefon mobil o mitjancant balises acustiques d ecolocalitzacio activades sota demanda.

El sistema esta optimitzat per a funcionar amb bateria de liti recarregable, apagant la radio Wi-Fi durant la nit i sincronitzant-se en temps real amb una base de dades centralitzada a Firebase.

### 2. Especificacions de Maquinari (ESP32-S3)
* Microcontrolador: ESP32-S3 DevKit-C (Dual-Core 240 MHz, 16 MB Flash, 8 MB PSRAM).
* Audio Digital: Amplificador I2S MAX98357A (3W mono classe D) amb pin de tall de consum (SD_MODE a 0 uA en repos).
* Connectivitat: Bluetooth 5.0 LE (iBeacon amb potencia calibrada a 1 metre) i Wi-Fi 802.11 b/g/n.
* Sensor de Proximitat / Registre: Lector RFID RC522 per bus SPI.
* Alimentacio: Bateria Li-Ion 3.7V (18650 o LiPo) amb lectura de tensio per divisor resistiu a l ADC.
* Autonomia: Mode ULP amb Modem Sleep diurn i apagada total nocturna.

### 3. Mapa de Connexio de Pins
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
* Boto de Configuracio: GPIO 0 (o GPIO 7 amb pull-up)
* LED d Estat: GPIO 48

### 4. Sincronitzacio amb Firebase
* Actualitzacio Dinamica: Els nodes consulten periodicament la configuracio a Firebase. Si es modifica el nom de l aula, la planta, el volum o els parametres d acces al nuvol, el node adapta la seva configuracio local a la memoria Flash (NVS) d immediat.
* Telemetria i Registre de Logs: Cada esdeveniment critic (lectura RFID, activacio de so de balisa, alerta de bateria baixa i arrencada) es transmet a Firebase amb la seva marca de temps.

### 5. Gestio Energetica i Horari Wi-Fi
* Horari Escolar (07:00 a 20:30): La radio Wi-Fi roman activa amb protocol Wi-Fi Modem Sleep, permetent la comunicacio amb Firebase i l acces al servidor web local amb un consum mitja de 15 a 20 mA.
* Periode Nocturn (20:30 a 07:00): La radio Wi-Fi es desconnecta completament, reduint el consum del dispositiu a menys d 1 mA per maximitzar la durada de la bateria.
* Sobreescriptura Manual: La pulsacio del boto fisic durant 2.5 segons permet encendre el Wi-Fi temporalment durant 5 minuts per a tasques de manteniment.

---

## Llicencia / Licencia
Copyright (c) 2026 EloyGM. Tots els drets reservats / Todos los derechos reservados.
Consultar l arxiu LICENSE per a mes detalls / Consultar el archivo LICENSE para mas detalles.
