/*
 * Versión 2.0 - Lectura dual de sensores
 * 
 * Mejoras:
 * - Soporte para 2 sensores mediante multiplexor 74HC4052
 * - Selección automática de canales (X0-Y0 y X1-Y1)
 * - Mantiene medición bipolar para precisión
 * - Estructura modular para fácil escalamiento
 */

#define Rx 10000      // Resistencia fija de 10K
#define SupplyV 5     // Voltaje de referencia
#define SENSOR_COUNT 2 // Número de sensores

// Configuración de pines
const int pinA = 4;    // Selector A del multiplexor
const int pinB = 3;    // Selector B del multiplexor
const int pinEnable = 6; // Habilitación del multiplexor (LOW=enable)
const int pinExcitationP = 5;  // Excitación positiva
const int pinExcitationN = 11; // Excitación negativa
const int pinAnalog = A1;      // Entrada analógica

void setup() {
  Serial.begin(9600);
  pinMode(pinA, OUTPUT);
  pinMode(pinB, OUTPUT);
  pinMode(pinEnable, OUTPUT);
  pinMode(pinExcitationP, OUTPUT);
  pinMode(pinExcitationN, OUTPUT);
  digitalWrite(pinEnable, HIGH); // Inicia con multiplexor deshabilitado
}

void loop() {
  for(int sensor = 0; sensor < SENSOR_COUNT; sensor++) {
    float resistencia = medirResistencia(sensor);
    Serial.print("Sensor ");
    Serial.print(sensor + 1);
    Serial.print(" - Resistencia (Ohmios): ");
    Serial.println(resistencia);
  }
  Serial.println("-------------------");
  delay(1000);
}

float medirResistencia(int channel) {
  // Selección de canal (0=X0-Y0, 1=X1-Y1)
  digitalWrite(pinA, channel & 0x01);
  digitalWrite(pinB, (channel >> 1) & 0x01);
  
  digitalWrite(pinEnable, LOW);  // Habilitar multiplexor

  // Medición con polaridad positiva
  digitalWrite(pinExcitationP, HIGH);
  delayMicroseconds(90);
  float V1 = analogRead(pinAnalog) * (SupplyV / 1024.0);
  digitalWrite(pinExcitationP, LOW);

  delay(100); // Pequeña pausa entre mediciones

  // Medición con polaridad negativa
  digitalWrite(pinExcitationN, HIGH);
  delayMicroseconds(90);
  float V2 = analogRead(pinAnalog) * (SupplyV / 1024.0);
  digitalWrite(pinExcitationN, LOW);

  digitalWrite(pinEnable, HIGH); // Deshabilitar multiplexor

  // Cálculo de resistencia (promedio bipolar)
  float R1 = (Rx * (SupplyV - V1)) / V1;
  float R2 = (Rx * V2) / (SupplyV - V2);

  return (R1 + R2) / 2;
}