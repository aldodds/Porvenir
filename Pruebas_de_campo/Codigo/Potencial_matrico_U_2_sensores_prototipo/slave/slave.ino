#include <Wire.h>

#define I2C_SDA_PIN 21
#define I2C_SCL_PIN 22
#define I2C_SLAVE_ADDR 0x08

struct __attribute__((packed)) SoilStation {
  int resistance1;
  int resistance2;
};

SoilStation soilStation = { 0 };

volatile bool shouldSleep = false;
volatile bool sending = false;

void setup() {
  Serial.begin(9600);

  setupMatricPotential();

  Wire.setPins(I2C_SDA_PIN, I2C_SCL_PIN);
  Wire.begin(I2C_SLAVE_ADDR); // Inicializar como esclavo con la dirección 0x08
  Wire.onRequest(requestEvent); // Registrar el evento de solicitud del maestro
}

void loop() {
  if (!sending) {
    readMatricPotential();
    printMatricPotential();
    sending = true;
  }
  
  delay(100);
}

void requestEvent() {
  Wire.write((uint8_t*) &soilStation, sizeof(SoilStation));
}
