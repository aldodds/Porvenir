// ------------------------------------------------------------
// Código mejorado para lectura de sensores de humedad del suelo Watermark 200SS
// usando un multiplexor 74HC4052 y un microcontrolador ESP32 (ESP-WROOM-32).
// Se realiza una lectura pseudo CA (doble dirección) para evitar polarización,
// se calcula la resistencia del sensor mediante un divisor resistivo,
// y se convierte dicha resistencia a centibares (cb o kPa) de tensión del suelo.
// El código está preparado para dos sensores, con soporte para sensor de temperatura opcional.
// ------------------------------------------------------------

// --- Definición de Pines ESP32 ---
const int MUX_INH_PIN = 27;     // Pin INH (inhabilitación) del 74HC4052
const int MUX_A_PIN   = 33;     // Pin A de selección del canal en el multiplexor
const int MUX_B_PIN   = 26;     // Pin B de selección del canal en el multiplexor
const int EXCITE_1_PIN = 32;    // Pin que activa una dirección de corriente
const int EXCITE_2_PIN = 13;    // Pin que activa la dirección opuesta de corriente
const int ANALOG_READ_PIN = 34; // Pin analógico del ESP32 para leer el voltaje

// --- Constantes del Circuito y Sensor ---
const float Rx = 10000.0;       // Resistencia en serie de 10kΩ usada en el divisor
const float MUX_RESISTANCE = 86.0;  // Resistencia medida del multiplexor (86Ω)
const float SupplyV = 3.3;      // Voltaje de referencia (alimentación del sistema)
const float ADC_MAX_VALUE = 4095.0; // Valor máximo del ADC del ESP32 (resolución de 12 bits)
const int   NUM_OF_READS = 20;   // Número de lecturas por dirección (aumentado para mejor precisión)
const long  DEFAULT_TEMP_C = 24; // Temperatura fija asumida (en grados Celsius)

// --- Constantes de Calibración y Límites ---
const long  OPEN_RESISTANCE = 35000; // Resistencia que se considera circuito abierto
const long  SHORT_RESISTANCE = 200;  // Resistencia que se considera cortocircuito
const long  SHORT_CB = 240;          // Valor CB fijo si hay cortocircuito
const long  OPEN_CB = 255;           // Valor CB fijo si hay circuito abierto
const float C_FACTOR = 1.1;          // Factor de corrección de calibración

// --- Tiempos de Estabilización ---
const int EXCITE_STABILIZE_TIME = 100;   // Microsegundos de estabilización antes de lectura (como en código Irrometer)
const int POLARITY_CHANGE_DELAY = 100;   // Milisegundos entre cambios de polaridad para evitar carga residual

// --- Variables Globales ---
float WM1_Resistance = 0.0;    // Resistencia medida del Sensor 1
float WM2_Resistance = 0.0;    // Resistencia medida del Sensor 2
int   WM1_CB = 0;              // Valor CB (kPa) estimado del Sensor 1
int   WM2_CB = 0;              // Valor CB (kPa) estimado del Sensor 2

// --- Prototipos de Funciones ---
float readWMsensor();                       // Función para leer resistencia desde el sensor seleccionado
int   myCBvalue(float res, float TC, float cF);  // Función para convertir resistencia a CB (kPa)

// --- Configuración inicial ---
void setupMatricPotential() {
  Serial.println("Iniciando Lector de Sensores Watermark con ESP32 y 74HC4052");

  pinMode(MUX_INH_PIN, OUTPUT);                       // Configura pin INH como salida
  pinMode(MUX_A_PIN, OUTPUT);                         // Configura pin A como salida
  pinMode(MUX_B_PIN, OUTPUT);                         // Configura pin B como salida
  pinMode(EXCITE_1_PIN, OUTPUT);                      // Configura excitación 1 como salida
  pinMode(EXCITE_2_PIN, OUTPUT);                      // Configura excitación 2 como salida

  digitalWrite(MUX_INH_PIN, HIGH);                    // Deshabilita el multiplexor al inicio
  digitalWrite(EXCITE_1_PIN, LOW);                    // Asegura que no haya corriente al inicio
  digitalWrite(EXCITE_2_PIN, LOW);                    // Asegura que no haya corriente al inicio

  delay(100);                                         // Espera breve
  Serial.println("Setup completado.");
}

void printMatricPotential() {
  Serial.print("Sensor 1: Resistencia=");
  Serial.print(WM1_Resistance);
  Serial.print(" Ohms, Tension=");
  Serial.print(abs(soilStation.resistance1));
  Serial.println(" cb/kPa");

  Serial.print("Sensor 2: Resistencia=");
  Serial.print(WM2_Resistance);
  Serial.print(" Ohms, Tension=");
  Serial.print(abs(soilStation.resistance2));
  Serial.println(" cb/kPa");
}

