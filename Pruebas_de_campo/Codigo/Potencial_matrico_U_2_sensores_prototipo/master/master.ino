// Definir el modelo del módem ANTES de incluir TinyGSM
#define TINY_GSM_MODEM_SIM800    // Módem es SIM800
#define TINY_GSM_RX_BUFFER 1024  // Buffer RX a 1Kb
#define TINY_GSM_USE_GPRS true
#define TINY_GSM_YIELD_MS 10

#include <TinyGsmClient.h>
#include <PubSubClient.h>

#define SerialMon Serial
#define SerialAT Serial1

#define BUFFER_SIZE 5

// Configuración del módulo SIM800
TinyGsm modem(SerialAT);
TinyGsmClient gsmClient(modem);

// Configuración del mqtt
PubSubClient mqttClient(gsmClient);

// Structs

struct __attribute__((packed)) SoilStation {
  int resistance1;
  int resistance2;
};

SoilStation soilStation = { 0 };

void setup() {
  Serial.begin(115200);

  setupGsm();

  if (modem.isNetworkConnected() && modem.isGprsConnected())
    setupMqtt();
}

void loop() {
  // Mantener conexiones para poder operar

  maintainGsmConnection();
  if (!modem.isNetworkConnected() || !modem.isGprsConnected()) {
    SerialMon.println("TinyGSM no conectado.");
    return;
  }

  maintainMqttConnection();
  if (!mqttClient.connected()) {
    SerialMon.println("MqttBroker no conectado.");
    return;
  }

  mqttClient.loop();

  // Lectura y envio de datos de sensores

  delay(1000);

  sendSoilStationData();
}
