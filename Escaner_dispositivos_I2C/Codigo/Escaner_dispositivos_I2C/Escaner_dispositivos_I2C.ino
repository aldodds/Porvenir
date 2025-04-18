// Este código escanea y detecta todos los dispositivos conectados al bus I2C en un rango de direcciones.
#include <Wire.h>

void setup() {
  Wire.begin();
  Serial.begin(9600);
  
  Serial.println("Escaneando dispositivos I2C...");
  
  for(byte address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    byte error = Wire.endTransmission();
    
    if (error == 0) {
      Serial.print("Dispositivo encontrado en 0x");
      Serial.println(address, HEX);
    }
  }
}

void loop() {

}