// --- Bucle principal ---
void readMatricPotential() {
  Serial.println("\n--- Iniciando ciclo de lectura ---");

  digitalWrite(MUX_INH_PIN, LOW);                     // Habilita el multiplexor
  delay(10);                                          // Breve espera para estabilización

  Serial.println("Seleccionando Sensor 1 (Canal 1)...");
  // Si usamos sensor de temperatura, los sensores WM van en canales 1,2
  // Si no, usamos canales 0,1 para los sensores WM
  digitalWrite(MUX_B_PIN, LOW);                     // Selección canal 0 (B=0, A=0)
  digitalWrite(MUX_A_PIN, LOW);
  delay(10);                                          // Espera tras cambiar canal
  WM1_Resistance = readWMsensor();                    // Lee resistencia del Sensor 1
  Serial.print("Resistencia Sensor 1 (Ohms): ");
  Serial.println(WM1_Resistance);

  Serial.println("Seleccionando Sensor 2 (Canal 2)...");
  digitalWrite(MUX_B_PIN, LOW);                     // Selección canal 1 (B=0, A=1)
  digitalWrite(MUX_A_PIN, HIGH);
  delay(10);                                          // Espera tras cambiar canal
  WM2_Resistance = readWMsensor();                    // Lee resistencia del Sensor 2
  Serial.print("Resistencia Sensor 2 (Ohms): ");
  Serial.println(WM2_Resistance);

  digitalWrite(MUX_INH_PIN, HIGH);                    // Deshabilita el multiplexor
  Serial.println("Mux Deshabilitado.");

  WM1_CB = myCBvalue(WM1_Resistance, 24.0, C_FACTOR);  // Convierte a CB el valor del sensor 1
  WM2_CB = myCBvalue(WM2_Resistance, 24.0, C_FACTOR);  // Convierte a CB el valor del sensor 2

  soilStation.resistance1 = WM1_CB;
  soilStation.resistance2 = WM2_CB;
}

