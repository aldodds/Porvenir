# Conexiones

|Arduino | ESP32 | MUX 70HC4052|
|:------:|:------:|:------:|
|PWM 3 |GPIO 18/25/21/26 |B (9)     |
|PWM 4 |GPIO 17/26/22/33 |A (10)    |
|PWM 5 |GPIO 19/14/32    |X (13)    |
|PWM 6 |GPIO 16/13/23/27 |Inh (6)   |
|PWM 11|GPIO 23/27/18/13 |R 10 Kohm |
|A1    |GPIO 34          |Y (3)     |

# Códigos

## Opción 1

```C++
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
const float SupplyV = 3.3;      // Voltaje de referencia (alimentación del sistema)
const float ADC_MAX_VALUE = 4095.0; // Valor máximo del ADC del ESP32 (resolución de 12 bits)
const int   NUM_OF_READS = 5;   // Número de lecturas por dirección (aumentado para mejor precisión)
const long  DEFAULT_TEMP_C = 24; // Temperatura fija asumida (en grados Celsius)
const bool  USE_TEMP_SENSOR = false; // Cambiar a true si se utiliza sensor de temperatura

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
float Temp_Resistance = 0.0;   // Resistencia medida del sensor de temperatura (si está presente)
float Temp_C = DEFAULT_TEMP_C; // Temperatura medida o valor predeterminado
int   WM1_CB = 0;              // Valor CB (kPa) estimado del Sensor 1
int   WM2_CB = 0;              // Valor CB (kPa) estimado del Sensor 2

// --- Prototipos de Funciones ---
float readWMsensor();                       // Función para leer resistencia desde el sensor seleccionado
int   myCBvalue(float res, float TC, float cF);  // Función para convertir resistencia a CB (kPa)
float readTemperature();                    // Función para leer temperatura si hay un sensor conectado
float myTempValue(float tempRes);           // Función para convertir resistencia a temperatura

// --- Configuración inicial ---
void setup() {
  Serial.begin(115200);                               // Inicia comunicación serial a 115200 baudios
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

// --- Bucle principal ---
void loop() {
  Serial.println("\n--- Iniciando ciclo de lectura ---");

  digitalWrite(MUX_INH_PIN, LOW);                     // Habilita el multiplexor
  delay(10);                                          // Breve espera para estabilización

  // Solo leer temperatura si hay sensor configurado
  if (USE_TEMP_SENSOR) {
    Serial.println("Leyendo sensor de temperatura...");
    digitalWrite(MUX_B_PIN, LOW);                     // Selección canal 0 (B=0, A=0) para temperatura
    digitalWrite(MUX_A_PIN, LOW);
    delay(10);                                        // Espera tras cambiar canal
    Temp_Resistance = readWMsensor();                 // Lee resistencia del sensor de temperatura
    Temp_C = myTempValue(Temp_Resistance);            // Convierte resistencia a temperatura
    Serial.print("Resistencia Temp (Ohms): ");
    Serial.println(Temp_Resistance);
    Serial.print("Temperatura (C): ");
    Serial.println(Temp_C);
  } else {
    Serial.print("Usando temperatura predeterminada (C): ");
    Serial.println(DEFAULT_TEMP_C);
    Temp_C = DEFAULT_TEMP_C;
  }

  Serial.println("Seleccionando Sensor 1 (Canal 1)...");
  // Si usamos sensor de temperatura, los sensores WM van en canales 1,2
  // Si no, usamos canales 0,1 para los sensores WM
  if (USE_TEMP_SENSOR) {
    digitalWrite(MUX_B_PIN, LOW);                     // Selección canal 1 (B=0, A=1)
    digitalWrite(MUX_A_PIN, HIGH);
  } else {
    digitalWrite(MUX_B_PIN, LOW);                     // Selección canal 0 (B=0, A=0)
    digitalWrite(MUX_A_PIN, LOW);
  }
  delay(10);                                          // Espera tras cambiar canal
  WM1_Resistance = readWMsensor();                    // Lee resistencia del Sensor 1
  Serial.print("Resistencia Sensor 1 (Ohms): ");
  Serial.println(WM1_Resistance);

  Serial.println("Seleccionando Sensor 2 (Canal 2)...");
  if (USE_TEMP_SENSOR) {
    digitalWrite(MUX_B_PIN, HIGH);                    // Selección canal 2 (B=1, A=0)
    digitalWrite(MUX_A_PIN, LOW);
  } else {
    digitalWrite(MUX_B_PIN, LOW);                     // Selección canal 1 (B=0, A=1)
    digitalWrite(MUX_A_PIN, HIGH);
  }
  delay(10);                                          // Espera tras cambiar canal
  WM2_Resistance = readWMsensor();                    // Lee resistencia del Sensor 2
  Serial.print("Resistencia Sensor 2 (Ohms): ");
  Serial.println(WM2_Resistance);

  digitalWrite(MUX_INH_PIN, HIGH);                    // Deshabilita el multiplexor
  Serial.println("Mux Deshabilitado.");

  WM1_CB = myCBvalue(WM1_Resistance, Temp_C, C_FACTOR);  // Convierte a CB el valor del sensor 1
  WM2_CB = myCBvalue(WM2_Resistance, Temp_C, C_FACTOR);  // Convierte a CB el valor del sensor 2

  Serial.println("\n--- Resultados Finales ---");
  Serial.print("Temperatura (C): ");
  Serial.println(Temp_C);

  Serial.print("Sensor 1: Resistencia=");
  Serial.print(WM1_Resistance);
  Serial.print(" Ohms, Tension=");
  Serial.print(abs(WM1_CB));
  Serial.println(" cb/kPa");

  Serial.print("Sensor 2: Resistencia=");
  Serial.print(WM2_Resistance);
  Serial.print(" Ohms, Tension=");
  Serial.print(abs(WM2_CB));
  Serial.println(" cb/kPa");

  Serial.println("\nEsperando para la proxima lectura...");
  delay(5000);                                        // Espera 5 segundos antes de la siguiente lectura
}

// --- Función para convertir resistencia de temperatura a grados Celsius ---
float myTempValue(float tempRes) {
  float tempC = DEFAULT_TEMP_C;
  
  // Si la resistencia es válida, convertir usando ecuación estándar para sensores de temperatura
  if (tempRes > 0 && tempRes < 30000) {
    tempC = (-23.89 * (log(tempRes))) + 246.00;
  } else {
    Serial.println("Error: Sensor temperatura ausente o valores fuera de rango, usando valor predeterminado");
  }
  
  return tempC;
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

  // Resta la resistencia extra de 200 ohmios introducida por las resistencias en serie del circuito de proteccion
  WM_Resistance = WM_Resistance - 200.0;

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
```

