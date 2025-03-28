/*
Este código se crea a partir del código base de la estación meteorológica
La diferencia es que utiliza un canal serial específico para la estación meteorológica (Serial.begin2)
Se usa para la ESP32 SIM800L LILYGO
*/

char databuffer[35];
double temp;

void getBuffer() {
  int index;
  for (index = 0; index < 35; index++) {
    if (Serial2.available()) {
      databuffer[index] = Serial2.read();
      if (databuffer[0] != 'c') {
        index = -1;
      }
    } else {
      index--;
    }
  }
}

int transCharToInt(char* _buffer, int _start, int _stop) {
  int result = 0;
  for (int _index = _start; _index <= _stop; _index++) {
    result = 10 * result + (_buffer[_index] - '0');
  }
  return result;
}

int transCharToInt_T(char* _buffer) {
  if (_buffer[13] == '-') {
    return -((_buffer[14] - '0') * 10 + (_buffer[15] - '0'));
  }
  return ((_buffer[13] - '0') * 100) + ((_buffer[14] - '0') * 10) + (_buffer[15] - '0');
}

int WindDirection() {
  return transCharToInt(databuffer, 1, 3);
}

float WindSpeedAverage() {
  return 0.44704 * transCharToInt(databuffer, 5, 7);
}

float WindSpeedMax() {
  return 0.44704 * transCharToInt(databuffer, 9, 11);
}

float Temperature() {
  return (transCharToInt_T(databuffer) - 32.00) * 5.00 / 9.00;
}

float RainfallOneHour() {
  return transCharToInt(databuffer, 17, 19) * 25.40 * 0.01;
}

float RainfallOneDay() {
  return transCharToInt(databuffer, 21, 23) * 25.40 * 0.01;
}

int Humidity() {
  return transCharToInt(databuffer, 25, 26);
}

float BarPressure() {
  return transCharToInt(databuffer, 28, 32) / 10.00;
}

void setup() {
  Serial.begin(115200); // Para depuración
  Serial2.begin(9600); // Comunicación con la estación meteorológica usando TX (GPIO 17) y RX (GPIO 16)
}

void loop() {
  getBuffer();
  Serial.print("Wind Direction: "); Serial.println(WindDirection());
  Serial.print("Average Wind Speed (One Minute): "); Serial.print(WindSpeedAverage()); Serial.println(" m/s");
  Serial.print("Max Wind Speed (Five Minutes): "); Serial.print(WindSpeedMax()); Serial.println(" m/s");
  Serial.print("Rain Fall (One Hour): "); Serial.print(RainfallOneHour()); Serial.println(" mm");
  Serial.print("Rain Fall (24 Hour): "); Serial.print(RainfallOneDay()); Serial.println(" mm");
  Serial.print("Temperature: "); Serial.print(Temperature()); Serial.println(" C");
  Serial.print("Humidity: "); Serial.print(Humidity()); Serial.println(" %");
  Serial.print("Barometric Pressure: "); Serial.print(BarPressure()); Serial.println(" hPa");
  Serial.println();
  delay(2000);
}