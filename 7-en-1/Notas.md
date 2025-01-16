# Sensor

Fabricante: JXCT
Sensor JXBS-3001-RS485-Soil 7 en 1

# Librerías

* SoftwareSerial
* ModbusMaster by Doc Walker desde la versión 2.0.1

# Requerimientos

* Convertidor RS485 a TTL (UART)
* Fuente de alimentación de 12 a 24 VDC

# Conexión 

## Sensor JXBS-3001-RS485-Soil 7 in 1

| Función       | Color de cable | Especificaciones     |
|:-------------:|:--------------:|:--------------------:|
| Energía       | Marrón         |Alimentación positiva |
| Energía       | Negro          |Alimentación negativa |
| Comunicación  | Amarillo/Gris  |485A                  |
| Comunicación  | Azul           |485B                  |

## Módulo convertidor XY-485

| Cable del sensor JXBS-3001-RS485-Soil 7 in 1| Módulo XY-485|
|:-------------------------------------------:|:-------------:|
| Amarillo                                    | A+            |
| Azul                                        | B-            |

## Módulo convertidor XY-485 a Arduino UNO

| Módulo          | Arduino UNO |
|:---------------:|:-----------:|
| 3.3 V - 33 V    | 5 V         |
| GND             | GND         |
| TX              | 0/2/10      |
| RX              | 1/3/11      |

# Configuraciones del sensor
* Baud rate: 2400/4800/9600 (por defecto)
* Parámetro: 8 data bits, 1 stop bit, sin paridad
* Función de lectura: `0x03` para leer regristros
* Los valores leídos se convierten según la documentación:
    * Humedad: `(valor*0.1)`
    * Temperatura: `(int16_t)valor * 0.1 °c` para manejar negativos
    * pH : `valor*0.01`
* la dirección del sensor sea la correcta (por defecto `0x01`).

## Dirección de las variables
* Conductividad eléctrica `0x0015`.
* pH `0x0006`.
* Nitrógeno, fósforo y potasio `0x001E, 0x001F, 0x0020`.

# Códigos para el IDE de Arduino

## Verificar frame Modbus

```c
#include <SoftwareSerial.h>
#include <ModbusMaster.h>

SoftwareSerial RS485(10, 11); // RX, TX
ModbusMaster node;
const int RE_DE = 8;

void preTransmission() {
  digitalWrite(RE_DE, HIGH);
}

void postTransmission() {
  digitalWrite(RE_DE, LOW);
}

void setup() {
  pinMode(RE_DE, OUTPUT);
  digitalWrite(RE_DE, LOW);
  Serial.begin(9600);
  RS485.begin(9600);

  node.begin(1, RS485); // Dirección del sensor
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);

  Serial.println("Leyendo Temperatura");
}

void loop() {
  uint8_t result;
  uint16_t data;

  // Leer temperatura (registro 0x0013)
  result = node.readHoldingRegisters(0x0013, 1);
  if (result == node.ku8MBSuccess) {
    data = node.getResponseBuffer(0);
    float temperature = (int16_t)data * 0.1;
    Serial.print("Temperatura (°C): ");
    Serial.println(temperature);
  } else {
    Serial.print("Error al leer temperatura: ");
    Serial.println(result);
  }

  delay(2000);
}
```

## Con puerto serial secundario