## Opción 2

```C++
// ------------------------------------------------------------
// Código para lectura de sensores de humedad del suelo Watermark 200SS
// usando un multiplexor 74HC4052 y un microcontrolador ESP32 (ESP-WROOM-32).
// Se realiza una lectura pseudo CA (doble dirección) para evitar polarización,
// se calcula la resistencia del sensor mediante un divisor resistivo,
// y se convierte dicha resistencia a centibares (cb o kPa) de tensión del suelo.
// El código está preparado para dos sensores, asumiendo una temperatura constante de 24°C.
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
const float SupplyV = 3.3;      // Voltaje de referencia (alimentación del sistema)
const float ADC_MAX_VALUE = 4095.0; // Valor máximo del ADC del ESP32 (resolución de 12 bits)
const int   NUM_OF_READS = 1;   // Número de lecturas por dirección (puede aumentarse si se desea promediar)
const long  DEFAULT_TEMP_C = 24; // Temperatura fija asumida (en grados Celsius)

// --- Constantes de Calibración y Límites ---
const long  OPEN_RESISTANCE = 35000; // Resistencia que se considera circuito abierto
const long  SHORT_RESISTANCE = 200;  // Resistencia que se considera cortocircuito
const long  SHORT_CB = 240;          // Valor CB fijo si hay cortocircuito
const long  OPEN_CB = 255;           // Valor CB fijo si hay circuito abierto
const float C_FACTOR = 1.1;          // Factor de corrección de calibración

// --- Variables Globales ---
float WM1_Resistance = 0.0;    // Resistencia medida del Sensor 1
float WM2_Resistance = 0.0;    // Resistencia medida del Sensor 2
int   WM1_CB = 0;              // Valor CB (kPa) estimado del Sensor 1
int   WM2_CB = 0;              // Valor CB (kPa) estimado del Sensor 2

// --- Prototipos de Funciones ---
float readWMsensor();                                 // Función para leer resistencia desde el sensor seleccionado
int   myCBvalue(float res, float TC, float cF);       // Función para convertir resistencia a CB (kPa)

// --- Configuración inicial ---
void setup() {
  Serial.begin(115200);                               // Inicia comunicación serial a 115200 baudios
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

// --- Bucle principal ---
void loop() {
  Serial.println("\n--- Iniciando ciclo de lectura ---");

  digitalWrite(MUX_INH_PIN, LOW);                     // Habilita el multiplexor
  delay(1);                                           // Breve espera para estabilización

  Serial.println("Seleccionando Sensor 1 (Canal 0)...");
  digitalWrite(MUX_B_PIN, LOW);                       // Selección canal 0 (B=0, A=0)
  digitalWrite(MUX_A_PIN, LOW);
  delay(10);                                          // Espera tras cambiar canal
  WM1_Resistance = readWMsensor();                    // Lee resistencia del Sensor 1
  Serial.print("Resistencia Sensor 1 (Ohms): ");
  Serial.println(WM1_Resistance);

  Serial.println("Seleccionando Sensor 2 (Canal 1)...");
  digitalWrite(MUX_B_PIN, LOW);                       // Selección canal 1 (B=0, A=1)
  digitalWrite(MUX_A_PIN, HIGH);
  delay(10);                                          // Espera tras cambiar canal
  WM2_Resistance = readWMsensor();                    // Lee resistencia del Sensor 2
  Serial.print("Resistencia Sensor 2 (Ohms): ");
  Serial.println(WM2_Resistance);

  digitalWrite(MUX_INH_PIN, HIGH);                    // Deshabilita el multiplexor
  Serial.println("Mux Deshabilitado.");

  WM1_CB = myCBvalue(WM1_Resistance, DEFAULT_TEMP_C, C_FACTOR);  // Convierte a CB el valor del sensor 1
  WM2_CB = myCBvalue(WM2_Resistance, DEFAULT_TEMP_C, C_FACTOR);  // Convierte a CB el valor del sensor 2

  Serial.println("\n--- Resultados Finales ---");
  Serial.print("Temperatura asumida (C): ");
  Serial.println(DEFAULT_TEMP_C);

  Serial.print("Sensor 1: Resistencia=");
  Serial.print(WM1_Resistance);
  Serial.print(" Ohms, Tension=");
  Serial.print(abs(WM1_CB));
  Serial.println(" cb/kPa");

  Serial.print("Sensor 2: Resistencia=");
  Serial.print(WM2_Resistance);
  Serial.print(" Ohms, Tension=");
  Serial.print(abs(WM2_CB));
  Serial.println(" cb/kPa");

  Serial.println("\nEsperando para la proxima lectura...");
  delay(2000);                                        // Espera 2 segundos antes de la siguiente lectura
}

// --- Función de lectura de sensor usando corriente alterna simulada ---
float readWMsensor() {
  float aRead_A1 = 0;          // Variable para acumular lectura A1
  float aRead_A2 = 0;          // Variable para acumular lectura A2
  float senVWM1 = 0;           // Voltaje calculado A1
  float senVWM2 = 0;           // Voltaje calculado A2

  for (int i = 0; i < NUM_OF_READS; i++) {
    digitalWrite(EXCITE_2_PIN, LOW);                  // Apaga dirección 2
    digitalWrite(EXCITE_1_PIN, HIGH);                 // Enciende dirección 1
    delayMicroseconds(100);                           // Espera microsegundos para estabilización
    aRead_A1 += analogRead(ANALOG_READ_PIN);          // Lee ADC
    digitalWrite(EXCITE_1_PIN, LOW);                  // Apaga dirección 1
    delay(100);                                       // Espera para evitar acumulación de carga

    digitalWrite(EXCITE_1_PIN, LOW);                  // Asegura dirección 1 apagada
    digitalWrite(EXCITE_2_PIN, HIGH);                 // Enciende dirección 2
    delayMicroseconds(100);                           // Espera breve
    aRead_A2 += analogRead(ANALOG_READ_PIN);          // Lee ADC
    digitalWrite(EXCITE_2_PIN, LOW);                  // Apaga dirección 2
  }

  // Calcula voltaje promedio en ambas direcciones
  senVWM1 = (aRead_A1 / ADC_MAX_VALUE) * SupplyV / (float)NUM_OF_READS;
  senVWM2 = (aRead_A2 / ADC_MAX_VALUE) * SupplyV / (float)NUM_OF_READS;

  // Calcula resistencia basada en división de voltaje
  double WM_ResistanceA = (senVWM1 > 0) ? (Rx * (SupplyV - senVWM1) / senVWM1) : OPEN_RESISTANCE;
  double WM_ResistanceB = (SupplyV - senVWM2 != 0) ? (Rx * senVWM2 / (SupplyV - senVWM2)) : OPEN_RESISTANCE;

  // Promedia ambas resistencias
  double WM_Resistance = (WM_ResistanceA + WM_ResistanceB) / 2.0;

  return WM_Resistance;                                // Retorna resistencia promedio
}

// --- Función para convertir resistencia a tensión (CB/kPa) ---
int myCBvalue(float res, float TC, float cF) {
  int WM_CB;
  float resK = res / 1000.0;                           // Convierte a kilo-ohmios
  float tempD = 1.00 + 0.018 * (TC - 24.00);           // Ajuste por temperatura

  if (res > 550.0) {
    if (res > 8000.0) {
      WM_CB = (-2.246 - 5.239 * resK * tempD - 0.06756 * resK * resK * (tempD * tempD)) * cF;
    } else if (res > 1000.0) {
      WM_CB = (-3.213 * resK - 4.093) / (1 - 0.009733 * resK - 0.01205 * TC) * cF;
    } else {
      WM_CB = (resK * 23.156 - 12.736) * tempD;
    }
  } else {
    if (res > 300.0) {
      WM_CB = 0;                                       // Baja humedad = suelo saturado
    } else if (res < 300.0 && res >= SHORT_RESISTANCE) {
      WM_CB = SHORT_CB;                                // Cortocircuito
      Serial.print("Sensor Short WM\n");
    }
  }

  if (res >= OPEN_RESISTANCE || res == 0) {
    WM_CB = OPEN_CB;                                   // Circuito abierto
  }

  return WM_CB;                                        // Retorna valor CB estimado
}
```

