#define MQTT_SERVER "mqtt3.thingspeak.com"
#define MQTT_PORT 1883
#define MQTT_USERNAME "EhANFhw0BQEGHjkDPQcELw4"
#define MQTT_PASSWORD "M5I9avf0GBhwrkrLNJ75zFoX"
#define MQTT_CLIENT_ID "EhANFhw0BQEGHjkDPQcELw4"

#define WEATHER_STATION_CHANNEL "channels/2791069/publish"
#define SOIL_STATION_CHANNEL "channels/2791076/publish"
#define MATRIC_POTENTIAL_AMBIENT_LIGHT_UV_INDEX_CHANNEL "channels/2906294/publish"

void setupMqtt() {
  mqttClient.setClient(gsmClient);
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setSocketTimeout(30);
  mqttClient.setKeepAlive(60);

  setupMqttConnection();
}

void setupMqttConnection() {
  Serial.print("Intentando conectar al broker MQTT...");
  if (mqttClient.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD)) {
    SerialMon.println("Conectado.");
  } else {
    SerialMon.println("Fallo.");
  }
}

void maintainMqttConnection() {
  if (!mqttClient.connected()) {
    Serial.println("Recuperando conexión fallo...");
    Serial.print("Error de conexión mqtt, rc=");
    Serial.println(mqttClient.state());
    setupMqtt();
  }
}

// send data functions

void sendResistances(
  float resistance1,
  float resistance2
) {
  if (!mqttClient.connected()) {
    SerialMon.println("MQTT Broker disconnected.");
    return;
  }

  String payload = "&field1=" + String(resistance1) + "&field2=" + String(resistance2);

  if (mqttClient.publish(MATRIC_POTENTIAL_AMBIENT_LIGHT_UV_INDEX_CHANNEL, payload.c_str())) {
    SerialMon.println("Resistance 1 and resistance 2 sent successfully.");
  } else {
    SerialMon.println("Resistance 1 and resistance 2 transmission failure.");
  }
}
