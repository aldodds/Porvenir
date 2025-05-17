#include <Wire.h>

#define I2C_SDA_PIN 21
#define I2C_SCL_PIN 22
#define I2C_SLAVE_ADDR 0x08

void setupI2C() {
  Wire.setPins(I2C_SDA_PIN, I2C_SCL_PIN);
  Wire.begin();
}