## Opción 3

```C++
#include <math.h> // Incluye la biblioteca matemática (aunque no se usa activamente sin el sensor de temperatura)

// --- Definición de Pines ESP32 ---
const int MUX_INH_PIN = 23;  // Pin conectado a Inh/E del 74HC4052 (LOW = Habilitado)
const int MUX_A_PIN   = 22;  // Pin conectado a A (Selector de canal) del 74HC4052
const int MUX_B_PIN   = 21;  // Pin conectado a B (Selector de canal) del 74HC4052
const int EXCITE_1_PIN = 19; // Pin conectado a X (Común) del 74HC4052 (Excitación/Drenaje 1)
const int EXCITE_2_PIN = 18; // Pin conectado a través de Rf=10k a Y (Común) del 74HC4052 (Excitación/Drenaje 2)
const int ANALOG_READ_PIN = 34; // Pin conectado al punto de unión entre Rf=10k y Y (Común) para leer el voltaje

// --- Constantes del Circuito y Sensor ---
const float Rx = 10000.0;          // Resistencia fija (Rf) en Ohmios (10k) - Usar float para cálculos precisos
const float SupplyV = 3.3;         // Voltaje de excitación (¡USAR 3.3V para ESP32 para proteger el ADC!)
const float ADC_MAX_VALUE = 4095.0; // Valor máximo del ADC del ESP32 (12 bits: 2^12 - 1) - Usar float
const int   NUM_OF_READS = 1;      // Número de lecturas a promediar por dirección (1 es suficiente según el ejemplo)
const long  DEFAULT_TEMP_C = 24;   // Temperatura asumida en °C (ya que no hay sensor de temp.)

// --- Constantes de Calibración y Límites (del código de ejemplo) ---
const long  OPEN_RESISTANCE = 35000; // Resistencia aproximada para circuito abierto/sensor desconectado
const long  SHORT_RESISTANCE = 200;  // Resistencia aproximada para corto circuito en terminales del sensor
const long  SHORT_CB = 240;          // Valor Centibar/kPa para indicar corto circuito (Código de error)
const long  OPEN_CB = 255;           // Valor Centibar/kPa para indicar circuito abierto (Código de error)
const float C_FACTOR = 1.1;          // Factor de corrección opcional para ajustar la curva de calibración

// --- Variables Globales ---
float WM1_Resistance = 0.0; // Resistencia calculada para el Sensor 1 (Ohms)
float WM2_Resistance = 0.0; // Resistencia calculada para el Sensor 2 (Ohms)
int   WM1_CB = 0;           // Valor de tensión hídrica para el Sensor 1 (Centibars/kPa)
int   WM2_CB = 0;           // Valor de tensión hídrica para el Sensor 2 (Centibars/kPa)

// --- Prototipos de Funciones ---
float readWMsensor();          // Función para leer la resistencia de un sensor seleccionado en el Mux
int   myCBvalue(float res, float TC, float cF); // Función para convertir Resistencia a Centibars

// --- Configuración Inicial ---
void setup() {
  Serial.begin(115200); // Inicia la comunicación serial a una velocidad mayor (común en ESP32)
  Serial.println("Iniciando Lector de Sensores Watermark con ESP32 y 74HC4052");

  // Configura los pines digitales como SALIDA
  pinMode(MUX_INH_PIN, OUTPUT);  // Pin de habilitación del Mux
  pinMode(MUX_A_PIN, OUTPUT);    // Pin selector A del Mux
  pinMode(MUX_B_PIN, OUTPUT);    // Pin selector B del Mux
  pinMode(EXCITE_1_PIN, OUTPUT); // Pin de excitación/drenaje 1
  pinMode(EXCITE_2_PIN, OUTPUT); // Pin de excitación/drenaje 2

  // Configura el pin analógico (no es estrictamente necesario con analogRead, pero buena práctica)
  // pinMode(ANALOG_READ_PIN, INPUT); // analogRead() lo configura automáticamente

  // Estado inicial seguro: Mux deshabilitado, pines de excitación en BAJO
  digitalWrite(MUX_INH_PIN, HIGH); // Deshabilita el Mux (estado seguro inicial)
  digitalWrite(EXCITE_1_PIN, LOW); // Asegura que el pin de excitación 1 esté apagado
  digitalWrite(EXCITE_2_PIN, LOW); // Asegura que el pin de excitación 2 esté apagado

  delay(100); // Pequeña pausa para asegurar que todo se estabilice
  Serial.println("Setup completado.");
}

// --- Bucle Principal ---
void loop() {
  Serial.println("\n--- Iniciando ciclo de lectura ---");

  // Habilita el Mux para permitir la selección de canal y lectura
  digitalWrite(MUX_INH_PIN, LOW);
  delay(1); // Pequeña espera para que el Mux se active

  // --- Lectura del Sensor 1 (Canal 0: X0/Y0) ---
  Serial.println("Seleccionando Sensor 1 (Canal 0)...");
  // Selecciona el canal 0 en el Mux (B=LOW, A=LOW)
  digitalWrite(MUX_B_PIN, LOW);
  digitalWrite(MUX_A_PIN, LOW);
  delay(10); // Espera a que el Mux conmute y se estabilice (según ejemplo)

  // Llama a la función para leer la resistencia del sensor actualmente seleccionado
  WM1_Resistance = readWMsensor();
  Serial.print("Resistencia Sensor 1 (Ohms): ");
  Serial.println(WM1_Resistance);

  // --- Lectura del Sensor 2 (Canal 1: X1/Y1) ---
  Serial.println("Seleccionando Sensor 2 (Canal 1)...");
  // Selecciona el canal 1 en el Mux (B=LOW, A=HIGH)
  digitalWrite(MUX_B_PIN, LOW);
  digitalWrite(MUX_A_PIN, HIGH);
  delay(10); // Espera a que el Mux conmute y se estabilice

  // Llama a la función para leer la resistencia del sensor actualmente seleccionado
  WM2_Resistance = readWMsensor();
  Serial.print("Resistencia Sensor 2 (Ohms): ");
  Serial.println(WM2_Resistance);

  // Deshabilita el Mux después de leer todos los sensores (ahorra energía y evita lecturas accidentales)
  digitalWrite(MUX_INH_PIN, HIGH);
  Serial.println("Mux Deshabilitado.");

  // --- Cálculos de Tensión Hídrica (Centibars/kPa) ---
  // Convierte las resistencias medidas a valores de tensión hídrica usando la temperatura por defecto
  WM1_CB = myCBvalue(WM1_Resistance, DEFAULT_TEMP_C, C_FACTOR);
  WM2_CB = myCBvalue(WM2_Resistance, DEFAULT_TEMP_C, C_FACTOR);

  // --- Salida de Resultados ---
  Serial.println("\n--- Resultados Finales ---");
  // Muestra la temperatura usada en los cálculos
  Serial.print("Temperatura asumida (C): ");
  Serial.println(DEFAULT_TEMP_C);
  // Muestra la resistencia y tensión para el Sensor 1
  Serial.print("Sensor 1: Resistencia=");
  Serial.print(WM1_Resistance);
  Serial.print(" Ohms, Tension=");
  Serial.print(abs(WM1_CB)); // Usa abs() por si la fórmula da negativo fuera de rango
  Serial.println(" cb/kPa");
  // Muestra la resistencia y tensión para el Sensor 2
  Serial.print("Sensor 2: Resistencia=");
  Serial.print(WM2_Resistance);
  Serial.print(" Ohms, Tension=");
  Serial.print(abs(WM2_CB)); // Usa abs()
  Serial.println(" cb/kPa");

  // --- Espera para el Próximo Ciclo ---
  Serial.println("\nEsperando para la proxima lectura...");
  // Espera 60 segundos (60000 milisegundos) antes de la siguiente lectura. Ajusta según necesidad.
  delay(60000);
}

// --- Función para Leer la Resistencia del Sensor (Pseudo-CA) ---
// Realiza la lectura en ambas polaridades y promedia la resistencia.
// Asume que el canal correcto del Mux ya ha sido seleccionado antes de llamar a esta función.
float readWMsensor() {
  float aRead_A1 = 0; // Acumulador para lecturas ADC en Dirección 1
  float aRead_A2 = 0; // Acumulador para lecturas ADC en Dirección 2
  float senVWM1 = 0;  // Voltaje calculado en Dirección 1
  float senVWM2 = 0;  // Voltaje calculado en Dirección 2

  // El bucle está aquí por si se quiere promediar múltiples lecturas rápidas (NUM_OF_READS > 1)
  for (int i = 0; i < NUM_OF_READS; i++) {
    // --- Dirección 1 de Excitación ---
    // EXCITE_1 (conectado a X) = ALTO (Fuente), EXCITE_2 (conectado a Y vía Rf) = BAJO (Tierra)
    digitalWrite(EXCITE_2_PIN, LOW);  // Asegura que el pin 2 sea tierra primero
    digitalWrite(EXCITE_1_PIN, HIGH); // Aplica voltaje de excitación (3.3V)
    delayMicroseconds(100);           // Espera ~100 microsegundos (según recomendación fabricante 100-200µs)
    aRead_A1 += analogRead(ANALOG_READ_PIN); // Lee el valor ADC y lo suma al acumulador
    digitalWrite(EXCITE_1_PIN, LOW);  // Finaliza la excitación en esta dirección

    // Pausa entre direcciones (según código de ejemplo, aunque no estrictamente en especificación)
    // Podría ayudar a disipar cualquier carga residual mínima.
    delay(100); // 100 milisegundos de pausa

    // --- Dirección 2 de Excitación (Polaridad Inversa) ---
    // EXCITE_1 (conectado a X) = BAJO (Tierra), EXCITE_2 (conectado a Y vía Rf) = ALTO (Fuente)
    digitalWrite(EXCITE_1_PIN, LOW);  // Asegura que el pin 1 sea tierra primero
    digitalWrite(EXCITE_2_PIN, HIGH); // Aplica voltaje de excitación (3.3V) por el otro lado
    delayMicroseconds(100);           // Espera ~100 microsegundos
    aRead_A2 += analogRead(ANALOG_READ_PIN); // Lee el valor ADC y lo suma al acumulador
    digitalWrite(EXCITE_2_PIN, LOW);  // Finaliza la excitación en esta dirección
  }

  // --- Cálculo de Voltajes Promedio ---
  // Convierte la suma de lecturas ADC a voltaje promedio para cada dirección
  // Usa ADC_MAX_VALUE (4095.0) y SupplyV (3.3) para ESP32
  senVWM1 = (aRead_A1 / ADC_MAX_VALUE) * SupplyV / (float)NUM_OF_READS;
  senVWM2 = (aRead_A2 / ADC_MAX_VALUE) * SupplyV / (float)NUM_OF_READS;

  // --- Cálculo de Resistencias ---
  // Calcula la resistencia del sensor para cada dirección usando la fórmula del divisor de voltaje
  // Asegúrate de manejar la división por cero si senVWM1 es 0
  double WM_ResistanceA = (senVWM1 > 0) ? (Rx * (SupplyV - senVWM1) / senVWM1) : OPEN_RESISTANCE; // Evita división por cero
  // Asegúrate de manejar la división por cero si (SupplyV - senVWM2) es 0 ( improbable si SupplyV > 0)
  double WM_ResistanceB = (SupplyV - senVWM2 != 0) ? (Rx * senVWM2 / (SupplyV - senVWM2)) : OPEN_RESISTANCE; // Evita división por cero

  // --- Promedio de Resistencias ---
  // Promedia las resistencias calculadas en ambas direcciones para obtener el valor final
  double WM_Resistance = (WM_ResistanceA + WM_ResistanceB) / 2.0;

  // --- Manejo de Límites (Opcional pero útil) ---
  // Si la resistencia es muy alta o muy baja, podría indicar un problema
   if (WM_Resistance > OPEN_RESISTANCE) {
      WM_Resistance = OPEN_RESISTANCE; // Limita al valor de circuito abierto
   }
   // No se limita el valor bajo aquí, se maneja en myCBvalue

  // Devuelve la resistencia promedio calculada
  return (float)WM_Resistance;
}


// --- Función para Convertir Resistencia a Centibars/kPa ---
// Utiliza la ecuación de calibración modificada de Dr. Shock (1998)
// Incluye compensación por temperatura (usando DEFAULT_TEMP_C aquí)
// Y manejo de casos de corto circuito o circuito abierto.
int myCBvalue(float res, float TC, float cF) {
  int WM_CB;                        // Variable para almacenar el resultado en Centibars
  float resK = res / 1000.0;        // Convierte la resistencia de Ohms a kOhms
  float tempD = 1.0 + 0.018 * (TC - 24.0); // Factor de corrección por temperatura (base 24°C)

  // Verifica si la resistencia está dentro del rango normal de operación del sensor
  if (res > 550.0) { // Rango normal de calibración
    if (res > 8000.0) { // Rango alto (suelo más seco)
      // Fórmula para resistencias > 8 kOhms
      WM_CB = (-2.246 - 5.239 * resK * tempD - 0.06756 * resK * resK * tempD * tempD) * cF;
    } else if (res > 1000.0) { // Rango medio (entre 1 kOhm y 8 kOhms)
      // Fórmula para resistencias entre 1k y 8k Ohms
       // OJO: La fórmula original tenía TC en el denominador, la versión más reciente parece usar tempD implícitamente.
       // Usaremos la estructura de la v1.1 que parece más limpia:
       WM_CB = (-3.213 * resK - 4.093) / (1.0 - 0.009733 * resK - 0.01205 * (TC - 24.0)) * cF ; // Corrección: usar TC-24 como en v1.1
       // Nota: La fórmula parece sensible, asegurarse que es la correcta para el sensor específico.
    } else { // Rango bajo (entre 550 Ohms y 1 kOhm, suelo húmedo)
      // Fórmula para resistencias < 1 kOhm (pero > 550 Ohms)
      WM_CB = (resK * 23.156 - 12.736) * tempD * cF; // Añadido cF aquí también por consistencia
    }
    // Asegurar que el valor no sea negativo (0 cb es el mínimo práctico)
    if (WM_CB < 0) WM_CB = 0;

  } else { // La resistencia está por debajo del rango normal (podría ser un sensor nuevo/húmedo o un corto)
    if (res > 300.0) { // Resistencia muy baja pero no un corto (sensor muy húmedo o nuevo)
      WM_CB = 0; // Asignar 0 cb/kPa
    } else if (res >= SHORT_RESISTANCE) { // Resistencia en el rango de corto circuito
      WM_CB = SHORT_CB; // Asignar código de error para corto (240)
      Serial.println("ADVERTENCIA: Posible corto circuito detectado!");
    } else { // Resistencia extremadamente baja o inválida
       WM_CB = SHORT_CB; // También tratar como corto
       Serial.println("ADVERTENCIA: Lectura de resistencia muy baja!");
    }
  }

  // Verifica si la resistencia es demasiado alta (circuito abierto o sensor desconectado)
  // O si la resistencia es 0 (lo cual es inválido físicamente)
  if (res >= OPEN_RESISTANCE || res <= 0) {
    WM_CB = OPEN_CB; // Asignar código de error para circuito abierto (255)
    if (res <= 0) {
       Serial.println("ADVERTENCIA: Resistencia calculada <= 0!");
    } else {
       Serial.println("ADVERTENCIA: Circuito abierto o sensor desconectado detectado!");
    }
  }

  // Devuelve el valor calculado en Centibars/kPa
  return WM_CB;
}
```

