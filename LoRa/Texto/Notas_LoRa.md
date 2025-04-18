# Módulo Ra-02

LoRa Ra-02 (433 MHz)

## Conexiones módulo

| Ra-02 | Arduino UNO |
|:-------------:|:--------------:|
| GND   | GND   |
| 3.3 V | 3.3 V |
| RST   | D9    |
| NSS   | D10   |
| MOSI  | D11   |
| MISO  | D12   |
| SCK   | D13   |
| DIO0  | D2    |

## Códigos

### Verificar módulo

Inspecciona y verifica los registros internos del módulo LoRa para depuración o ajuste avanzado de configuraciones. 
Es útil en las primeras etapas de desarrollo para **asegurarte de que el módulo LoRa está correctamente conectado y configurado**. También puede ser utilizado cuando se necesita ajustar parámetros específicos del módulo o depurar problemas en la comunicación.

```c
/*
  LoRa register dump

  This examples shows how to inspect and output the LoRa radio's
  registers on the Serial interface
*/
#include <SPI.h>              // include libraries
#include <LoRa.h>

void setup() {
  Serial.begin(9600);               // initialize serial
  while (!Serial);

  Serial.println("LoRa Dump Registers");

  // override the default CS, reset, and IRQ pins (optional)
  // LoRa.setPins(7, 6, 1); // set CS, reset, IRQ pin

  if (!LoRa.begin(433E6)) {         // initialize ratio at 915 MHz
    Serial.println("LoRa init failed. Check your connections.");
    while (true);                   // if failed, do nothing
  }

  LoRa.dumpRegisters(Serial);
}


void loop() {
}
```

*Fuente: Obtenido de la librería LoRa en el IDE de Arduino*

## Transmisión

### Primera versión

1. Sistema de direccionamiento:

+ Gateway: 0x01
+ Nodo 1: 0x02
+ Nodo 2: 0x03

2. Gestión de colisiones:

+ Intervalos aleatorios entre envíos
+ Sistema de ACK para confirmar recepción
+ Timeout para detectar fallos de comunicación

#### Gateway

