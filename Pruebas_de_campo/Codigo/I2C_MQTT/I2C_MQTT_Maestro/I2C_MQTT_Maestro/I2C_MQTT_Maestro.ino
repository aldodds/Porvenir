// Definir el modelo del módem ANTES de incluir TinyGSM
#define TINY_GSM_MODEM_SIM800      // Módem es SIM800
#define TINY_GSM_RX_BUFFER 1024  // Buffer RX a 1Kb
#define TINY_GSM_USE_GPRS true

#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>
#include <TinyGsmClient.h>
#include <PubSubClient.h>
#include "DFRobot_LTR390UV.h"

#define SDA_PIN 21  // Pin SDA por defecto en ESP32
#define SCL_PIN 22  // Pin SCL por defecto en ESP32

#define SLAVE_ADDRESS  0x08  // Dirección I2C del ESP32 esclavo
// #define SENSOR_ADDRESS 0x1C  // Dirección I2C del sensor meteorológico
#define LCD_ADDRESS    0x27  // Dirección I2C del LCD (ajústala si es diferente)

// Inicializar LCD con dirección I2C
LiquidCrystal_PCF8574 lcd(LCD_ADDRESS);

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

// Inicializar sensor UV
DFRobot_LTR390UV sensorUV(/*addr = */LTR390UV_DEVICE_ADDR, /*pWire = */&Wire);

void setup() {
  Wire.begin(SDA_PIN, SCL_PIN);  // Configurar I2C en GPIO21 (SDA) y GPIO22 (SCL)
  Serial.begin(115200);

  // Inicializar LCD
  lcd.begin(16, 2); 
  lcd.setBacklight(255); // Activar retroiluminación
  lcd.setCursor(0, 0);
  lcd.print("Hello World");  // Escribir "Hello World" en la primera línea del LCD

  // Inicializar sensor UV
  while(sensorUV.begin() != 0){
    Serial.println(" Error al inicializar el sensor!!");
    delay(1000);
  }

  Serial.println("Sensor UV encontrado!");

  sensorUV.setALSOrUVSMeasRate(sensorUV.e18bit, sensorUV.e1000ms);
  sensorUV.setALSOrUVSGain(sensorUV.eGain3);
  sensorUV.setMode(sensorUV.eUVSMode);

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

  uint8_t response = 0;
  uint32_t uvData = 0;

  // Solicitar dato al ESP32 esclavo
  Wire.requestFrom(SLAVE_ADDRESS, 1);  
  if (Wire.available()) {
    response = Wire.read();
    Serial.print("ESP32 Esclavo respondió: ");
    Serial.println(response);
  }

  delay(3000);  // Esperar 3 segundos antes de pedir al sensor

  // Leer datos del sensor UV
  uvData = sensorUV.readOriginalData();
  Serial.print("Índice UV: ");
  Serial.println(uvData);

  // Mostrar en LCD
  lcd.setCursor(0, 1);  // Segunda línea
  lcd.print("UV Index: ");
  lcd.print(uvData);

  if (mqttClient.connected()) {
    String message = "&field3=" + String(uvData) + "&field4=" + String(response);

    if (mqttClient.publish(CHANNEL_1_TOPIC, message.c_str())) {
        Serial.println("Mensaje publicado exitosamente.");
      } else {
        Serial.println("Error al publicar el mensaje.");
      }
  }

  delay(10000);  // Esperar 3 segundos antes de repetir

  mqttClient.loop();
}