// --- Función de lectura de sensor usando corriente alterna simulada, mejorada ---
float readWMsensor() {
  float aRead_A1 = 0;          // Variable para acumular lectura A1
  float aRead_A2 = 0;          // Variable para acumular lectura A2
  float validReadings_A1 = 0;  // Contador de lecturas válidas A1
  float validReadings_A2 = 0;  // Contador de lecturas válidas A2
  float senVWM1 = 0;           // Voltaje calculado A1
  float senVWM2 = 0;           // Voltaje calculado A2
  int adcValue = 0;            // Valor ADC individual

  // Eliminamos posible carga residual en el sensor
  digitalWrite(EXCITE_1_PIN, LOW);
  digitalWrite(EXCITE_2_PIN, LOW);
  delay(50);  // Pequeña espera para descarga

  // Primera dirección
  for (int i = 0; i < NUM_OF_READS; i++) {
    digitalWrite(EXCITE_2_PIN, LOW);                  // Apaga dirección 2
    digitalWrite(EXCITE_1_PIN, HIGH);                 // Enciende dirección 1
    delayMicroseconds(EXCITE_STABILIZE_TIME);         // Espera microsegundos para estabilización
    
    adcValue = analogRead(ANALOG_READ_PIN);           // Lee ADC
    
    // Filtro simple: ignorar valores muy extremos
    if (adcValue > 10 && adcValue < 4080) {
      aRead_A1 += adcValue;
      validReadings_A1++;
    }
    
    digitalWrite(EXCITE_1_PIN, LOW);                  // Apaga dirección 1
    delay(POLARITY_CHANGE_DELAY);                     // Espera para evitar acumulación de carga
  }

  // Segunda dirección
  for (int i = 0; i < NUM_OF_READS; i++) {
    digitalWrite(EXCITE_1_PIN, LOW);                  // Asegura dirección 1 apagada
    digitalWrite(EXCITE_2_PIN, HIGH);                 // Enciende dirección 2
    delayMicroseconds(EXCITE_STABILIZE_TIME);         // Espera breve
    
    adcValue = analogRead(ANALOG_READ_PIN);           // Lee ADC
    
    // Filtro simple: ignorar valores muy extremos
    if (adcValue > 10 && adcValue < 4080) {
      aRead_A2 += adcValue;
      validReadings_A2++;
    }
    
    digitalWrite(EXCITE_2_PIN, LOW);                  // Apaga dirección 2
    delay(POLARITY_CHANGE_DELAY);                     // Espera para evitar acumulación de carga
  }

  // Si no se obtuvieron lecturas válidas, retornar resistencia de circuito abierto
  if (validReadings_A1 == 0 || validReadings_A2 == 0) {
    Serial.println("Error: No se obtuvieron lecturas válidas del sensor.");
    return OPEN_RESISTANCE;
  }

  // Calcula voltaje promedio en ambas direcciones (solo para lecturas válidas)
  senVWM1 = (aRead_A1 / validReadings_A1) * SupplyV / ADC_MAX_VALUE;
  senVWM2 = (aRead_A2 / validReadings_A2) * SupplyV / ADC_MAX_VALUE;

  // Verificación de lecturas anómalas
  if (senVWM1 < 0.05 || senVWM1 > (SupplyV - 0.05) || 
      senVWM2 < 0.05 || senVWM2 > (SupplyV - 0.05)) {
    Serial.println("Advertencia: Voltajes leídos cerca de límites. Posible desconexión o cortocircuito.");
  }

  // Calcula resistencia basada en división de voltaje
  double WM_ResistanceA = 0.0;
  double WM_ResistanceB = 0.0;
  
  // Prevenir divisiones por cero o valores muy pequeños
  if (senVWM1 > 0.05) {
    WM_ResistanceA = Rx * (SupplyV - senVWM1) / senVWM1;
  } else {
    WM_ResistanceA = OPEN_RESISTANCE;
  }
  
  if ((SupplyV - senVWM2) > 0.05) {
    WM_ResistanceB = Rx * senVWM2 / (SupplyV - senVWM2);
  } else {
    WM_ResistanceB = OPEN_RESISTANCE;
  }

  // Detección de valores atípicos entre las dos direcciones
  if (abs(WM_ResistanceA - WM_ResistanceB) > WM_ResistanceA * 0.3) {
    Serial.println("Advertencia: Gran diferencia entre lecturas en ambas direcciones");
  }

  // Promedia ambas resistencias
  double WM_Resistance = (WM_ResistanceA + WM_ResistanceB) / 2.0;

  // Compensación total:
  // - 200Ω (resistencias externas)
  // - 172Ω (2 * 86Ω del multiplexor, ida y vuelta)
  WM_Resistance = WM_Resistance - 200.0 - (2 * MUX_RESISTANCE);

  // Limitar a valores razonables
  if (WM_Resistance > OPEN_RESISTANCE) WM_Resistance = OPEN_RESISTANCE;
  if (WM_Resistance < SHORT_RESISTANCE) WM_Resistance = SHORT_RESISTANCE;

  return WM_Resistance;                                // Retorna resistencia promedio
}

// --- Función para convertir resistencia a tensión (CB/kPa) ---
int myCBvalue(float res, float TC, float cF) {
  int WM_CB;
  float resK = res / 1000.0;                           // Convierte a kilo-ohmios
  float tempD = 1.00 + 0.018 * (TC - 24.00);           // Ajuste por temperatura

  // Para evitar valores inválidos en las ecuaciones
  if (res <= 0) res = OPEN_RESISTANCE;

  if (res > 550.0) {
    if (res > 8000.0) {
      WM_CB = (-2.246 - 5.239 * resK * tempD - 0.06756 * resK * resK * (tempD * tempD)) * cF;
    } else if (res > 1000.0) {
      float denominator = 1 - 0.009733 * resK - 0.01205 * TC;
      // Prevenir división por cero
      if (abs(denominator) < 0.001) denominator = 0.001;
      WM_CB = (-3.213 * resK - 4.093) / denominator * cF;
    } else {
      WM_CB = (resK * 23.156 - 12.736) * tempD;
    }
  } else {
    if (res > 300.0) {
      WM_CB = 0;                                       // Baja humedad = suelo saturado
    } else if (res < 300.0 && res >= SHORT_RESISTANCE) {
      WM_CB = SHORT_CB;                                // Cortocircuito
      Serial.print("Advertencia: Posible cortocircuito en sensor (");
      Serial.print(res);
      Serial.println(" ohms)");
    }
  }

  if (res >= OPEN_RESISTANCE) {
    WM_CB = OPEN_CB;                                   // Circuito abierto
    Serial.println("Advertencia: Posible circuito abierto o sensor no conectado");
  }

  // Asegurar valores positivos y dentro de rango razonable
  WM_CB = abs(WM_CB);
  if (WM_CB > 200 && WM_CB != SHORT_CB && WM_CB != OPEN_CB) {
    WM_CB = 200;  // Limitar a un valor máximo razonable
  }

  return WM_CB;                                        // Retorna valor CB estimado
}