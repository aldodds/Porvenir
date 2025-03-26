# MCU
MCU: LTR390-UV-01

# Tener en cuenta
* Soporta I2C y UART
* 13 a 20 *bits* de resolución efectiva
* Rango de ganancia ajustable: 1, 3, 6, 9 y 18
* Dirección I2C: **0X1C**
* Descargar la libreria: **DFRobot_LTR390UV.h**

# Conexiones de la tarjeta

| Etiqueta| Nombre   |Descripción|
|:-----------|:--------|:--------:
| D/T   | SDA/TXD    |I2C Línea de dato / UART Transmision de datos
| C/R   | SCL/RXD    |I2C Línea de reloj / UART Recepción de datos
| -   |    |Energía -
| +   |     |Energía + (DC 3.3 - 5 V)

# Respuesta
* Longitud de onda de respuesta UV: 280nm a 430nm
* Longitud de onda de respuesta espectral de ALS: 450nm a 700nm
* Respuesta digital

# Rango de Referencia de Luz Ambiental

| Lugar/Entorno | Iluminancia |
|--------------|-------------|
| Día soleado | 30000-300000 lux |
| En el interior en un día soleado | 100-1000 lux |
| Día nublado | 3000-10000 lux |
| Al aire libre en un día nublado | 50-500 lux |
| En el interior en un día nublado | 5-50 lux |
| En el interior al anochecer | 10 lux |
| Al amanecer y al atardecer | 300 lux |
| De noche | 0.001-0.02 lux |
| Tarde de luna llena | 0.02-0.3 lux |
| Oficina sin luz | 30-50 lux |
| Exposición a luz de linterna | 8000-15000 lux |


# Ejemplo de códigos

## I2C

### Luz ambiental

```c
#include "DFRobot_LTR390UV.h"
DFRobot_LTR390UV ltr390(/*addr = */LTR390UV_DEVICE_ADDR, /*pWire = */&Wire);

void setup()
{
  Serial.begin(115200);
  while(ltr390.begin() != 0){
    Serial.println(" Error al inicializar el sensor!!");
    delay(1000);
  }
  Serial.println(" El sensor se inicializó correctamente!!");
  ltr390.setALSOrUVSMeasRate(ltr390.e18bit,ltr390.e100ms);//Datos de 18 bits, tiempo de muestreo de 100 ms 
  ltr390.setALSOrUVSGain(ltr390.eGain3);//Ganancia 3
  ltr390.setMode(ltr390.eALSMode);//Establecer modo de luz ambiental
}
void loop()
{
  float als = 0;
  als = ltr390.readALSTransformData();//Obtener datos convertidos de intensidad de luz ambiental, que solo se pueden usar en modo de luz ambiental
  Serial.print("ALS:");
  Serial.print(als);
  Serial.println("Lux");
  delay(1000);
}
```

### UV

```c
#include "DFRobot_LTR390UV.h"
DFRobot_LTR390UV ltr390(/*addr = */LTR390UV_DEVICE_ADDR, /*pWire = */&Wire);

void setup()
{
  Serial.begin(115200);
  while(ltr390.begin() != 0){
    Serial.println(" Error al inicializar el sensor!!");
    delay(1000);
  }
  Serial.println(" El sensor se inicializó correctamente!!");
  ltr390.setALSOrUVSMeasRate(ltr390.e18bit,ltr390.e100ms);//Datos de 18 bits, tiempo de muestreo de 100 ms 
  ltr390.setALSOrUVSGain(ltr390.eGain3);//Ganancia de 3
  ltr390.setMode(ltr390.eUVSMode);//Establece el modo UV
}
void loop()
{
  uint32_t data = 0;
  data = ltr390.readOriginalData();//Obtener datos sin procesar de UV
  Serial.print("data:");
  Serial.println(data);
  delay(1000);
}
```

## UART

### Luz ambiental

```c
#include "DFRobot_LTR390UV.h"
#include <SoftwareSerial.h>
#define UARTMODE //UART mode

SoftwareSerial mySerial(/*rx =*/4, /*tx =*/5);
DFRobot_LTR390UV ltr390(/*addr =*/LTR390UV_DEVICE_ADDR, /*s =*/&mySerial);

void setup()
{
  #define UARTMODE
  mySerial.begin(9600);
  Serial.begin(115200);
  while(ltr390.begin() != 0){
    Serial.println(" Error al inicializar el sensor!!");
    delay(1000);
  }
  Serial.println(" El sensor se inicializó correctamente!!");
  ltr390.setALSOrUVSMeasRate(ltr390.e18bit,ltr390.e100ms);//Datos de 18 bits, tiempo de muestreo de 100 ms
  ltr390.setALSOrUVSGain(ltr390.eGain3);//Ganancia de 3
  ltr390.setMode(ltr390.eALSMode);//Establece el modo de luz ambiental
}

void loop()
{
  float als = 0;
  als = ltr390.readALSTransformData();//Obtener los datos convertidos de la intensidad de la luz ambiental, que solo se pueden usar en el modo de luz ambiental.
  Serial.print("ALS:");
  Serial.print(als);
  Serial.println("Lux");
  delay(100);
}
```

### UV

```c
#include "DFRobot_LTR390UV.h"
#include <SoftwareSerial.h>
#define UARTMODE //UART mode

SoftwareSerial mySerial(/*rx =*/4, /*tx =*/5);
DFRobot_LTR390UV ltr390(/*addr =*/LTR390UV_DEVICE_ADDR, /*s =*/&mySerial);

void setup()
{
  #define UARTMODE
  mySerial.begin(9600);
  Serial.begin(115200);
  while(ltr390.begin() != 0){
    Serial.println(" Error al inicializar el sensor!!");
    delay(1000);
  }
  Serial.println(" El sensor se inicializó correctamente!!");
  ltr390.setALSOrUVSMeasRate(ltr390.e18bit,ltr390.e100ms);//Datos de 18 bits, tiempo de muestreo de 100 ms
  ltr390.setALSOrUVSGain(ltr390.eGain3);//Ganancia de 3
  ltr390.setMode(ltr390.eUVSMode);//Establece el modo UV 
}

void loop()
{
  uint32_t data = 0;
  data = ltr390.readOriginalData();//Obtener datos sin procesar de UV
  Serial.print("data:");
  Serial.println(data);
  delay(100);
}
```