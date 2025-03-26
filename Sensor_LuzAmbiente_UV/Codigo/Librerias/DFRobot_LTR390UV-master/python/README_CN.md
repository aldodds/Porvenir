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

要使用这个库，首先将库下载到Raspberry Pi，然后打开例程文件夹。要执行一个例程demox.py，请在命令行中输入python demox.py。例如，要执行control_led.py例程，你需要输入:

```python
python control_led.py
```



## 方法

```python
def begin(self)
    '''
      @brief 初始化传感器
    '''

  def set_mode(self,mode)
    '''
      @brief 设置模块采集数据模式
      @param mode 采集数据选择
    '''
  
  def set_ALS_or_UVS_meas_rate(self,data)
    '''
      @brief 设置模块采集数据位数和采集时间，采集时间必须大于采集位数所需时间
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
      @param data 控制数据
    '''

  def set_ALS_or_UVS_gain(self,data)
    '''
      @brief 设置传感器增益调节
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
      @param data 控制数据 
    '''
  def set_ALD_or_UVS_intcfg(self,data)
    '''
      @brief 环境光中断设置
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
      @param data 控制数据
    '''

  def set_UVS_or_ALS_thres_up_data(self,data)
    '''
      @brief 设置中断阈值上限值
      @param data 中断上限阈值，范围0~0x000fffff
    '''
  
  def set_UVS_or_ALS_thres_low_data(self,data)
    '''
      @brief 设置中断阈值下限值
      @param data 中断下限阈值，范围0~0x000fffff
    '''
  
  def read_original_data(self)
    '''
      @brief 获取原始数据
      @return 返回获取得原始数据
    '''
  
  def read_UVS_transform_data(self)
    '''
      @brief 获取转换后得UVS数据
      @return 返回转换后的数据
    '''

  def set_UVS_or_ALS_thresvar(self,data)
    '''
      @brief 设置环境光或紫外线数据变化次数中断
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
      @param data 发送的数据
    '''
```

## Compatibility

* RaspberryPi Version

| Board        | Work Well | Work Wrong | Untested | Remarks |
| ------------ | :-------: | :--------: | :------: | ------- |
| RaspberryPi2 |           |            |    √     |         |
| RaspberryPi3 |     √     |            |          |         |
| RaspberryPi4 |           |            |     √    |         |

* Python Version

| Python  | Work Well | Work Wrong | Untested | Remarks |
| ------- | :-------: | :--------: | :------: | ------- |
| Python2 |     √     |            |          |         |
| Python3 |     √     |            |          |         |

## History

- 2022-06-30 - 1.0.0 版本

## Credits

Written by TangJie(jie.tang@dfrobot.com), 2021. (Welcome to our [website](https://www.dfrobot.com/))