## Opción 4

```C++
#include <math.h>

// Definiciones y constantes
#define NUM_OF_READ 1  // Número de iteraciones de lectura (cada una incluye ambas direcciones)
const int Rx = 10000;  // Resistencia fija de 10KΩ en serie con el sensor
const long DEFAULT_TEMP_C = 24;  // Temperatura por defecto (24°C)
const long OPEN_RESISTANCE = 35000;  // Valor de resistencia para circuito abierto
const long SHORT_RESISTANCE = 200;   // Valor de resistencia para cortocircuito
const long SHORT_CB = 240, OPEN_CB = 255;  // Códigos de error
const int SUPPLY_V = 5;  // Voltaje de alimentación (5V)
const float C_FACTOR = 1.1;  // Factor de corrección para ajustar la curva

// Definición de pines ESP32
const int MUX_ENABLE = 16;    // PWM6 -> GPIO16 (Inh)
const int MUX_SELECT_A = 17;  // PWM4 -> GPIO17 (A)
const int MUX_SELECT_B = 18;  // PWM3 -> GPIO18 (B)
const int EXCITE_POS = 19;    // PWM5 -> GPIO19 (X común)
const int EXCITE_NEG = 23;    // PWM11 -> GPIO23 (Y común)
const int ANALOG_IN = 34;     // A1 -> GPIO34 (Entrada analógica)

// Variables globales
int i, j = 0;
float sensorVoltage1 = 0, sensorVoltage2 = 0;
float analogRead1 = 0, analogRead2 = 0;
float resistance1 = 0, resistance2 = 0;
int sensor1CB = 0, sensor2CB = 0;

void setup() {
  // Inicializar comunicación serial
  Serial.begin(115200);
  
  // Configurar pines como salidas
  pinMode(MUX_ENABLE, OUTPUT);
  pinMode(MUX_SELECT_A, OUTPUT);
  pinMode(MUX_SELECT_B, OUTPUT);
  pinMode(EXCITE_POS, OUTPUT);
  pinMode(EXCITE_NEG, OUTPUT);
  
  // Inicializar pines en estado bajo
  digitalWrite(MUX_ENABLE, HIGH);  // Deshabilitar multiplexor inicialmente
  digitalWrite(EXCITE_POS, LOW);
  digitalWrite(EXCITE_NEG, LOW);
  
  delay(100);  // Espera inicial para estabilización
}

void loop() {
  // Habilitar el multiplexor
  digitalWrite(MUX_ENABLE, LOW);
  delay(10);
  
  // Leer Sensor 1 (canales X0-Y0)
  digitalWrite(MUX_SELECT_A, LOW);
  digitalWrite(MUX_SELECT_B, LOW);
  delay(10);
  resistance1 = readSensor();
  
  // Leer Sensor 2 (canales X1-Y1)
  digitalWrite(MUX_SELECT_A, HIGH);
  digitalWrite(MUX_SELECT_B, LOW);
  delay(10);
  resistance2 = readSensor();
  
  // Deshabilitar el multiplexor
  digitalWrite(MUX_ENABLE, HIGH);
  
  // Convertir resistencias a centibares (kPa)
  sensor1CB = calculateCB(resistance1, DEFAULT_TEMP_C, C_FACTOR);
  sensor2CB = calculateCB(resistance2, DEFAULT_TEMP_C, C_FACTOR);
  
  // Mostrar resultados por serial
  Serial.println("=== Lectura de Sensores ===");
  Serial.print("Sensor 1 Resistencia: "); Serial.print(resistance1); Serial.println(" Ω");
  Serial.print("Sensor 1 (cb/kPa): "); Serial.println(abs(sensor1CB));
  Serial.print("Sensor 2 Resistencia: "); Serial.print(resistance2); Serial.println(" Ω");
  Serial.print("Sensor 2 (cb/kPa): "); Serial.println(abs(sensor2CB));
  Serial.println("==========================");
  
  delay(2000);  // Esperar 30 segundos entre lecturas
}

// Función para leer el sensor con alternancia de polaridad
float readSensor() {
  analogRead1 = 0;
  analogRead2 = 0;
  
  // Realizar múltiples lecturas (definido por NUM_OF_READ)
  for (i = 0; i < NUM_OF_READ; i++) {
    // Primera dirección (polaridad positiva)
    digitalWrite(EXCITE_POS, HIGH);
    delayMicroseconds(100);  // Espera para estabilización (100µs)
    analogRead1 += analogRead(ANALOG_IN);  // Leer valor analógico
    digitalWrite(EXCITE_POS, LOW);
    
    delay(100);  // Pequeña espera entre cambios de polaridad
    
    // Segunda dirección (polaridad negativa)
    digitalWrite(EXCITE_NEG, HIGH);
    delayMicroseconds(100);  // Espera para estabilización (100µs)
    analogRead2 += analogRead(ANALOG_IN);  // Leer valor analógico
    digitalWrite(EXCITE_NEG, LOW);
    
    delay(100);  // Pequeña espera entre iteraciones
  }
  
  // Calcular voltajes promedio
  sensorVoltage1 = ((analogRead1 / 4095.0) * SUPPLY_V) / NUM_OF_READ;  // ESP32 tiene ADC de 12 bits (0-4095)
  sensorVoltage2 = ((analogRead2 / 4095.0) * SUPPLY_V) / NUM_OF_READ;
  
  // Calcular resistencias en ambas direcciones
  double resistanceA = (Rx * (SUPPLY_V - sensorVoltage1) / sensorVoltage1);
  double resistanceB = Rx * sensorVoltage2 / (SUPPLY_V - sensorVoltage2);
  
  // Devolver el promedio de ambas direcciones
  return (resistanceA + resistanceB) / 2.0;
}

// Función para convertir resistencia a centibares (kPa)
int calculateCB(int resistance, float tempC, float correctionFactor) {
  int cbValue;
  float resistanceK = resistance / 1000.0;  // Convertir a kΩ
  float tempFactor = 1.00 + 0.018 * (tempC - 24.00);  // Factor de corrección por temperatura
  
  // Rango normal de calibración
  if (resistance > 550.00) {
    if (resistance > 8000.00) {  // Por encima de 8kΩ
      cbValue = (-2.246 - 5.239 * resistanceK * tempFactor - 0.06756 * resistanceK * resistanceK * (tempFactor * tempFactor)) * correctionFactor;
    } 
    else if (resistance > 1000.00) {  // Entre 1kΩ y 8kΩ
      cbValue = (-3.213 * resistanceK - 4.093) / (1 - 0.009733 * resistanceK - 0.01205 * tempC) * correctionFactor;
    } 
    else {  // Por debajo de 1kΩ
      cbValue = (resistanceK * 23.156 - 12.736) * tempFactor;
    }
  } 
  else {  // Fuera del rango normal
    if (resistance > 300.00) {
      cbValue = 0.00;  // Sensor nuevo no acondicionado
    }
    if (resistance < 300.00 && resistance >= SHORT_RESISTANCE) {
      cbValue = SHORT_CB;  // Código de error para cortocircuito
      Serial.println("Error: Cortocircuito en sensor detectado");
    }
  }
  
  // Detección de circuito abierto
  if (resistance >= OPEN_RESISTANCE || resistance == 0) {
    cbValue = OPEN_CB;  // Código de error para circuito abierto
    Serial.println("Error: Circuito abierto o sensor no presente");
  }
  
  return cbValue;
}
```