```c
#include <ModbusMaster.h>

// Declaración del objeto Modbus
ModbusMaster node;

const int RE_DE = 8; // Pin de control RS485 (HIGH = Transmitir, LOW = Recibir)

// Configura el puerto serial para RS485
void preTransmission() {
  digitalWrite(RE_DE, HIGH);
}

void postTransmission() {
  digitalWrite(RE_DE, LOW);
}

void setup() {
  // Configuración de pines
  pinMode(RE_DE, OUTPUT);
  digitalWrite(RE_DE, LOW); // Modo recepción

  // Configuración de comunicación serial
  Serial.begin(9600); // Monitor serial
  Serial1.begin(9600); // Comunicación UART con el RS485 (TX = 10, RX = 11)

  // Configuración del objeto Modbus
  node.begin(1, Serial1); // Dirección del sensor = 1, puerto Serial1
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);

  Serial.println("Lectura de las 7 variables del sensor JXBS-3001-RS485-Soil 7 en 1");
}

void loop() {
  uint8_t result;
  uint16_t data;

  // 1. Humedad del suelo (registro 0x0012)
  result = node.readHoldingRegisters(0x0012, 1);
  if (result == node.ku8MBSuccess) {
    data = node.getResponseBuffer(0);
    Serial.print("Humedad (%): ");
    Serial.println(data * 0.1); // Conversión según documentación
  } else {
    Serial.println("Error al leer humedad.");
  }

  // 2. Temperatura del suelo (registro 0x0013)
  result = node.readHoldingRegisters(0x0013, 1);
  if (result == node.ku8MBSuccess) {
    data = node.getResponseBuffer(0);
    float temperature = (int16_t)data * 0.1; // Manejar valores negativos
    Serial.print("Temperatura (°C): ");
    Serial.println(temperature);
  } else {
    Serial.println("Error al leer temperatura.");
  }

  // 3. Conductividad eléctrica (registro 0x0015)
  result = node.readHoldingRegisters(0x0015, 1);
  if (result == node.ku8MBSuccess) {
    data = node.getResponseBuffer(0);
    Serial.print("Conductividad (uS/cm): ");
    Serial.println(data);
  } else {
    Serial.println("Error al leer conductividad.");
  }

  // 4. pH del suelo (registro 0x0006)
  result = node.readHoldingRegisters(0x0006, 1);
  if (result == node.ku8MBSuccess) {
    data = node.getResponseBuffer(0);
    Serial.print("pH: ");
    Serial.println(data * 0.01); // Conversión según documentación
  } else {
    Serial.println("Error al leer pH.");
  }

  // 5. Nitrógeno (registro 0x001E)
  result = node.readHoldingRegisters(0x001E, 1);
  if (result == node.ku8MBSuccess) {
    data = node.getResponseBuffer(0);
    Serial.print("Nitrógeno (mg/kg): ");
    Serial.println(data);
  } else {
    Serial.println("Error al leer nitrógeno.");
  }

  // 6. Fósforo (registro 0x001F)
  result = node.readHoldingRegisters(0x001F, 1);
  if (result == node.ku8MBSuccess) {
    data = node.getResponseBuffer(0);
    Serial.print("Fósforo (mg/kg): ");
    Serial.println(data);
  } else {
    Serial.println("Error al leer fósforo.");
  }

  // 7. Potasio (registro 0x0020)
  result = node.readHoldingRegisters(0x0020, 1);
  if (result == node.ku8MBSuccess) {
    data = node.getResponseBuffer(0);
    Serial.print("Potasio (mg/kg): ");
    Serial.println(data);
  } else {
    Serial.println("Error al leer potasio.");
  }

  Serial.println("----------------------------");
  delay(2000); // Esperar 2 segundos antes de la próxima lectura
}
```

## V1.0 sin puerto serial secundario y con modo tranmisión y recepción

```c
#include <SoftwareSerial.h>
#include <ModbusMaster.h>

// Configuración de SoftwareSerial
SoftwareSerial RS485(10, 11); // RX, TX (pines digitales para comunicación RS485)

// Declaración del objeto Modbus
ModbusMaster node;

const int RE_DE = 8; // Pin de control RS485 (HIGH = Transmitir, LOW = Recibir)

// Configura el puerto serial para RS485
void preTransmission() {
  digitalWrite(RE_DE, HIGH);
}

void postTransmission() {
  digitalWrite(RE_DE, LOW);
}

void setup() {
  // Configuración de pines
  pinMode(RE_DE, OUTPUT);
  digitalWrite(RE_DE, LOW); // Modo recepción

  // Configuración de comunicación serial
  Serial.begin(9600); // Monitor serial
  RS485.begin(9600);  // Puerto RS485 con SoftwareSerial

  // Configuración del objeto Modbus
  node.begin(1, RS485); // Dirección del sensor = 1, puerto RS485 virtual
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);

  Serial.println("Lectura de las 7 variables del sensor JXBS-3001-RS485-Soil 7 en 1");
}

void loop() {
  uint8_t result;
  uint16_t data;

  // 1. Humedad del suelo (registro 0x0012)
  result = node.readHoldingRegisters(0x0012, 1);
  if (result == node.ku8MBSuccess) {
    data = node.getResponseBuffer(0);
    Serial.print("Humedad (%): ");
    Serial.println(data * 0.1); // Conversión según documentación
  } else {
    Serial.println("Error al leer humedad.");
  }

  // 2. Temperatura del suelo (registro 0x0013)
  result = node.readHoldingRegisters(0x0013, 1);
  if (result == node.ku8MBSuccess) {
    data = node.getResponseBuffer(0);
    float temperature = (int16_t)data * 0.1; // Manejar valores negativos
    Serial.print("Temperatura (°C): ");
    Serial.println(temperature);
  } else {
    Serial.println("Error al leer temperatura.");
  }

  // 3. Conductividad eléctrica (registro 0x0015)
  result = node.readHoldingRegisters(0x0015, 1);
  if (result == node.ku8MBSuccess) {
    data = node.getResponseBuffer(0);
    Serial.print("Conductividad (uS/cm): ");
    Serial.println(data);
  } else {
    Serial.println("Error al leer conductividad.");
  }

  // 4. pH del suelo (registro 0x0006)
  result = node.readHoldingRegisters(0x0006, 1);
  if (result == node.ku8MBSuccess) {
    data = node.getResponseBuffer(0);
    Serial.print("pH: ");
    Serial.println(data * 0.01); // Conversión según documentación
  } else {
    Serial.println("Error al leer pH.");
  }

  // 5. Nitrógeno (registro 0x001E)
  result = node.readHoldingRegisters(0x001E, 1);
  if (result == node.ku8MBSuccess) {
    data = node.getResponseBuffer(0);
    Serial.print("Nitrógeno (mg/kg): ");
    Serial.println(data);
  } else {
    Serial.println("Error al leer nitrógeno.");
  }

  // 6. Fósforo (registro 0x001F)
  result = node.readHoldingRegisters(0x001F, 1);
  if (result == node.ku8MBSuccess) {
    data = node.getResponseBuffer(0);
    Serial.print("Fósforo (mg/kg): ");
    Serial.println(data);
  } else {
    Serial.println("Error al leer fósforo.");
  }

  // 7. Potasio (registro 0x0020)
  result = node.readHoldingRegisters(0x0020, 1);
  if (result == node.ku8MBSuccess) {
    data = node.getResponseBuffer(0);
    Serial.print("Potasio (mg/kg): ");
    Serial.println(data);
  } else {
    Serial.println("Error al leer potasio.");
  }

  Serial.println("----------------------------");
  delay(2000); // Esperar 2 segundos antes de la próxima lectura
}
```