```C++
#include <SPI.h>
#include <LoRa.h>

// Definición de pines
#define LORA_RST 14
#define LORA_SS 5
#define LORA_DIO0 26

// Direcciones de los dispositivos
const byte GATEWAY_ADDRESS = 0x01;
const byte NODE1_ADDRESS = 0x02;
const byte NODE2_ADDRESS = 0x03;

// Variables para el manejo de mensajes
String receivedData = "";
unsigned long lastNodeCheck = 0;
const unsigned long NODE_TIMEOUT = 30000; // 30 segundos timeout
bool node1Active = false;
bool node2Active = false;

// Variables para el registro de última actividad
unsigned long lastNode1Time = 0;
unsigned long lastNode2Time = 0;

void setup() {
  Serial.begin(9600);
  while (!Serial);
  
  Serial.println("LoRa Gateway Iniciando...");
  
  // Configurar pines LoRa
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
  
  // Iniciar LoRa
  if (!LoRa.begin(433E6)) {
    Serial.println("Error iniciando LoRa!");
    while (1);
  }
  
  // Configurar parámetros LoRa
  LoRa.setSpreadingFactor(7);
  LoRa.setSignalBandwidth(125E3);
  LoRa.setCodingRate4(5);
  LoRa.enableCrc();
  
  Serial.println("Gateway LoRa iniciado correctamente!");
  Serial.println("Dirección Gateway: 0x" + String(GATEWAY_ADDRESS, HEX));
}

void loop() {
  // Verificar si hay paquetes disponibles
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    processLoRaPacket(packetSize);
  }
  
  // Verificar estado de los nodos
  checkNodesStatus();
}

void processLoRaPacket(int packetSize) {
  // Leer encabezado del paquete
  byte destination = LoRa.read();
  byte sender = LoRa.read();
  byte msgId = LoRa.read();
  
  // Verificar si el mensaje es para este Gateway
  if (destination != GATEWAY_ADDRESS) {
    return;
  }
  
  // Verificar si el remitente es un nodo válido
  if (sender != NODE1_ADDRESS && sender != NODE2_ADDRESS) {
    return;
  }
  
  // Leer payload
  String payload = "";
  while (LoRa.available()) {
    payload += (char)LoRa.read();
  }
  
  // Actualizar estado del nodo
  if (sender == NODE1_ADDRESS) {
    lastNode1Time = millis();
    node1Active = true;
  } else if (sender == NODE2_ADDRESS) {
    lastNode2Time = millis();
    node2Active = true;
  }
  
  // Imprimir información del paquete
  Serial.println("\n--- Nuevo Paquete ---");
  Serial.println("De: 0x" + String(sender, HEX));
  Serial.println("ID Mensaje: " + String(msgId));
  Serial.println("RSSI: " + String(LoRa.packetRssi()));
  Serial.println("SNR: " + String(LoRa.packetSnr()));
  Serial.println("Payload: " + payload);
  
  // Enviar ACK
  sendAck(sender, msgId);
}

void sendAck(byte nodeAddress, byte msgId) {
  LoRa.beginPacket();
  LoRa.write(nodeAddress);      // Destino
  LoRa.write(GATEWAY_ADDRESS);  // Remitente
  LoRa.write(msgId);           // ID del mensaje original
  LoRa.print("ACK");
  LoRa.endPacket();
}

void checkNodesStatus() {
  unsigned long currentTime = millis();
  
  // Verificar Node 1
  if (node1Active && (currentTime - lastNode1Time > NODE_TIMEOUT)) {
    node1Active = false;
    Serial.println("Nodo 1 inactivo!");
  }
  
  // Verificar Node 2
  if (node2Active && (currentTime - lastNode2Time > NODE_TIMEOUT)) {
    node2Active = false;
    Serial.println("Nodo 2 inactivo!");
  }
}
```

#### Nodo 1

