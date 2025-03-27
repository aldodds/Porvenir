// Definir el modelo del módem ANTES de incluir TinyGSM
#define TINY_GSM_MODEM_SIM800      // Módem es SIM800
#define TINY_GSM_RX_BUFFER 1024  // Buffer RX a 1Kb
#define TINY_GSM_USE_GPRS true

#include <TinyGsmClient.h>
#include <PubSubClient.h>

// Configuración de pines TTGO T-Call
#define MODEM_RST            5
#define MODEM_PWKEY          4
#define MODEM_POWER_ON       23
#define MODEM_TX             27
#define MODEM_RX             26
#define LED_PIN              13    // LED interno de la ESP32 GPIO13

#define SerialAT  Serial1

// Configuración del módulo SIM800
TinyGsm modem(SerialAT);
TinyGsmClient gsmClient(modem);

PubSubClient mqttClient(gsmClient);

#define CHANNEL_1_TOPIC "channels/2791069/publish"

void setup() {
  Serial.begin(115200);

  pinMode(13, OUTPUT);

  setupGsm();

  setupMqtt();
}

void loop() {
  if (!modem.isGprsConnected()) {
    reconnectGsm();
    if (!modem.isGprsConnected()) {
      Serial.println("Error: No se pudo reconectar GPRS.");
      return;
    }
  }

  if (!mqttClient.connected()) {
    reconnect();
  }

  if (mqttClient.connected()) {
    
    if (mqttClient.publish(CHANNEL_1_TOPIC, "&field1=1")) {
        Serial.println("Mensaje publicado exitosamente.");
        digitalWrite(13, HIGH);
      } else {
        Serial.println("Error al publicar el mensaje.");
      }
  }

  delay(1000);
  digitalWrite(13, LOW);

  delay(9000);
  

  mqttClient.loop();
}
