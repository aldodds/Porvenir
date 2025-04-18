#include <Wire.h>

#define SLAVE_ADDRESS 0x08  // Dirección I2C del ESP32 esclavo

void requestEvent() {
  Wire.write(1);  // Enviar el valor "1" al maestro
}

void setup() {
  Wire.begin(SLAVE_ADDRESS);  // Configurar ESP32 como esclavo
  Wire.onRequest(requestEvent);  
  Serial.begin(115200);
}

void loop() {
  delay(100);  // Pequeño retardo para estabilidad
}