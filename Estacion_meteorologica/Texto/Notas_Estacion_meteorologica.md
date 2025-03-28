# Estación metereológica

**Desconectar** los cables TX y RX porque interferirá con la carga del código.
**Primero cargar** el código y luego conectar los cables Tx y Rx

Si se carga el código base se debe color el puente (jumper cap) en *J1* y si se carga un código externo se anexa el puente en *J2*.

# Fuente

Wiki de la [estación meteorológica](https://wiki.dfrobot.com/APRS_Weather_Station_Sensor_Kit_SEN0186)

# Conexiones ESP32

| ESP32 | Estación meteorológica |
|:-------------:|:--------------:|
| GND   | GND   |
| 3.3 V | 5 V   |
| RXD   | TX    |
| TXD   | RX    |