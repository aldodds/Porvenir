DFRobot_LTR390UV
===========================

* [English Version](./README.md)

SEN0540是一个可以作为环境光获取或紫外线获取得传感器，能过使用用户库方便快捷得使用传感器。

![产品效果图片](../../resources/images/SEN0540.png)

## 产品链接（https://www.dfrobot.com）

    SKU：SEN0540
  
## 目录

  * [概述](#概述)
  * [库安装](#库安装)
  * [方法](#方法)
  * [兼容性](#兼容性)
  * [历史](#历史)
  * [创作者](#创作者)

## 概述

SEN0540是一个可以作为环境光获取或紫外线获取得传感器，能过使用用户库方便快捷得使用传感器。

## 库安装

使用此库前，请首先下载库文件，将其粘贴到\Arduino\libraries目录中，然后打开examples文件夹并在该文件夹中运行演示。

## 方法

```C++
  /**
   * @fn begin
   * @brief Init SEN0540 device
   * @return Return value init status
   * @retval 0  Succeed
   * @retval -1 Failed
   */
  int8_t begin(void);

  /**
   * @fn setMode
   * @brief 设置模块采集数据模式
   * @param mode 采集数据选择
   * @return NONE
   */
  void setMode(eModel_t mode);

  /**
   * @fn setALSOrUVSMeasRate
   * @brief 设置模块采集数据位数和采集时间，采集时间必须大于采集位数所需时间
   * @n --------------------------------------------------------------------------------------------------------
   * @n |    bit7    |    bit6    |    bit5    |    bit4    |    bit3    |    bit2    |    bit1    |    bit0    |
   * @n ---------------------------------------------------------------------------------------------------------
   * @n |  Reserved  |        ALS/UVS Resolution            |  Reserved  |   ALS/UVS Measurement Rate           |
   * @n ---------------------------------------------------------------------------------------------------------
   * @n | ALS/UVS Resolution       |000|20 Bit, Conversion time = 400ms                                         |
   * @n |                          |001|19 Bit, Conversion time = 200ms                                         |
   * @n |                          |010|18 Bit, Conversion time = 100ms(default)                                |
   * @n |                          |011|17 Bit, Conversion time = 50ms                                          |
   * @n |                          |100|16 Bit, Conversion time = 25ms                                          |
   * @n |                          |110/111|Reserved                                                            |
   * @n ---------------------------------------------------------------------------------------------------------
   * @n | ALS/UVS Measurement Rate |000|25ms                                                                    |
   * @n |                          |001|50ms                                                                    |
   * @n |                          |010|100ms (default)                                                         |
   * @n |                          |011|200ms                                                                   |
   * @n |                          |100|500ms                                                                   |
   * @n |                          |101|1000ms                                                                  |
   * @n |                          |110/111|2000ms                                                              |
   * @n ---------------------------------------------------------------------------------------------------------
   * @param bit 设置数据位数
   * @param time 设置采样时间
   * @return None
   */
 void setALSOrUVSMeasRate(eResolution bit,eMeasurementRate time);

  /**
   * @fn setALSOrUVSGain
   * @brief 设置传感器增益调节
   * @n ---------------------------------------------------------------------------------------------------------
   * @n |    bit7    |    bit6    |    bit5    |    bit4    |    bit3    |    bit2    |    bit1    |    bit0    |
   * @n ---------------------------------------------------------------------------------------------------------
   * @n |                                    Reserved                    |          ALS/UVS Gain Range          |
   * @n ---------------------------------------------------------------------------------------------------------
   * @n | ALS/UVS Gain Range       |000|Gain Range: 1                                                           |
   * @n |                          |001|Gain Range: 3 (default)                                                 |
   * @n |                          |010|Gain Range: 6                                                           |
   * @n |                          |011|Gain Range: 9                                                           |
   * @n |                          |100|Gain Range: 18                                                          |
   * @n |                          |110/111|Reserved                                                            |
   * @n ---------------------------------------------------------------------------------------------------------                  
   * @param data 控制数据 
   * @return None
   */
  void setALSOrUVSGain(eGainRange data);


  /**
   * @fn readData
   * @brief 获取原始数据
   * @return 返回获取得原始数据
   */
  uint32_t readOriginalData(void);

```


## 兼容性

MCU                | SoftwareSerial | HardwareSerial |      IIC      |
------------------ | :----------: | :----------: | :----------: | 
Arduino Uno        |      √       |      X       |      √       |
Mega2560           |      √       |      √       |      √       |
Leonardo           |      √       |      √       |      √       |
ESP32              |      X       |      √       |      √       |
ESP8266            |      √       |      X       |      √       |
micro:bit          |      X       |      X       |      √       |
FireBeetle M0      |      X       |      √       |      X        |
raspberry          |      X       |      √       |      √       |

## 历史
- 2022-06-30 - 1.0.0 版本

## 创作者

Written by TangJie(jie.tang@dfrobot.com), 2021. (Welcome to our [website](https://www.dfrobot.com/))