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

## Explicación

1. El maestro solicita información al Esclavo 1 (envía 1).
2. El Esclavo 1 verifica que la solicitud es para él, responde con sus datos.
3. El maestro solicita información al Esclavo 2 (envía 2).
4. El Esclavo 2 responde de manera similar.

Si un esclavo está apagado o su señal se demora, el maestro simplemente pasa al siguiente tras el timeout, evitando conflictos.