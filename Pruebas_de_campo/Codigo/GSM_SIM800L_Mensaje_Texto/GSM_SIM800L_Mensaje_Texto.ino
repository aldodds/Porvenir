/*
V.1.0
Este código envía un mensaje de texto "Porvenir" al número de teléfono especificado (+573...). 
Espera un comando (1) desde el monitor serial para iniciar el envío. 
Muestra mensajes en el monitor serial y controla un LED para indicar el estado del envío.
*/

// Definir el modelo del módem ANTES de incluir TinyGSM
#define TINY_GSM_MODEM_SIM800      // Módem es SIM800
#define TINY_GSM_RX_BUFFER   1024  // Buffer RX a 1Kb

#include <TinyGsmClient.h>

// Configuración del número de teléfono
#define SMS_TARGET  "+573042102865"

// Configuración de pines TTGO T-Call
#define MODEM_RST            5
#define MODEM_PWKEY          4
#define MODEM_POWER_ON       23
#define MODEM_TX             27
#define MODEM_RX             26
#define LED_PIN              13    // LED interno de la ESP32 GPIO13

// Configuración de Serial
#define SerialMon Serial
#define SerialAT  Serial1

// Configuración del módulo SIM800
TinyGsm modem(SerialAT);

void setup() {
  // Inicializar Serial Monitor
  SerialMon.begin(115200);
  SerialMon.println("Iniciando configuración...");

  // Configurar pines del módem
  pinMode(MODEM_PWKEY, OUTPUT);
  pinMode(MODEM_RST, OUTPUT);
  pinMode(MODEM_POWER_ON, OUTPUT);
  pinMode(LED_PIN, OUTPUT);     // Configuración del LED interno
  digitalWrite(LED_PIN, LOW);   // Asegurar que el LED comience apagado

  digitalWrite(MODEM_PWKEY, LOW);
  digitalWrite(MODEM_RST, HIGH);
  digitalWrite(MODEM_POWER_ON, HIGH);

  SerialMon.println("Configurando módulo GSM...");
  
  // Configurar módulo GSM
  SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
  delay(3000);

  // Inicializar módem
  SerialMon.println("Inicializando modem...");
  modem.restart();
  delay(10000); // Esperar 10 segundos para que el módulo se estabilice

  // Configurar APN de Claro Colombia
  modem.gprsConnect("internet.comcel.com.co", "comcel", "comcel");

  // Verificar si el módem responde
  SerialMon.println("Verificando módem...");
  if(!modem.testAT()) {
    SerialMon.println("Error: No hay respuesta del módem");
    return;
  }
  SerialMon.println("Módem respondiendo OK");

  // Verificar registro en la red
  SerialMon.println("Verificando registro en la red...");
  if(!modem.waitForNetwork()) {
    SerialMon.println("Error: No hay registro en la red");
    return;
  }
  SerialMon.println("Registro en red OK");

  // Verificar intensidad de señal
  int signalQuality = modem.getSignalQuality();
  SerialMon.println("Intensidad de señal: " + String(signalQuality));

  SerialMon.println("Esperando señal de inicio...");
}

void loop() {
  // Verificar si hay datos en el monitor serial
  if (SerialMon.available()) {
    char command = SerialMon.read();

    // Si se recibe el comando '1'
    if (command == '1') {
      SerialMon.println("Se recibió el comando: 1");

      // Apagar el LED al inicio del intento de envío
      digitalWrite(LED_PIN, LOW);
      
      // Intentar enviar SMS
      SerialMon.println("Intentando enviar SMS...");
      bool smsSent = modem.sendSMS(SMS_TARGET, "Porvenir");
      
      if (smsSent) {
        SerialMon.println("SMS enviado exitosamente");
        digitalWrite(LED_PIN, HIGH);  // Encender LED solo si se envió el SMS exitosamente
        delay(1000);                  // Mantener el LED encendido 1 segundo
        digitalWrite(LED_PIN, LOW);   // Apagar el LED después del envío exitoso
      } else {
        SerialMon.println("Error al enviar SMS");
      }

      // Mostrar mensaje de finalización
      SerialMon.println("Proceso completado.");
      SerialMon.println("Esperando señal de inicio...");
    }
  }
}
