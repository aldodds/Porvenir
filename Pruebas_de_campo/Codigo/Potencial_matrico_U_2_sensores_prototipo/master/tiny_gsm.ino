// Configuración de pines TTGO T-Call
#define MODEM_RST            5
#define MODEM_PWKEY          4
#define MODEM_POWER_ON       23
#define MODEM_TX             27
#define MODEM_RX             26
#define LED_PIN              13    // LED interno de la ESP32 GPIO13

#define GSM_MAX_ATTEMPTS    5

int gsmAttempts = 0;

void setupGsm() {
  // Configurar pines del módem
  pinMode(MODEM_PWKEY, OUTPUT);
  pinMode(MODEM_RST, OUTPUT);
  pinMode(MODEM_POWER_ON, OUTPUT);
  pinMode(LED_PIN, OUTPUT);     // Configuración del LED interno
  digitalWrite(LED_PIN, LOW);   // Asegurar que el LED comience apagado

  digitalWrite(MODEM_PWKEY, LOW);
  digitalWrite(MODEM_RST, HIGH);
  digitalWrite(MODEM_POWER_ON, HIGH);

  SerialMon.println("| Configurando módulo GSM |");
  
  // Configurar módulo GSM
  SerialAT.begin(9600, SERIAL_8N1, MODEM_RX, MODEM_TX);
  delay(3000);

  setupGsmModem();
  setupGsmConnection();
}

void setupGsmModem() {
  // Inicializar módem
  SerialMon.print(F("\tInicializando modem..."));
  modem.restart();
  vTaskDelay(pdMS_TO_TICKS(10000)); // Esperar 10 segundos para que el módulo se estabilice
  SerialMon.println(F("Inicializado."));
  
  // Verificar si el módem responde
  SerialMon.print(F("\tVerificando módem..."));
  if(!modem.testAT()) {
    SerialMon.println("Error.");
    return;
  }
  SerialMon.println(F("Verificado."));
}

void setupGsmConnection() {
  modem.gprsConnect("internet.comcel.com.co", "comcel", "comcel");

  // Verificar registro en la red

  SerialMon.println(F("\tVerificando registro en la red..."));
  if(!modem.waitForNetwork()) {
    SerialMon.println(F("\tError: No hay registro en la red"));
    return;
  }
  Serial.println(F("\tRegistro en red OK"));

  // Verificar intensidad de señal
  int signalQuality = modem.getSignalQuality();
  SerialMon.print("\tIntensidad de señal: " + String(signalQuality));

  if (modem.isGprsConnected()) {
    SerialMon.println(F("\tGPRS está activo y conectado."));
  } else {
      SerialMon.println(F("\tGPRS no está conectado."));  
  }
}

void maintainGsmConnection() {
  if (!modem.isNetworkConnected() || !modem.isGprsConnected() && gsmAttempts < GSM_MAX_ATTEMPTS) {
    gsmAttempts++;
    setupGsmModem();
    delay(500);
    setupGsmConnection();
  }
}