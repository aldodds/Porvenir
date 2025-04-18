#define MQTT_SERVER "mqtt3.thingspeak.com"
#define MQTT_PORT 1883
#define MQTT_USERNAME "EhANFhw0BQEGHjkDPQcELw4"
#define MQTT_PASSWORD "M5I9avf0GBhwrkrLNJ75zFoX"
#define MQTT_CLIENT_ID "EhANFhw0BQEGHjkDPQcELw4"

void setupMqtt() {
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setSocketTimeout(30);
  mqttClient.setKeepAlive(60);
}

void reconnect() {
  Serial.print("Intentando conectar al broker MQTT...");
  setupMqtt();
  if (mqttClient.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD)) {
    Serial.println("Conectado");
  } else {
    Serial.print("fallo, rc=");
    Serial.print(mqttClient.state());
    Serial.println(" intentando en 5 segundos");
    delay(5000);
  }
}