```C++
#include <SPI.h>
#include <LoRa.h>

// Definición de pines
#define LORA_RST 14
#define LORA_SS 5
#define LORA_DIO0 26
#define SENSOR_PIN A0

// Direcciones
const byte NODE_ADDRESS = 0x02;  // Direccion del Nodo 1
const byte GATEWAY_ADDRESS = 0x01;

// Variables para el manejo de mensajes
byte msgCount = 0;
unsigned long lastSendTime = 0;
const unsigned long sendInterval = 10000;  // Intervalo base de envio (10 segundos)
const unsigned long randomInterval = 2000; // Variacion aleatoria maxima

// Variables para el manejo de ACK
bool waitingForAck = false;
unsigned long ackTimeout = 2000;  // 2 segundos de timeout para ACK
unsigned long ackWaitStart = 0;
byte lastMsgId = 0;

void setup() {
  Serial.begin(9600);
  while (!Serial);
  
  Serial.println("LoRa Nodo 1 Iniciando...");
  
  // Configurar pines LoRa
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
  
  // Iniciar LoRa
  if (!LoRa.begin(433E6)) {
    Serial.println("Error iniciando LoRa!");
    while (1);
  }
  
  // Configurar parámetros LoRa
  LoRa.setSpreadingFactor(7);
  LoRa.setSignalBandwidth(125E3);
  LoRa.setCodingRate4(5);
  LoRa.enableCrc();
  
  // Inicializar random seed
  randomSeed(analogRead(A5));
  
  Serial.println("Nodo 1 LoRa iniciado correctamente!");
  Serial.println("Dirección Nodo: 0x" + String(NODE_ADDRESS, HEX));
}

void loop() {
  unsigned long currentTime = millis();
  
  // Verificar si hay ACK pendiente
  if (waitingForAck) {
    checkForAck();
  }
  
  // Enviar nuevo mensaje si es tiempo y no estamos esperando ACK
  if (!waitingForAck && (currentTime - lastSendTime > sendInterval + random(randomInterval))) {
    sendSensorData();
  }
  
  // Otras tareas del nodo pueden ir aquí
}

void sendSensorData() {
  int sensorValue = analogRead(SENSOR_PIN);
  
  // Crear mensaje JSON
  String message = "{\"id\":\"Node1\",\"sensor\":" + String(sensorValue) + "}";
  
  // Enviar paquete
  LoRa.beginPacket();
  LoRa.write(GATEWAY_ADDRESS);  // Destino
  LoRa.write(NODE_ADDRESS);     // Remitente
  LoRa.write(msgCount);         // ID del mensaje
  LoRa.print(message);
  LoRa.endPacket();
  
  Serial.println("Enviando mensaje ID: " + String(msgCount));
  Serial.println("Payload: " + message);
  
  // Configurar espera de ACK
  lastMsgId = msgCount;
  waitingForAck = true;
  ackWaitStart = millis();
  
  msgCount++;
  lastSendTime = millis();
}

void checkForAck() {
  int packetSize = LoRa.parsePacket();
  
  if (packetSize) {
    // Leer encabezado del paquete
    byte destination = LoRa.read();
    byte sender = LoRa.read();
    byte ackMsgId = LoRa.read();
    
    // Verificar si es un ACK válido para nosotros
    if (destination == NODE_ADDRESS && sender == GATEWAY_ADDRESS && ackMsgId == lastMsgId) {
      String ackPayload = "";
      while (LoRa.available()) {
        ackPayload += (char)LoRa.read();
      }
      
      if (ackPayload == "ACK") {
        Serial.println("ACK recibido para mensaje " + String(lastMsgId));
        waitingForAck = false;
        return;
      }
    }
  }
  
  // Verificar timeout de ACK
  if (millis() - ackWaitStart > ackTimeout) {
    Serial.println("Timeout de ACK para mensaje " + String(lastMsgId));
    waitingForAck = false;
    // Aqui podrias implementar reintentos si lo deseas
  }
}
```

#### Nodo 2

