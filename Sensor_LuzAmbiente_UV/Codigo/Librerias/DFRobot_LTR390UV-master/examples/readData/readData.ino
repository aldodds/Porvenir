/*!
 *@file  readAlsData.ino
 *@brief Run the routine to get ambient light intensity, and change the mode to get UV intensity
 *@n
 * @n connected table
 * ---------------------------------------------------------------------------------------------------------------
 *    board   |             MCU                | Leonardo/Mega2560/M0 |    UNO    | ESP8266 | ESP32 |  microbit  |
 *     VCC    |            3.3V/5V             |        VCC           |    VCC    |   VCC   |  VCC  |     X      |
 *     GND    |              GND               |        GND           |    GND    |   GND   |  GND  |     X      |
 *     RX     |              TX                |     Serial1 TX1      |     5     |   5/D6  |  D2   |     X      |
 *     TX     |              RX                |     Serial1 RX1      |     4     |   4/D7  |  D3   |     X      |
 * ---------------------------------------------------------------------------------------------------------------
 * 
 * @copyright   Copyright (c) 2021 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license     The MIT License (MIT)
 * @author      [TangJie](jie.tang@dfrobot.com)
 * @version     V1.0
 * @date        2021-08-31
 * @url         https://github.com/DFRobor/DFRobot_LTR390UV
 */
#include "DFRobot_LTR390UV.h"

#if defined(ARDUINO_AVR_UNO)||defined(ESP8266)
#include <SoftwareSerial.h>
#endif

#define UARTMODE //Serial mode
//#define I2CMODE //I2C mode
#if defined UARTMODE
#if defined(ARDUINO_AVR_UNO)||defined(ESP8266)
  SoftwareSerial mySerial(/*rx =*/4, /*tx =*/5);
  DFRobot_LTR390UV ltr390(/*addr =*/LTR390UV_DEVICE_ADDR, /*s =*/&mySerial);
#else
  DFRobot_LTR390UV ltr390(/*addr =*/LTR390UV_DEVICE_ADDR, /*s =*/&Serial1);
#endif
#endif
#if defined I2CMODE
DFRobot_LTR390UV ltr390(/*addr = */LTR390UV_DEVICE_ADDR, /*pWire = */&Wire);
#endif

void setup()
{
  
#if defined UARTMODE
  //Init MCU communication serial port
#if defined(ARDUINO_AVR_UNO)||defined(ESP8266)
  mySerial.begin(9600);
#elif defined(ESP32)
  Serial1.begin(9600, SERIAL_8N1, /*rx =*/D3, /*tx =*/D2);
#else
  Serial1.begin(9600);
#endif
#endif
  Serial.begin(115200);
  
  while(ltr390.begin() != 0){
    Serial.println(" Sensor initialize failed!!");
    delay(1000);
  }
  Serial.println(" Sensor  initialize success!!");
  /**
   * @brief Set module to 18 bit data and 100ms acquisition time 
   *  e20bit 20-bit data     e25ms   acquisition time 25ms    e2000ms  acquisition time 2000ms 
   *  e19bit 19-bit data     e50ms   acquisition time 50ms 
   *  e18bit 18-bit data     e100ms  acquisition time 100ms 
   *  e17bit 17-bit data     e200ms  acquisition time 200ms 
   *  e16bit 16-bit data     e500ms  acquisition time 500ms 
   *  e13bit 13-bit data     e1000ms acquisition time 1000ms 
   */
  ltr390.setALSOrUVSMeasRate(ltr390.e18bit,ltr390.e100ms);
  /**
   * @brief Set sensor gain
   *        eGain1 1x
   *        eGain3 3x
   *        eGain6 6x
   *        eGain9 9x
   *        eGain18 18x
   * 
   */
  ltr390.setALSOrUVSGain(ltr390.eGain1);
  ltr390.setMode(ltr390.eALSMode);//Set ambient light mode 
  //ltr390.setMode(ltr390.eUVSMode);//Set UV mode 
}
void loop()
{
  float als = 0;
  uint32_t data = 0;
  data = ltr390.readOriginalData();//Get raw data of ambient light or UV light, determined by the set mode
  Serial.print("data:");
  Serial.println(data);
  als = ltr390.readALSTransformData();//Get data converted from ambient light intensity, which can only be used in ambient light mode
  Serial.print("ALS:");
  Serial.print(als);
  Serial.println("Lux");
  delay(100);
}
