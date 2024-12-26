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

### Maestro

```c
#include <SPI.h>
#include <RH_RF95.h>

// Pines del módulo
#define RFM95_CS 10
#define RFM95_RST 9
#define RFM95_INT 2

// Frecuencia LoRa
#define RF95_FREQ 433.0

RH_RF95 rf95(RFM95_CS, RFM95_INT);

void setup() {
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);
  Serial.begin(9600);

  // Reiniciar módulo
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  if (!rf95.init()) {
    Serial.println("Fallo en inicialización LoRa");
    while (1);
  }

  rf95.setFrequency(RF95_FREQ);
  rf95.setTxPower(13, false); // Potencia de transmisión
}

void loop() {
  uint8_t slaves[] = {1, 2}; // Direcciones de los esclavos
  for (uint8_t i = 0; i < 2; i++) {
    Serial.print("Solicitando datos al esclavo: ");
    Serial.println(slaves[i]);

    // Enviar solicitud
    uint8_t request[] = {slaves[i]};
    rf95.send(request, sizeof(request));
    rf95.waitPacketSent();

    // Esperar respuesta
    if (rf95.waitAvailableTimeout(2000)) { // Timeout 2 segundos
      uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
      uint8_t len = sizeof(buf);
      if (rf95.recv(buf, &len)) {
        Serial.print("Datos recibidos del esclavo ");
        Serial.print(slaves[i]);
        Serial.print(": ");
        Serial.println((char*)buf);
      }
    } else {
      Serial.print("No hay respuesta del esclavo ");
      Serial.println(slaves[i]);
    }
  }
}
```

## Esclavo

**Esclavo 1**

```c
#include <SPI.h>
#include <RH_RF95.h>

// Pines del módulo
#define RFM95_CS 10
#define RFM95_RST 9
#define RFM95_INT 2

// Frecuencia LoRa
#define RF95_FREQ 433.0

RH_RF95 rf95(RFM95_CS, RFM95_INT);

// Dirección del esclavo
#define SLAVE_ADDRESS 1

void setup() {
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);
  Serial.begin(9600);

  // Reiniciar módulo
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  if (!rf95.init()) {
    Serial.println("Fallo en inicialización LoRa");
    while (1);
  }

  rf95.setFrequency(RF95_FREQ);
}

void loop() {
  if (rf95.available()) {
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);

    // Recibir solicitud
    if (rf95.recv(buf, &len) && buf[0] == SLAVE_ADDRESS) {
      Serial.println("Solicitud recibida del maestro");

      // Enviar datos
      uint8_t response[] = "1";
      rf95.send(response, sizeof(response));
      rf95.waitPacketSent();
      Serial.println("Datos enviados al maestro");
    }
  }
}
```

**Esclavo 2**

El código es idéntico al Esclavo 1, excepto por la dirección:

```c
#define SLAVE_ADDRESS 2
```

### Explicación

1. El maestro solicita información al Esclavo 1 (envía 1).
2. El Esclavo 1 verifica que la solicitud es para él, responde con sus datos.
3. El maestro solicita información al Esclavo 2 (envía 2).
4. El Esclavo 2 responde de manera similar.

Si un esclavo está apagado o su señal se demora, el maestro simplemente pasa al siguiente tras el timeout, evitando conflictos.

## Diagnóstico

A continuación, se proporcionan dos códigos de diagnóstico: uno para el maestro y otro para los esclavos. Estos códigos realizarán pruebas básicas para verificar que los módulos LoRa Ra-02 están funcionando correctamente y que la comunicación SPI con el Arduino está establecida. Los diagnósticos incluyen:

1. Verificación de inicialización del módulo LoRa.
2. Pruebas de envío y recepción de mensajes simples.
3. Monitorización de errores en la comunicación.


### Maestro

Este código enviará mensajes simples periódicamente y mostrará en el monitor serial si el módulo LoRa está funcionando correctamente.

```c
#include <SPI.h>
#include <RH_RF95.h>

// Pines del módulo
#define RFM95_CS 10
#define RFM95_RST 9
#define RFM95_INT 2

// Frecuencia LoRa
#define RF95_FREQ 433.0

RH_RF95 rf95(RFM95_CS, RFM95_INT);

void setup() {
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);
  Serial.begin(9600);
  Serial.println("Iniciando diagnóstico del maestro...");

  // Reiniciar módulo
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  if (!rf95.init()) {
    Serial.println("Error: Fallo en inicialización del módulo LoRa.");
    while (1); // Detener el programa si no inicia
  }

  rf95.setFrequency(RF95_FREQ);
  rf95.setTxPower(13, false); // Configurar potencia de transmisión
  Serial.println("Módulo LoRa inicializado correctamente.");
}

void loop() {
  // Enviar un mensaje de prueba
  const char* testMessage = "Prueba desde maestro";
  Serial.println("Enviando mensaje de prueba...");
  rf95.send((uint8_t*)testMessage, strlen(testMessage));
  rf95.waitPacketSent();
  Serial.println("Mensaje enviado. Esperando respuesta...");

  // Esperar una respuesta
  if (rf95.waitAvailableTimeout(2000)) { // Timeout 2 segundos
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);

    if (rf95.recv(buf, &len)) {
      Serial.print("Respuesta recibida: ");
      Serial.println((char*)buf);
    } else {
      Serial.println("Error: No se pudo decodificar la respuesta.");
    }
  } else {
    Serial.println("Error: No se recibió respuesta (timeout).");
  }

  delay(5000); // Esperar 5 segundos antes de la siguiente prueba
}
```

Este código permitirá al esclavo escuchar mensajes del maestro y responder con un mensaje de confirmación. También mostrará errores en caso de fallos.

### Esclavo

```c
#include <SPI.h>
#include <RH_RF95.h>

// Pines del módulo
#define RFM95_CS 10
#define RFM95_RST 9
#define RFM95_INT 2

// Frecuencia LoRa
#define RF95_FREQ 433.0

RH_RF95 rf95(RFM95_CS, RFM95_INT);

void setup() {
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);
  Serial.begin(9600);
  Serial.println("Iniciando diagnóstico del esclavo...");

  // Reiniciar módulo
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  if (!rf95.init()) {
    Serial.println("Error: Fallo en inicialización del módulo LoRa.");
    while (1); // Detener el programa si no inicia
  }

  rf95.setFrequency(RF95_FREQ);
  Serial.println("Módulo LoRa inicializado correctamente.");
}

void loop() {
  Serial.println("Esperando mensaje del maestro...");

  if (rf95.available()) {
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);

    if (rf95.recv(buf, &len)) {
      Serial.print("Mensaje recibido: ");
      Serial.println((char*)buf);

      // Responder con un mensaje de confirmación
      const char* response = "Respuesta del esclavo";
      rf95.send((uint8_t*)response, strlen(response));
      rf95.waitPacketSent();
      Serial.println("Respuesta enviada.");
    } else {
      Serial.println("Error: No se pudo decodificar el mensaje recibido.");
    }
  } else {
    Serial.println("No hay mensajes disponibles.");
  }

  delay(1000); // Esperar 1 segundo antes de volver a comprobar
}
```