## Opción 5

```C++
#include <math.h>

//********************************************************************************************************************
// Adaptación para ESP32 para leer dos sensores Watermark con multiplexor 74HC4052
// Basado en código original de Irrometer Co Inc.
// Adaptado para ESP32 y reducido a dos sensores
//********************************************************************************************************************

// Definición de pines para ESP32
#define PIN_B_SELECTOR 25      // Control de selector B del MUX (equivalente a PWM3 en Arduino)
#define PIN_A_SELECTOR 26      // Control de selector A del MUX (equivalente a PWM4 en Arduino)
#define PIN_X_EXCITATION 14    // Excitación del canal X (equivalente a PWM5 en Arduino)
#define PIN_MUX_ENABLE 13      // Enable/Disable del MUX (equivalente a PWM6 en Arduino)
#define PIN_Y_EXCITATION 27    // Excitación del canal Y (equivalente a PWM11 en Arduino)
#define PIN_ANALOG_READ 34     // Pin analógico para lectura (equivalente a A1 en Arduino)

// Constantes del sistema
#define NUM_OF_READ 1          // Número de iteraciones, cada una incluye dos lecturas del sensor (ambas direcciones)
const int Rx = 10000;          // Resistencia fija en serie con el sensor (10K ohms)
const float default_TempC = 24.0; // Temperatura por defecto en grados Celsius
const long open_resistance = 35000; // Valor de resistencia para circuito abierto
const long short_resistance = 200;  // Valor de resistencia para cortocircuito
const long short_CB = 240;      // Código de falla para cortocircuito del sensor
const long open_CB = 255;       // Código de falla para circuito abierto
const int SupplyV = 5;          // Voltaje de alimentación (5V del ESP32)
const float cFactor = 1.1;      // Factor de corrección recomendado para ajustar la curva

// Variables globales
int i, j = 0;
int WM1_CB = 0, WM2_CB = 0;    // Valores de centibares para cada sensor
float SenVWM1 = 0, SenVWM2 = 0; // Voltajes medidos
float ARead_A1 = 0, ARead_A2 = 0; // Lecturas analógicas
float WM1_Resistance = 0, WM2_Resistance = 0; // Resistencias calculadas

void setup() {
  // Inicializar comunicación serial a 9600 bps
  Serial.begin(9600);
  
  // Configurar pines como salidas
  pinMode(PIN_B_SELECTOR, OUTPUT);   // Selector B del MUX
  pinMode(PIN_A_SELECTOR, OUTPUT);   // Selector A del MUX
  pinMode(PIN_X_EXCITATION, OUTPUT); // Excitación X
  pinMode(PIN_MUX_ENABLE, OUTPUT);   // Enable/Disable MUX
  pinMode(PIN_Y_EXCITATION, OUTPUT); // Excitación Y
  
  // Establecer estado inicial de pines
  digitalWrite(PIN_X_EXCITATION, LOW);
  digitalWrite(PIN_Y_EXCITATION, LOW);
  
  delay(100); // Esperar 100ms para asegurar que las salidas estén configuradas
  
  Serial.println("Sistema de lectura de sensores Watermark inicializado");
  Serial.println("---------------------------------------------------");
}

void loop() {
  // Habilitar el multiplexor
  digitalWrite(PIN_MUX_ENABLE, LOW); // LOW habilita el MUX
  
  // Leer el primer sensor Watermark (canal 0)
  delay(100); // Esperar 100ms antes de leer
  
  // Direccionar el MUX al canal 0 (A=0, B=0)
  digitalWrite(PIN_B_SELECTOR, LOW);
  digitalWrite(PIN_A_SELECTOR, LOW);
  
  delay(10); // Esperar a que el MUX se estabilice
  
  // Leer el sensor en canal 0
  WM1_Resistance = readWMsensor();
  
  // Leer el segundo sensor Watermark (canal 1)
  delay(100); // Esperar 100ms antes de cambiar de canal
  
  // Direccionar el MUX al canal 1 (A=1, B=0)
  digitalWrite(PIN_B_SELECTOR, LOW);
  digitalWrite(PIN_A_SELECTOR, HIGH);
  
  delay(10); // Esperar a que el MUX se estabilice
  
  // Leer el sensor en canal 1
  WM2_Resistance = readWMsensor();
  
  // Deshabilitar el multiplexor para ahorrar energía
  digitalWrite(PIN_MUX_ENABLE, HIGH); // HIGH deshabilita el MUX
  
  // Convertir las resistencias medidas a kPa/centibares de tensión hídrica del suelo
  WM1_CB = myCBvalue(WM1_Resistance, default_TempC, cFactor);
  WM2_CB = myCBvalue(WM2_Resistance, default_TempC, cFactor);
  
  // Mostrar resultados
  Serial.println("Resultados de lecturas:");
  Serial.print("Sensor 1 - Resistencia (Ohms): ");
  Serial.println(WM1_Resistance);
  Serial.print("Sensor 1 - Tensión hídrica (cb/kPa): ");
  Serial.println(abs(WM1_CB));
  Serial.print("Sensor 2 - Resistencia (Ohms): ");
  Serial.println(WM2_Resistance);
  Serial.print("Sensor 2 - Tensión hídrica (cb/kPa): ");
  Serial.println(abs(WM2_CB));
  Serial.println("---------------------------------------------------");
  
  // Esperar antes de la siguiente lectura (3 minutos)
  delay(180000); // 180000ms = 3 minutos
}

// Función para convertir resistencia a centibares/kPa
int myCBvalue(int res, float TC, float cF) {
  int WM_CB;
  float resK = res / 1000.0;
  float tempD = 1.00 + 0.018 * (TC - 24.00);

  if (res > 550.00) { // Si está en el rango normal de calibración
    if (res > 8000.00) { // Por encima de 8k
      WM_CB = (-2.246 - 5.239 * resK * (1 + .018 * (TC - 24.00)) - .06756 * resK * resK * (tempD * tempD)) * cF;
    } else if (res > 1000.00) { // Entre 1k y 8k
      WM_CB = (-3.213 * resK - 4.093) / (1 - 0.009733 * resK - 0.01205 * (TC)) * cF;
    } else { // Por debajo de 1k
      WM_CB = (resK * 23.156 - 12.736) * tempD;
    }
  } else { // Por debajo del rango normal pero por encima de cortocircuito
    if (res > 300.00) {
      WM_CB = 0.00;
    }
    if (res < 300.00 && res >= short_resistance) { // Cortocircuito en el cable
      WM_CB = short_CB; // 240 es un código de falla para cortocircuito de terminal del sensor
      Serial.println("¡ADVERTENCIA! Cortocircuito detectado en sensor");
    }
  }
  if (res >= open_resistance || res == 0) {
    WM_CB = open_CB; // 255 es un código de falla para circuito abierto o sensor no presente
    Serial.println("¡ADVERTENCIA! Circuito abierto o sensor no presente");
  }
  return WM_CB;
}

// Función para leer el sensor y calcular su resistencia
float readWMsensor() {
  ARead_A1 = 0;
  ARead_A2 = 0;

  for (i = 0; i < NUM_OF_READ; i++) // Promedio de múltiples lecturas
  {
    // Primera dirección (polaridad): X excitado
    digitalWrite(PIN_X_EXCITATION, HIGH); // Establecer pin X como voltaje positivo
    delayMicroseconds(90); // Esperar 90 microsegundos y tomar lectura
    ARead_A1 += analogRead(PIN_ANALOG_READ); // Leer el pin analógico y sumar al total
    digitalWrite(PIN_X_EXCITATION, LOW); // Desactivar excitación

    delay(100); // Esperar 100ms antes de invertir polaridad

    // Segunda dirección (polaridad invertida): Y excitado
    digitalWrite(PIN_Y_EXCITATION, HIGH); // Establecer pin Y como voltaje positivo
    delayMicroseconds(90); // Esperar 90 microsegundos y tomar lectura
    ARead_A2 += analogRead(PIN_ANALOG_READ); // Leer el pin analógico y sumar al total
    digitalWrite(PIN_Y_EXCITATION, LOW); // Desactivar excitación
  }

  // Convertir lecturas ADC a voltajes
  // ESP32 tiene ADC de 12 bits (0-4095) a diferencia de Arduino (0-1023)
  SenVWM1 = ((ARead_A1 / 4095.0) * SupplyV) / (NUM_OF_READ); // Promedio de lecturas en primera dirección
  SenVWM2 = ((ARead_A2 / 4095.0) * SupplyV) / (NUM_OF_READ); // Promedio de lecturas en segunda dirección

  // Calcular resistencia en ambas direcciones usando divisor de voltaje
  double WM_ResistanceA = (Rx * (SupplyV - SenVWM1) / SenVWM1); // Fórmula para primera dirección
  double WM_ResistanceB = Rx * SenVWM2 / (SupplyV - SenVWM2);   // Fórmula para segunda dirección (invertida)
  double WM_Resistance = ((WM_ResistanceA + WM_ResistanceB) / 2); // Promedio de ambas direcciones

  return WM_Resistance;
}
```
