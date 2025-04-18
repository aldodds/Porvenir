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

  Serial.println("Configurando módulo GSM...");
  
  // Configurar módulo GSM
  SerialAT.begin(9600, SERIAL_8N1, MODEM_RX, MODEM_TX);
  
  delay(3000);

  // Inicializar módem
  Serial.println("Inicializando modem...");
  modem.restart();
  delay(10000); // Esperar 10 segundos para que el módulo se estabilice

  Serial.print(F("Setting SSID/password..."));
  
  // Verificar si el módem responde
  Serial.println("Verificando módem...");
  if(!modem.testAT()) {
    Serial.println("Error: No hay respuesta del módem");
    return;
  }
  Serial.println("Módem respondiendo OK");

  //sendCommand("AT+CIPSSL=0");

  modem.gprsConnect("internet.comcel.com.co", "comcel", "comcel");

  while (SerialAT.available()) {
    Serial.write(SerialAT.read());
  }

  // Verificar registro en la red

  Serial.println("Verificando registro en la red...");
  if(!modem.waitForNetwork()) {
    Serial.println("Error: No hay registro en la red");
    return;
  }
  Serial.println("Registro en red OK");

  // Verificar intensidad de señal
  int signalQuality = modem.getSignalQuality();
  Serial.println("Intensidad de señal: " + String(signalQuality));

  if (modem.isGprsConnected()) {
    Serial.println("GPRS está activo y conectado.");
  } else {
      Serial.println("GPRS no está conectado.");  
  }
}

void reconnectGsm() {
  Serial.println("Reconectando GPRS...");
  modem.gprsConnect("internet.comcel.com.co", "comcel", "comcel");
}