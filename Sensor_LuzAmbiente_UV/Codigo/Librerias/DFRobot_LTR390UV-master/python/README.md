DFRobot_LTR390UV
===========================

- [中文版](./README_CN.md)

The SEN0540 sensor can be used to detect ambient light and UV light intensity, and users can use it easily with the library.

![产品效果图](../../resources/images/SEN0540.png)

## Product Link (https://www.dfrobot.com)

    SKU：SEN0540

## Table of Contents

  * [summary](#summary)
  * [installation](#installation)
  * [methods](#methods)
  * [compatibility](#compatibility)
  * [history](#history)
  * [credits](#credits)

## Summary

This LTR390-UV sensor provides both ambient light and UV sensing with UV spectral response from 280nm to 430nm and raw data output. It features high sensitivity, quick response, and strong anti-interference ability.
With built-in ADC and MCU, this UV module can convert light data to a digital signal capable of being directly output via I2C or UART interface. It can be used for light experiments, outdoor UV detection, and other scenarios requiring UV or ambient light monitoring.


## Installation

Download this library to Raspberry Pi before use, then open the routine folder. Type python demox.py on the command line to execute a routine demox.py. For example, to execute the control_led.py routine, you need to enter:

```python
python control_led.py
```

## Methods

```python
  def begin(self)
    '''
      @brief Initialize sensor
    '''

  def set_mode(self,mode)
    '''
      @brief Set data acquisition mode for module
      @param mode Select data acquisition mode
    '''
  
  def set_ALS_or_UVS_meas_rate(self,data)
    '''
      @brief Set acquisition data bit and time, the acquisition time must be greater than the time the bit needs
      @n --------------------------------------------------------------------------------------------------------
      @n |    bit7    |    bit6    |    bit5    |    bit4    |    bit3    |    bit2    |    bit1    |    bit0    |
      @n ---------------------------------------------------------------------------------------------------------
      @n |  Reserved  |        ALS/UVS Resolution            |  Reserved  |   ALS/UVS Measurement Rate           |
      @n ---------------------------------------------------------------------------------------------------------
      @n | ALS/UVS Resolution       |000|20 Bit, Conversion time = 400ms                                         |
      @n |                          |001|19 Bit, Conversion time = 200ms                                         |
      @n |                          |010|18 Bit, Conversion time = 100ms(default)                                |
      @n |                          |011|17 Bit, Conversion time = 50ms                                          |
      @n |                          |100|16 Bit, Conversion time = 25ms                                          |
      @n |                          |110/111|Reserved                                                            |
      @n ---------------------------------------------------------------------------------------------------------
      @n | ALS/UVS Measurement Rate |000|25ms                                                                    |
      @n |                          |001|50ms                                                                    |
      @n |                          |010|100ms (default)                                                         |
      @n |                          |011|200ms                                                                   |
      @n |                          |100|500ms                                                                   |
      @n |                          |101|1000ms                                                                  |
      @n |                          |110/111|2000ms                                                              |
      @n ---------------------------------------------------------------------------------------------------------
      @param data Control data
    '''

  def set_ALS_or_UVS_gain(self,data)
    '''
      @brief Set sensor gain
      @n ---------------------------------------------------------------------------------------------------------
      @n |    bit7    |    bit6    |    bit5    |    bit4    |    bit3    |    bit2    |    bit1    |    bit0    |
      @n ---------------------------------------------------------------------------------------------------------
      @n |                                    Reserved                    |          ALS/UVS Gain Range          |
      @n ---------------------------------------------------------------------------------------------------------
      @n | ALS/UVS Gain Range       |000|Gain Range: 1                                                           |
      @n |                          |001|Gain Range: 3 (default)                                                 |
      @n |                          |010|Gain Range: 6                                                           |
      @n |                          |011|Gain Range: 9                                                           |
      @n |                          |100|Gain Range: 18                                                          |
      @n |                          |110/111|Reserved                                                            |
      @n ---------------------------------------------------------------------------------------------------------                  
      @param data Control data 
    '''
  def set_ALD_or_UVS_intcfg(self,data)
    '''
      @brief Ambient light interrupt setting
      @n ---------------------------------------------------------------------------------------------------------
      @n |    bit7    |    bit6    |    bit5    |    bit4    |    bit3    |    bit2    |    bit1    |    bit0    |
      @n ---------------------------------------------------------------------------------------------------------
      @n |        Reserved         |         LS_INT_SEL      | LS_VAR_MODE| LS_INT_EN  |    Reserved             |
      @n ---------------------------------------------------------------------------------------------------------
      @n | LS_INT_SEL               |00|Reserved                                                                 |
      @n |                          |01|ALS Channel (Default)                                                    |
      @n |                          |10|Reserved                                                                 |
      @n |                          |11|UVS Channe                                                               |
      @n ---------------------------------------------------------------------------------------------------------
      @n |     LS_VAR_MODE          |0|LS threshold interrupt mode (default)                                     |
      @n |                          |1|LS variation interrupt mode                                               |
      @n --------------------------------------------------------------------------------------------------------- 
      @n |     LS_INT_EN            |0|LS interrupt disabled (default)                                           |
      @n |                          |1|LS interrupt enabled                                                      |
      @n ---------------------------------------------------------------------------------------------------------      
      @param data Control data
    '''

  def set_UVS_or_ALS_thres_up_data(self,data)
    '''
      @brief Set interrupt upper threshold
      @param data Interrupt upper threshold, range 0~0x000fffff
    '''
  
  def set_UVS_or_ALS_thres_low_data(self,data)
    '''
      @brief Set interrupt lower threshold
      @param data Interrupt lower threshold, range 0~0x000fffff
    '''
  
  def read_original_data(self)
    '''
      @brief Get raw data
      @return Get the obtained raw data
    '''
  
  def read_UVS_transform_data(self)
    '''
      @brief Get the converted UVS data
      @return Return the converted data
    '''

  def set_UVS_or_ALS_thresvar(self,data)
    '''
      @brief Set ambient light and UV data variance count interrupt
      @n ---------------------------------------------------------------------------------------------------------
      @n |    bit7    |    bit6    |    bit5    |    bit4    |    bit3    |    bit2    |    bit1    |    bit0    |
      @n ---------------------------------------------------------------------------------------------------------
      @n |        Reserved                                                |    UVS/ALS Variance Threshold        |
      @n ---------------------------------------------------------------------------------------------------------
      @n | UVS/ALS Variance Threshold   |000|New DATA_x varies by 8 counts compared to previous result           |
      @n |                              |001|New DATA_x varies by 16 counts compared to previous result.         |
      @n |                              |010|New DATA_x varies by 32 counts compared to previous result.         |
      @n |                              |011|New DATA_x varies by 64 counts compared to previous result.         |
      @n |                              |100|New DATA_x varies by 128 counts compared to previous result.        |
      @n |                              |101|New DATA_x varies by 256 counts compared to previous result.        |
      @n |                              |110|New DATA_x varies by 512 counts compared to previous result.        |
      @n |                              |111|New DATA_x varies by 1024 counts compared to previous result.       |
      @n ---------------------------------------------------------------------------------------------------------      
      @param data Data to be sent
    '''
```

## Compatibility

* RaspberryPi Version

| Board        | Work Well | Work Wrong | Untested | Remarks |
| ------------ | :-------: | :--------: | :------: | ------- |
| Raspberry Pi2 |           |            |    √     |         |
| Raspberry Pi3 |           |            |    √     |         |
| Raspberry Pi4 |       √   |            |          |         |

* Python Version

| Python  | Work Well | Work Wrong | Untested | Remarks |
| ------- | :-------: | :--------: | :------: | ------- |
| Python2 |     √     |            |          |         |
| Python3 |     √     |            |          |         |

## History

- 2022-06-30 - Version 1.0.0 released.

## Credits

Written by TangJie(jie.tang@dfrobot.com), 2021. (Welcome to our [website](https://www.dfrobot.com/))
