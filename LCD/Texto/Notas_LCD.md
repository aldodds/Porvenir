# Tener en cuenta
* Voltaje de operación: **5 V**
* Módulo: LCD I2C con PCF8574T
* Instalar la biblioteca `LiquidCrystal_I2C` desde el administrador de Bibliotecas de Arduino. Se tiene la versión 1.0.7.
* Instalar la librería `LiquidCrystal_PCF8574` by Matthias Hertel desde el administrador de Bibliotecas de Arduino. Se tiene la versión 2.2.0
* Dirección I2C: **0x27**
* La retroilumincación se enciende por medio de un puente *jumper*
* Se puede cambiar la dirección I2C del módulo configurando los terminales A0, A1 y A2 mediante puentes. Hay 8 combinaciones posibles, de 0x20 a 0x27. Por defecto, la dirección es 0x27, ya que no hay puentes y los terminales están en estado lógico alto.

# Principales métodos disponibles

* `lcd.clear()`: Limpia la pantalla.
* `lcd.setCursor(col, row)`: Posiciona el cursor en columna (col) y fila (row).
* `lcd.print("texto")`: Muestra texto en la posición actual.
* `lcd.backlight() / lcd.noBacklight()`: Controla la retroiluminación
* `lcd.blink() / lcd.noBlink()`: Activa/desactiva el parpadeo del cursor.
* `lcd.cursor() / lcd.noCursor()`: Activa/desactiva el cursor subrayado.

# Conexiones para ESP32 WROOM-32

| ESP32 | FC-113 |
|:-------------:|:--------------:|
| GND    | GND   |
| 3.3 V  | VCC   |
| GPIO21 | SDA   |
| GPIO22 | SCL   |

La ESP32 WROOM-32 (devkit) ofrece 3.3 V, por lo que los caracteres de la LCD no visualizan de forma clara, por lo tanto se debe conectar VCC y GND a una fuente externa de 5 V.

# Código básico

```c
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Configuración del LCD: Dirección I2C, Columnas, Filas
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  lcd.init();          // Inicializa el LCD
  lcd.backlight();     // Enciende la retroiluminación
  lcd.setCursor(0, 0); // Coloca el cursor en la posición inicial
  lcd.print("Hola Mundo!"); // Muestra un mensaje
}

void loop() {
  // Código adicional aquí
}
```
# Aviso del IDE de Arduino

Durante la compilación (upload) del código de Arduino con la ESP32, el IDE mencionó lo siguiente en la terminal: **WARNING: library LiquidCrystal I2C claims to run on avr architecture(s) and may be incompatible with your current board which runs on esp32 architecture(s).**

Por lo anterior, se debe instalar la librería *LiquidCrystal_PCF8574* by Matthias Hertel. Para este caso se tiene la versión 2.2.0

## Códigos para evitar el error

Se probaron los sigiuentes códigos para evitar la advertencia anterior.

### Primer código

```c++
#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>

// Configuración del LCD: Dirección I2C
LiquidCrystal_PCF8574 lcd(0x27);

void setup() {
  lcd.begin(16, 2);         // Inicializa el LCD con 16 columnas y 2 filas
  lcd.setBacklight(255);    // Ajusta la retroiluminación (0-255)
  lcd.setCursor(0, 0);      // Coloca el cursor en la posición inicial
  lcd.print("Hola Mundo!"); // Muestra un mensaje
}

void loop() {
  // Código adicional aquí
}
```

### Segundo código

```c++
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

// Para ESP32, necesitas especificar los pines I2C
#define SDA_PIN 21  // Pin SDA por defecto en ESP32
#define SCL_PIN 22  // Pin SCL por defecto en ESP32

LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  Wire.begin(SDA_PIN, SCL_PIN); // Inicializar I2C con los pines específicos
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Hola Mundo!");
}

void loop() {
  // Código adicional aquí
}
```
