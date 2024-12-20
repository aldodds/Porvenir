# Microcontrolador

ESP32 devkit C chip CH340

# Librería

+ BluetoothSerial by Henry Abrahamsen
+ Arduino ESP32 Boards by Arduino (gestor de tarjetas)
+ esp32 by espessif (gestor de tarjetas)

# Códigos en Arduino

## Identificar dirección MAC Bluetooth

```C
#include <esp_bt_main.h>
#include <esp_bt_device.h>

void setup() {
  Serial.begin(115200);

  // Inicializar Bluetooth
  if (!btStart()) {
    Serial.println("No se pudo iniciar el Bluetooth.");
    return;
  }
  if (esp_bluedroid_init() != ESP_OK) {
    Serial.println("No se pudo inicializar Bluedroid.");
    return;
  }
  if (esp_bluedroid_enable() != ESP_OK) {
    Serial.println("No se pudo habilitar Bluedroid.");
    return;
  }

  // Obtener y mostrar la dirección MAC del Bluetooth
  const uint8_t *mac = esp_bt_dev_get_address();
  if (mac == nullptr) {
    Serial.println("No se pudo obtener la dirección MAC.");
    return;
  }

  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  Serial.print("Dirección MAC del Bluetooth: ");
  Serial.println(macStr);
}

void loop() {
}
```