```C++
#include <SPI.h>
#include <LoRa.h>

// Definicion de pines
#define LORA_RST 14
#define LORA_SS 5
#define LORA_DIO0 26
#define SENSOR_PIN A0

// Direcciones
const byte NODE_ADDRESS = 0x03;  // Direccion del Nodo 2
const byte GATEWAY_ADDRESS = 0x01;

// Variables para el manejo de mensajes
byte msgCount = 0;
unsigned long lastSendTime = 0;
const unsigned long sendInterval = 10000;  // Intervalo base de envio (10 segundos)
const unsigned long randomInterval = 2000; // Variacion aleatoria maxima

// Variables para el manejo de ACK
bool waitingForAck = false;
unsigned long ackTimeout = 2000;  // 2 segundos de timeout para ACK
unsigned long ackWaitStart = 0;
byte lastMsgId = 0;

void setup() {
  Serial.begin(9600);
  while (!Serial);
  
  Serial.println("LoRa Nodo 2 Iniciando...");
  
  // Configurar pines LoRa
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
  
  // Iniciar LoRa
  if (!LoRa.begin(433E6)) {
    Serial.println("Error iniciando LoRa!");
    while (1);
  }
  
  // Configurar parametros LoRa
  LoRa.setSpreadingFactor(7);
  LoRa.setSignalBandwidth(125E3);
  LoRa.setCodingRate4(5);
  LoRa.enableCrc();
  
  // Inicializar random seed
  randomSeed(analogRead(A5));
  
  Serial.println("Nodo 2 LoRa iniciado correctamente!");
  Serial.println("Dirección Nodo: 0x" + String(NODE_ADDRESS, HEX));
}

void loop() {
  unsigned long currentTime = millis();
  
  // Verificar si hay ACK pendiente
  if (waitingForAck) {
    checkForAck();
  }
  
  // Enviar nuevo mensaje si es tiempo y no estamos esperando ACK
  if (!waitingForAck && (currentTime - lastSendTime > sendInterval + random(randomInterval))) {
    sendSensorData();
  }
  
  // Otras tareas del nodo pueden ir aqui
  // ...
}

void sendSensorData() {
  int sensorValue = analogRead(SENSOR_PIN);
  
  // Crear mensaje JSON
  String message = "{\"id\":\"Node2\",\"sensor\":" + String(sensorValue) + "}";
  
  // Enviar paquete
  LoRa.beginPacket();
  LoRa.write(GATEWAY_ADDRESS);  // Destino
  LoRa.write(NODE_ADDRESS);     // Remitente
  LoRa.write(msgCount);         // ID del mensaje
  LoRa.print(message);
  LoRa.endPacket();
  
  Serial.println("Enviando mensaje ID: " + String(msgCount));
  Serial.println("Payload: " + message);
  
  // Configurar espera de ACK
  lastMsgId = msgCount;
  waitingForAck = true;
  ackWaitStart = millis();
  
  msgCount++;
  lastSendTime = millis();
}

void checkForAck() {
  int packetSize = LoRa.parsePacket();
  
  if (packetSize) {
    // Leer encabezado del paquete
    byte destination = LoRa.read();
    byte sender = LoRa.read();
    byte ackMsgId = LoRa.read();
    
    // Verificar si es un ACK valido para nosotros
    if (destination == NODE_ADDRESS && sender == GATEWAY_ADDRESS && ackMsgId == lastMsgId) {
      String ackPayload = "";
      while (LoRa.available()) {
        ackPayload += (char)LoRa.read();
      }
      
      if (ackPayload == "ACK") {
        Serial.println("ACK recibido para mensaje " + String(lastMsgId));
        waitingForAck = false;
        return;
      }
    }
  }
  
  // Verificar timeout de ACK
  if (millis() - ackWaitStart > ackTimeout) {
    Serial.println("Timeout de ACK para mensaje " + String(lastMsgId));
    waitingForAck = false;
    // Aquí se puede implementar reintentos
  }
}
```


### Segunda versión

#### Gateway

```C++
#include <SPI.h>
#include <LoRa.h>

const int GATEWAY_ADDRESS = 0x01;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial.println("LoRa Gateway");
  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
}

void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // Leer la dirección del destinatario
    int recipient = LoRa.read();
    byte sender = LoRa.read();  // Dirección del remitente
    byte msgID = LoRa.read();  // ID del mensaje
    byte length = LoRa.read();  // Longitud del mensaje

    // Verificar si el encabezado tiene sentido
    if (length > packetSize - 4) {  // Encabezado = 4 bytes
      Serial.println("Error: longitud del mensaje no coincide.");
      while (LoRa.available()) LoRa.read();  // Vaciar el buffer
      return;
    }

    String incoming = "";
    while (LoRa.available()) {
      incoming += (char)LoRa.read();
    }

    if (recipient != GATEWAY_ADDRESS) {
      Serial.println("Mensaje no destinado a este Gateway.");
      return;
    }

    // Mostrar el mensaje recibido
    Serial.println("Mensaje recibido:");
    Serial.println("De: 0x" + String(sender, HEX));
    Serial.println("Mensaje: " + incoming);
    Serial.println("RSSI: " + String(LoRa.packetRssi()));
    Serial.println("SNR: " + String(LoRa.packetSnr()));

    // Enviar ACK al nodo remitente
    LoRa.beginPacket();
    LoRa.write(sender);  // Dirección del nodo
    LoRa.write(GATEWAY_ADDRESS);  // Dirección del Gateway
    LoRa.write(msgID);  // Mismo ID del mensaje
    LoRa.write(2);  // Longitud del ACK ("ACK")
    LoRa.print("ACK");
    LoRa.endPacket();

    Serial.println("ACK enviado al nodo: 0x" + String(sender, HEX));
  }
}

```

