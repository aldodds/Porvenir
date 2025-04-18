/*
  Este código configura una UART secundaria (UART2) en la ESP32 SIM800L T-Call V1.3
  para comunicarse con una estación meteorológica que transmite datos por protocolo UART.
  Se evita conflicto con el módulo SIM800L (que usa GPIO26 y GPIO27) al redirigir la UART2
  a los pines GPIO32 (RX) y GPIO33 (TX), permitiendo la lectura continua de datos meteorológicos.
*/
char databuffer[35]; // Arreglo que almacenará los datos recibidos del sensor
double temp;         // Variable auxiliar para la temperatura

// Función para llenar el buffer con 35 caracteres, asegurando que inicie con 'c'
void getBuffer() {
  int index;
  for (index = 0; index < 35; index++) {
    if (Serial2.available()) {           // Si hay datos disponibles
      databuffer[index] = Serial2.read(); // Leer un byte y guardarlo en el buffer
      if (databuffer[0] != 'c') {             // Verifica si el primer carácter es 'c'
        index = -1;                            // Si no, reinicia el conteo
      }
    } else {
      index--;                                 // Si no hay datos, espera y repite
    }
  }
}

// Convierte caracteres del buffer a entero entre posiciones _start y _stop
int transCharToInt(char* _buffer, int _start, int _stop) {
  int result = 0;
  for (int _index = _start; _index <= _stop; _index++) {
    result = 10 * result + (_buffer[_index] - '0');
  }
  return result;
}

// Convierte los caracteres de temperatura a entero considerando signo negativo
int transCharToInt_T(char* _buffer) {
  if (_buffer[13] == '-') {
    return -((_buffer[14] - '0') * 10 + (_buffer[15] - '0'));
  }
  return ((_buffer[13] - '0') * 100) + ((_buffer[14] - '0') * 10) + (_buffer[15] - '0');
}

// Dirección del viento en grados
int WindDirection() {
  return transCharToInt(databuffer, 1, 3);
}

// Velocidad media del viento en m/s
float WindSpeedAverage() {
  return 0.44704 * transCharToInt(databuffer, 5, 7);
}

// Velocidad máxima del viento en m/s
float WindSpeedMax() {
  return 0.44704 * transCharToInt(databuffer, 9, 11);
}

// Temperatura en grados Celsius
float Temperature() {
  return (transCharToInt_T(databuffer) - 32.00) * 5.00 / 9.00;
}

// Precipitación en la última hora en mm
float RainfallOneHour() {
  return transCharToInt(databuffer, 17, 19) * 25.40 * 0.01;
}

// Precipitación en las últimas 24 horas en mm
float RainfallOneDay() {
  return transCharToInt(databuffer, 21, 23) * 25.40 * 0.01;
}

// Humedad relativa en porcentaje
int Humidity() {
  return transCharToInt(databuffer, 25, 26);
}

// Presión barométrica en hPa
float BarPressure() {
  return transCharToInt(databuffer, 28, 32) / 10.00;
}

void setup() {
  Serial.begin(115200); // Inicializa puerto serial para depuración (USB)
  Serial2.begin(9600, SERIAL_8N1, 32, 33); // UART2 en GPIO32 (RX) y GPIO33 (TX) a 9600 baudios
}

void loop() {
  getBuffer(); // Llama a la función para llenar el buffer desde el sensor

  // Imprime los datos meteorológicos al monitor serie
  Serial.print("Wind Direction: "); Serial.println(WindDirection());
  Serial.print("Average Wind Speed (One Minute): "); Serial.print(WindSpeedAverage()); Serial.println(" m/s");
  Serial.print("Max Wind Speed (Five Minutes): "); Serial.print(WindSpeedMax()); Serial.println(" m/s");
  Serial.print("Rain Fall (One Hour): "); Serial.print(RainfallOneHour()); Serial.println(" mm");
  Serial.print("Rain Fall (24 Hour): "); Serial.print(RainfallOneDay()); Serial.println(" mm");
  Serial.print("Temperature: "); Serial.print(Temperature()); Serial.println(" C");
  Serial.print("Humidity: "); Serial.print(Humidity()); Serial.println(" %");
  Serial.print("Barometric Pressure: "); Serial.print(BarPressure()); Serial.println(" hPa");
  Serial.println();

  delay(2000); // Espera 2 segundos antes de la próxima lectura
}
