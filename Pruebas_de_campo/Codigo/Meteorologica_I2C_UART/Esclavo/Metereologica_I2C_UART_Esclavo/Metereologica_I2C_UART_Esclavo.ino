/*
Este código actúa como el dispositivo esclavo en la comunicación I2C.

Recibe solicitudes del maestro y responde con datos meteorológicos.

Simula la lectura de sensores mediante funciones que devuelven valores.
*/

// Importa la librería para la comunicación I2C
#include <Wire.h>  

// Dirección del esclavo en el bus I2C
#define I2C_SLAVE_ADDR 0x10  

// Pin del LED indicador
#define LED 2  

// Contador de envíos
int count = 0;  

// Estructura para almacenar los datos meteorológicos
typedef struct {
    uint16_t count;
    uint16_t wind_direction;
    float avg_wind_speed;
    float max_wind_speed;
    float rain_fall_1h;
    float rain_fall_24h;
    float temperature;
    float humidity;
    float pressure;
} WeatherData;

// Variable para almacenar los datos a enviar
WeatherData weatherData;  

// Función para actualizar los datos meteorológicos
void updateWeatherData() {
    getBuffer();  // Lee datos de sensores (simulado)

    // Asigna valores a la estructura con datos de sensores
    weatherData.count = count;
    weatherData.wind_direction = WindDirection();
    weatherData.avg_wind_speed = WindSpeedAverage();
    weatherData.max_wind_speed = WindSpeedMax();
    weatherData.rain_fall_1h = RainfallOneHour();
    weatherData.rain_fall_24h = RainfallOneDay();
    weatherData.temperature = Temperature();
    weatherData.humidity = Humidity();
    weatherData.pressure = BarPressure();
    count++;  // Incrementa el contador

    // Muestra los datos en el monitor serie
    Serial.println("ESP1 - Datos actualizados:");
    Serial.print("Count: "); Serial.println(weatherData.count);
    Serial.print("Wind Direction: "); Serial.println(weatherData.wind_direction);
    Serial.print("Average Wind Speed: "); Serial.println(weatherData.avg_wind_speed);
    Serial.print("Max Wind Speed: "); Serial.println(weatherData.max_wind_speed);
    Serial.print("Rain Fall (1 Hour): "); Serial.println(weatherData.rain_fall_1h);
    Serial.print("Rain Fall (24 Hours): "); Serial.println(weatherData.rain_fall_24h);
    Serial.print("Temperature: "); Serial.println(weatherData.temperature);
    Serial.print("Humidity: "); Serial.println(weatherData.humidity);
    Serial.print("Barometric Pressure: "); Serial.println(weatherData.pressure);
    Serial.println("----------------------------");
}

// Función para enviar datos al maestro
void sendWeatherData() {
    updateWeatherData();  // Actualiza los datos antes de enviarlos
    Wire.write((uint8_t*)&weatherData, sizeof(weatherData));  // Envía los datos por I2C
}

// Configuración inicial del esclavo
void setup() {
    Serial.begin(9600);  // Inicia la comunicación serie
    Wire.begin(I2C_SLAVE_ADDR);  // Configura el esclavo I2C con su dirección
    Wire.onRequest(sendWeatherData);  // Define la función que responde al maestro

    pinMode(LED, OUTPUT);  // Configura el LED como salida
    digitalWrite(LED, LOW);  // Apaga el LED inicialmente
}

// Bucle principal
void loop() {
    delay(1000);  // Espera para estabilidad
}