#### Nodo

```C++
#include <SPI.h>
#include <LoRa.h>

const int NODE_ADDRESS = 0x02;  // Cambia a 0x03 para Node 2
const int GATEWAY_ADDRESS = 0x01;

byte msgID = 0;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial.println("LoRa Node");
  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
}

void loop() {
  int sensorValue = analogRead(A0);

  // Crear mensaje
  String message = "{\"id\": \"Node" + String(NODE_ADDRESS, HEX) + "\", \"sensor\": " + String(sensorValue) + "}";
  Serial.println("Enviando: " + message);

  // Enviar paquete
  LoRa.beginPacket();
  LoRa.write(GATEWAY_ADDRESS);  // Dirección del Gateway
  LoRa.write(NODE_ADDRESS);  // Dirección del nodo
  LoRa.write(msgID);  // ID del mensaje
  LoRa.write(message.length());  // Longitud del mensaje
  LoRa.print(message);
  LoRa.endPacket();

  msgID++;

  // Esperar ACK
  unsigned long startTime = millis();
  bool ackReceived = false;
  while (millis() - startTime < 2000) {  // 2 segundos de espera
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
      int recipient = LoRa.read();
      byte sender = LoRa.read();
      byte incomingMsgID = LoRa.read();
      byte length = LoRa.read();

      String incoming = "";
      while (LoRa.available()) {
        incoming += (char)LoRa.read();
      }

      if (recipient == NODE_ADDRESS && incoming == "ACK" && incomingMsgID == msgID - 1) {
        ackReceived = true;
        break;
      }
    }
  }

  if (ackReceived) {
    Serial.println("ACK recibido.");
  } else {
    Serial.println("ACK no recibido.");
  }

  // Intervalo aleatorio entre envios
  delay(random(2000, 5000));
}

```

# ESP32

## Conexiones

| Ra-02 | ESP32  |
|:--------------:|:--------------:|
| GND   | GND    |
| 3.3 V | 3.3 V  |
| RST   | GPIO14 |
| NSS   | GPIO5/15*  |
| MOSI  | GPIO23 |
| MISO  | GPIO19 |
| SCK   | GPIO18 |
| DIO0  | GPIO26/25* |

*Cambio de pines cuando se usa con el módulo LoRa SX1278 Ra-02
  
## Código de verificación de módulo Ra-02

### Primera versión
```c++
#include <SPI.h> // include libraries
#include <LoRa.h>

void setup() {
  Serial.begin(9600); // initialize serial
  while (!Serial);

  Serial.println("LoRa Dump Registers");

  // Define los pines CS, reset y DIO0
  LoRa.setPins(5, 14, 26); // CS -> GPIO5, reset -> GPIO14, IRQ(DIO0) -> GPIO26

  if (!LoRa.begin(433E6)) { // inicializa el módulo LoRa a 433 MHz
    Serial.println("LoRa init failed. Check your connections.");
    while (true); // si falla, detenerse
  }

  LoRa.dumpRegisters(Serial);
}

void loop() {
}
```

### Segunda versión
```C++
#include <SPI.h>
#include <LoRa.h>

// Define los pines utilizados
#define SS_PIN    5    // NSS
#define RST_PIN   14   // Reset
#define DIO0_PIN  26    // DIO0

void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial.println("LoRa Dump Registers");

  // Configura los pines del módulo LoRa
  LoRa.setPins(SS_PIN, RST_PIN, DIO0_PIN);

  if (!LoRa.begin(433E6)) {
    Serial.println("LoRa init failed. Check your connections.");
    while (true);
  }

  LoRa.dumpRegisters(Serial);
}

void loop() {
}
```

Los pines SPI (MISO, MOSI, SCK) no necesitan ser definidos explícitamente porque el ESP32 usa pines fijos para SPI:

+ MISO: GPIO19
+ MOSI: GPIO23
+ SCK: GPIO18
