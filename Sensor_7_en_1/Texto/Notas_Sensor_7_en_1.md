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

| JXBS-3001-RS485-Soil 7 in 1| Módulo XY-485|
|:-------------------------------------------:|:-------------:|
| Amarillo                                    | A+            |
| Azul                                        | B-            |

## Módulo convertidor XY-485 a Arduino UNO

| Módulo XY-485   | Arduino UNO |
|:---------------:|:-----------:|
| VCC             | 5 V         |
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

## Verificar frame Modbus

```c
#include <SoftwareSerial.h> //Uso de la libreria SoftwareSerial
#include <ModbusMaster.h>

SoftwareSerial RS485(/*Rx*/ 10, /*Tx*/ 11);
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

# ESP32

+ No se usa `SoftwareSerial`, debido a que es limitado y la ESP32 cuenta con múltiples puertos UART hardware.
+ Si se necesita usar pines diferentes, se puede configurar dinámicamente al inicializar Serial2.

## Conexiones

### Sensor 7 en 1 con el módulo convertidor y ESP32

| Sensor 7 en 1   | ESP32       | Módulo XY-485 |
|:---------------:|:-----------:|:-------------:|
| Cable negro     | VIN / 3.3   |               |
| Cable márron    | GND         |               |
| Cable amarillo  |             | A+            |
| Cable azul      |             | B-            |

### Módulo convertidor y ESP32

| Módulo XY-485   | ESP32       |
|:---------------:|:-----------:|
| VCC             | VIN / 3.3   |
| GND             | GND         |
| TXD             | GPIO16/26   |
| RXD             | GPIO17/25   |

## Primer código

```C++  
#include <ModbusMaster.h>

#define RXD2 16
#define TXD2 17
#define RE_DE 4

HardwareSerial RS485(2);
ModbusMaster node;

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
  RS485.begin(9600, SERIAL_8N1, RXD2, TXD2);

  node.begin(1, RS485);
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);

  Serial.println("Leyendo Temperatura");
}

void loop() {
  uint8_t result = node.readHoldingRegisters(0x0013, 1);
  if (result == node.ku8MBSuccess) {
    float temperature = (int16_t)node.getResponseBuffer(0) * 0.1;
    Serial.print("Temperatura (°C): ");
    Serial.println(temperature);
  } else {
    Serial.print("Error: ");
    Serial.println(result);
  }
  delay(2000);
}
```

### Segundo código. Sin pines de control

```c++
#include <ModbusMaster.h>

// Configuración de pines UART para la ESP32
#define RS485_TX 17 // GPIO17 (TX)
#define RS485_RX 16 // GPIO16 (RX)

// Declaración del objeto Modbus
ModbusMaster node;

void setup() {
  // Configuración de comunicación serial
  Serial.begin(9600);          // Monitor serial
  Serial2.begin(9600, SERIAL_8N1, RS485_RX, RS485_TX); // UART hardware para RS485

  // Configuración del objeto Modbus
  node.begin(1, Serial2); // Dirección del sensor = 1, usando Serial2

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