## V2.0 modo recepción

```c
#include <SoftwareSerial.h>
#include <ModbusMaster.h>

// Configuración de SoftwareSerial
SoftwareSerial RS485(10, 11); // RX, TX (pines digitales para comunicación RS485)

// Declaración del objeto Modbus
ModbusMaster node;

void setup() {
  // Configuración de comunicación serial
  Serial.begin(9600); // Monitor serial
  RS485.begin(9600);  // Puerto RS485 con SoftwareSerial

  // Configuración del objeto Modbus
  node.begin(1, RS485); // Dirección del sensor = 1, puerto RS485 virtual

  Serial.println("Lectura de las 7 variables del sensor JXBS-3001-RS485-Soil 7 en 1");
}

void loop() {
  uint8_t result;
  uint16_t data;

  // 1. Humedad del suelo (registro 0x0012)
  result = node.readHoldingRegisters(0x0012, 1);
  if (result == node.ku8MBSuccess) {
    data = node.getResponseBuffer(0);
    Serial.print("Humedad (%): ");
    Serial.println(data * 0.1); // Conversión según documentación
  } else {
    Serial.println("Error al leer humedad.");
  }

  // 2. Temperatura del suelo (registro 0x0013)
  result = node.readHoldingRegisters(0x0013, 1);
  if (result == node.ku8MBSuccess) {
    data = node.getResponseBuffer(0);
    float temperature = (int16_t)data * 0.1; // Manejar valores negativos
    Serial.print("Temperatura (°C): ");
    Serial.println(temperature);
  } else {
    Serial.println("Error al leer temperatura.");
  }

  // 3. Conductividad eléctrica (registro 0x0015)
  result = node.readHoldingRegisters(0x0015, 1);
  if (result == node.ku8MBSuccess) {
    data = node.getResponseBuffer(0);
    Serial.print("Conductividad (uS/cm): ");
    Serial.println(data);
  } else {
    Serial.println("Error al leer conductividad.");
  }

  // 4. pH del suelo (registro 0x0006)
  result = node.readHoldingRegisters(0x0006, 1);
  if (result == node.ku8MBSuccess) {
    data = node.getResponseBuffer(0);
    Serial.print("pH: ");
    Serial.println(data * 0.01); // Conversión según documentación
  } else {
    Serial.println("Error al leer pH.");
  }

  // 5. Nitrógeno (registro 0x001E)
  result = node.readHoldingRegisters(0x001E, 1);
  if (result == node.ku8MBSuccess) {
    data = node.getResponseBuffer(0);
    Serial.print("Nitrógeno (mg/kg): ");
    Serial.println(data);
  } else {
    Serial.println("Error al leer nitrógeno.");
  }

  // 6. Fósforo (registro 0x001F)
  result = node.readHoldingRegisters(0x001F, 1);
  if (result == node.ku8MBSuccess) {
    data = node.getResponseBuffer(0);
    Serial.print("Fósforo (mg/kg): ");
    Serial.println(data);
  } else {
    Serial.println("Error al leer fósforo.");
  }

  // 7. Potasio (registro 0x0020)
  result = node.readHoldingRegisters(0x0020, 1);
  if (result == node.ku8MBSuccess) {
    data = node.getResponseBuffer(0);
    Serial.print("Potasio (mg/kg): ");
    Serial.println(data);
  } else {
    Serial.println("Error al leer potasio.");
  }

  Serial.println("----------------------------");
  delay(2000); // Esperar 2 segundos antes de la próxima lectura
}
```
