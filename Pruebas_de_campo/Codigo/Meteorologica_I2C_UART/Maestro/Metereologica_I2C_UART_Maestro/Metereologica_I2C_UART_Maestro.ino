/*
Este código actúa como el dispositivo maestro en una comunicación I2C con un esclavo que proporciona datos meteorológicos.

Usa la librería Wire.h para la comunicación I2C.

Emplea un LCD I2C para mostrar datos.

Utiliza un sensor UV DFRobot_LTR390UV.

Solicita datos al esclavo, los muestra en el monitor serie y en el LCD.
*/

// Importa las librerías necesarias
#include <Wire.h>  // Comunicación I2C
#include <LiquidCrystal_I2C.h>  // Manejo del LCD I2C
#include "DFRobot_LTR390UV.h"  // Sensor UV

// Dirección del esclavo en el bus I2C
#define I2C_SLAVE_ADDR 0x10  

// Pin del LED indicador
#define LED 13  

// Contador de solicitudes
int count = 0;  

// Inicialización del LCD con dirección I2C 0x27, 16 columnas y 2 filas
LiquidCrystal_I2C lcd(0x27, 16, 2);  

// Inicialización del sensor UV
DFRobot_LTR390UV ltr390(LTR390UV_DEVICE_ADDR, &Wire);  

// Estructura para almacenar datos meteorológicos recibidos del esclavo
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

// Variable para almacenar los datos recibidos
WeatherData receivedData;  

// Función para solicitar datos al esclavo I2C
void requestWeatherData() {
    Wire.requestFrom(I2C_SLAVE_ADDR, sizeof(receivedData));  // Solicita datos al esclavo
    if (Wire.available() == sizeof(receivedData)) {  // Verifica si los datos están disponibles
        Wire.readBytes((uint8_t*)&receivedData, sizeof(receivedData));  // Lee los datos en la estructura
        digitalWrite(LED, HIGH);  // Enciende el LED como indicador
    }
}

// Configuración inicial del sistema
void setup() {
    Serial.begin(115200);  // Inicia la comunicación serie
    Wire.begin();  // Inicia la comunicación I2C

    pinMode(LED, OUTPUT);  // Configura el pin del LED como salida
    digitalWrite(LED, LOW);  // Apaga el LED inicialmente

    lcd.init();  // Inicializa el LCD
    lcd.backlight();  // Enciende la retroiluminación del LCD
    lcd.setCursor(0, 0);  // Posiciona el cursor en la primera fila
    lcd.print("Hola Mundo!");  // Mensaje inicial en el LCD

    // Inicializa el sensor UV
    while (ltr390.begin() != 0) {
        Serial.println("Error al inicializar el sensor!!");  
        delay(1000);  // Espera y reintenta si hay error
    }
    Serial.println("El sensor se inicializó correctamente!!");

    // Configuración del sensor UV
    ltr390.setALSOrUVSMeasRate(ltr390.e18bit, ltr390.e100ms);  // Resolución 18 bits, muestreo 100 ms
    ltr390.setALSOrUVSGain(ltr390.eGain3);  // Ganancia de 3
}

// Bucle principal
void loop() {
    requestWeatherData();  // Solicita datos al esclavo

    // Si los datos son válidos, los muestra en el monitor serie y el LCD
    if (!isnan(receivedData.temperature)) {
        Serial.println("Count: " + String(receivedData.count));
        Serial.print("Wind Direction: "); Serial.println(receivedData.wind_direction);
        Serial.print("Average Wind Speed: "); Serial.println(receivedData.avg_wind_speed);
        Serial.print("Max Wind Speed: "); Serial.println(receivedData.max_wind_speed);
        Serial.print("Rain Fall (1 Hour): "); Serial.println(receivedData.rain_fall_1h);
        Serial.print("Rain Fall (24 Hours): "); Serial.println(receivedData.rain_fall_24h);
        Serial.print("Temperature: "); Serial.println(receivedData.temperature);
        Serial.print("Humidity: "); Serial.println(receivedData.humidity);
        Serial.print("Barometric Pressure: "); Serial.println(receivedData.pressure);
        Serial.println("----------------------------");  

        lcd.clear();  // Limpia el LCD
        lcd.setCursor(0, 0);  // Posiciona el cursor
        lcd.print("Temp: ");
        lcd.setCursor(7, 0);
        lcd.print(receivedData.temperature);
    }

    ltr390.setMode(ltr390.eUVSMode);  // Activa el modo UV en el sensor
    delay(100);  // Espera para asegurar la lectura

    uint32_t uv = ltr390.readOriginalData();  // Obtiene el índice UV
    Serial.print("Índice UV: ");
    Serial.println(uv);

    // Muestra el índice UV en el LCD
    lcd.setCursor(0, 1);
    lcd.print("UV: ");
    lcd.setCursor(5, 1);
    lcd.print(uv);

    digitalWrite(LED, LOW);  // Apaga el LED
    delay(10000);  // Espera 10 segundos antes de la siguiente solicitud
}