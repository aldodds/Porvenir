/*
 * Documentación del código
 * V.1.0
 * 
 * Descripción general:
 * Este programa permite simular un sensor Watermark 200SS utilizando un potenciómetro y un 
 * multiplexor 74HC4052 en Arduino UNO. Se mide la resistencia del potenciómetro, simulando 
 * la variación de resistencia del sensor real.
 * 
 * Funcionamiento:
 * 1. Se habilita el multiplexor y se selecciona el canal X0-Y0.
 * 2. Se aplica excitación en ambas direcciones con los pines PWM5 y PWM11.
 * 3. Se mide el voltaje en la entrada analógica A1.
 * 4. Se calcula la resistencia usando la fórmula del divisor de voltaje.
 * 5. Se promedian las mediciones en ambas direcciones para mayor precisión.
 * 
 * Basado en {Potencial_matrico.pdf}:
 * - Excitación en ambas direcciones:
 *   - Se usaron PWM5 y PWM11 para alternar la polaridad, como en los ejemplos del archivo.
 *   - Se aplicó un pulso corto (90 µs) antes de la lectura, siguiendo las recomendaciones.
 * 
 * - Uso del multiplexor 74HC4052:
 *   - Se respetó la configuración de A y B para seleccionar el canal X0-Y0.
 *   - Se activó y desactivó el multiplexor con PWM6, como en los códigos del archivo.
 * 
 * - Medición de resistencia:
 *   - Se aplicó la ecuación del divisor de voltaje usada en el archivo.
 *   - Se promediaron las lecturas en ambas direcciones para mayor precisión.
 * 
 * El código mantiene la lógica original de los ejemplos del archivo, adaptado para la 
 * simulación con un potenciómetro en lugar del sensor Watermark 200SS.
 */


#define Rx 10000   // Resistencia fija de 10K en serie con el potenciómetro
#define SupplyV 5  // Voltaje de referencia (5V en Arduino)

void setup() {
  Serial.begin(9600);
  pinMode(3, OUTPUT);  // Selector A del multiplexor
  pinMode(4, OUTPUT);  // Selector B del multiplexor
  pinMode(5, OUTPUT);  // Excitación positiva
  pinMode(6, OUTPUT);  // Habilitación del multiplexor
  pinMode(11, OUTPUT); // Excitación inversa
}

void loop() {
  float resistencia = medirResistencia();
  Serial.print("Resistencia simulada (Ohmios): ");
  Serial.println(resistencia);
  delay(1000);
}

float medirResistencia() {
  digitalWrite(6, LOW);  // Habilitar multiplexor

  // Primera medición con excitación en PWM5
  digitalWrite(5, HIGH); 
  delayMicroseconds(90); 
  float V1 = analogRead(A1) * (SupplyV / 1024.0); // Leer voltaje
  digitalWrite(5, LOW); 

  delay(100); 

  // Segunda medición con excitación en PWM11 (polaridad inversa)
  digitalWrite(11, HIGH);
  delayMicroseconds(90);
  float V2 = analogRead(A1) * (SupplyV / 1024.0); // Leer voltaje
  digitalWrite(11, LOW);

  digitalWrite(6, HIGH); // Deshabilitar multiplexor

  // Cálculo de resistencia en ambas direcciones
  float R1 = (Rx * (SupplyV - V1)) / V1;
  float R2 = (Rx * V2) / (SupplyV - V2);

  return (R1 + R2) / 2; // Promedio de ambas mediciones
}