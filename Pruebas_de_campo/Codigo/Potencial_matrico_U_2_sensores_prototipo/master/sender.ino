void sendSoilStationData() {
  Wire.requestFrom(I2C_SLAVE_ADDR, sizeof(SoilStation)); // Solicita 1 byte al esclavo

  Wire.requestFrom(I2C_SLAVE_ADDR, sizeof(SoilStation)); // Solicita 1 byte al esclavo

  if (Wire.available() == sizeof(SoilStation)) {
    Wire.readBytes((uint8_t*) &soilStation, sizeof(SoilStation));

    sendResistances(
      soilStation.resistance1,
      soilStation.resistance2
    );
  }
}