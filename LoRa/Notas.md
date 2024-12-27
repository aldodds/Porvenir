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

### Primer código

Inspeccionar y verificar los registros internos del módulo LoRa para depuración o ajuste avanzado de configuraciones. 
Es útil en las primeras etapas de desarrollo de tu proyecto para **asegurarte de que el módulo LoRa está correctamente conectado y configurado**. También puede ser utilizado cuando necesitas ajustar parámetros específicos del módulo o depurar problemas en la comunicación.

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