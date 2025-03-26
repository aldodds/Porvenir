/*
V 1.0
Ejemplo básico que utiliza el protocolo I2C para medir simultáneamente luz ambiental (ALS) y radiación UV con el sensor LTR390UV de DFRobot
El sensor no puede medir luz ambiental y radiación UV al mismo tiempo. Por eso, el código alterna entre: modo de luz ambienta y UV
*/

#include "DFRobot_LTR390UV.h"

DFRobot_LTR390UV ltr390(/*addr =*/ LTR390UV_DEVICE_ADDR, /*pWire =*/ &Wire);

void setup() {
  Serial.begin(115200);

  // Inicializar el sensor
  while (ltr390.begin() != 0) {
    Serial.println("Error al inicializar el sensor!!");
    delay(1000);
  }
  Serial.println("El sensor se inicializó correctamente!!");

  // Configuración general del sensor
  ltr390.setALSOrUVSMeasRate(ltr390.e18bit, ltr390.e100ms); // Resolución 18 bits, tiempo de muestreo 100 ms
  ltr390.setALSOrUVSGain(ltr390.eGain3);                   // Ganancia de 3
}

void loop() {
  // Modo de medición de luz ambiental
  ltr390.setMode(ltr390.eALSMode); // Establecer modo de luz ambiental
  delay(100); // Esperar para asegurar la lectura

  float als = ltr390.readALSTransformData(); // Leer luz ambiental en lux
  Serial.print("Luz ambiental (ALS): ");
  Serial.print(als);
  Serial.println(" Lux");

  // Modo de medición UV
  ltr390.setMode(ltr390.eUVSMode); // Establecer modo UV
  delay(100); // Esperar para asegurar la lectura

  uint32_t uv = ltr390.readOriginalData(); // Leer datos UV
  Serial.print("Índice UV: ");
  Serial.println(uv);

  delay(1000); // Pausa antes de la siguiente iteración
}
