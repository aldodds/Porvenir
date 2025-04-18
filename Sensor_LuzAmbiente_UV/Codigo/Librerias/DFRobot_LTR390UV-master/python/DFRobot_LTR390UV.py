# -*- coding: utf-8 -*
'''!
  @file       DFRobot_LTR390UV.py
  @brief       This is basic library of LTR390UV sensor
  @copyright   Copyright (c) 2021 DFRobot Co.Ltd (http://www.dfrobot.com)
  @license     The MIT License (MIT)
  @author      TangJie(jie.tang@dfrobot.com)
  @version     V1.0
  @date        2021-08-31
  @url         https://github.com/DFRobor/DFRobot_LTR390UV
'''

import serial
import time
import smbus
import os
import math
import RPi.GPIO as GPIO
import math

import modbus_tk
import modbus_tk.defines as cst
from modbus_tk import modbus_rtu

I2C_MODE                  = 0x01
UART_MODE                 = 0x02
DEV_ADDRESS               = 0x1c
ALSMode                   = 0x02
UVSMode                   = 0x0A

#Input Register
LTR390UV_INPUTREG_PID                           =0x00   #Device PID
LTR390UV_INPUTREG_VID                           =0x01   #Device VID, fixed to 0x3343
LTR390UV_INPUTREG_ADDR                          =0x02   #Device address of module
LTR390UV_INPUTREG_BAUDRATE                      =0x03   #Serial baud rate
LTR390UV_INPUTREG_STOPBIT                       =0x04   #Serial check bit and stop bit
LTR390UV_INPUTREG_VERSION                       =0x05   #Firmware version
LTR390UV_INPUTREG_PART_ID                       =0x06   #Device ID of sensor
LTR390UV_INPUTREG_ALS_DATA_LOW                  =0x07   #The low bit of ambient light intensity
LTR390UV_INPUTREG_ALS_DATA_HIGH                 =0x08   #The high bit of ambient light intensity
LTR390UV_INPUTREG_UVS_DATA_LOW                  =0x09   #The low bit of UV intensity
LTR390UV_INPUTREG_UVS_DATA_HIGH                 =0x0A   #The high bit of UV intensity
#Holding Register
LTR390UV_HOLDINGREG_ALS_UVS_GAIN                =0x06   #Gain adjustment
LTR390UV_HOLDINGREG_INT_CFG                     =0x07   #Interrupt config
LTR390UV_HOLDINGREG_UVS_ALS_THRES_UP_DATA_LOW   =0x08   #The low bit of upper threshold of UV or ambient light
LTR390UV_HOLDINGREG_UVS_ALS_THRES_UP_DATA_HIGH  =0x09   #The high bit of upper threshold of UV or ambient light
LTR390UV_HOLDINGREG_UVS_ALS_THRES_LOW_DATA_LOW  =0x0A   #The low bit of lower threshold of UV or ambient light
LTR390UV_HOLDINGREG_UVS_ALS_THRES_LOW_DATA_HIGH =0x0B   #The high bit of lower threshold of UV or ambient light
LTR390UV_HOLDINGREG_UVS_ALS_THRES_VAR_DATA      =0x0C   #Threshold of UV or ambient light data change counts 
LTR390UV_HOLDINGREG_ALS_UVS_MEAS_RATE           =0x0D   #Resolution and sampling time setting
LTR390UV_HOLDINGREG_MAIN_CTRL                   =0x0E   #Sensor mode select
a_gain = [1,3,6,9,18]
a_int = [4.,2.,1.,0.5,0.25,0.25]


eGain1 = 0 #Gain of 1
eGain3 = 1 #Gain of 3
eGain6 = 2 #Gain of 6
eGain9 = 3 #Gain of 9
eGain18 =4 #Gain of 18

e20bit = 0  #20-bit data
e19bit = 16 #19-bit data
e18bit = 32 #18-bit data
e17bit = 48 #17-bit data
e16bit = 64 #16-bit data
e13bit =80  #13-bit data

e25ms = 0  #Sampling time of 25ms
e50ms = 1  #Sampling time of 50ms
e100ms = 2 #Sampling time of 100ms
e200ms = 3 #Sampling time of 200ms
e500ms = 4 #Sampling time of 500ms
e1000ms =5 #Sampling time of 1000ms
e2000ms = 6 #Sampling time of 2000ms


class DFRobot_LTR390UV():
  def __init__(self ,bus = 0 ,baud = 9600, mode = I2C_MODE):
    self.mode = 0
    self.resolution = 0
    self.gain = 0
    if mode == I2C_MODE:
      self.i2cbus = smbus.SMBus(bus)
      self._uart_i2c = I2C_MODE
    else:
      self.master = modbus_rtu.RtuMaster(serial.Serial(port="/dev/ttyAMA0",baudrate=baud, bytesize=8, parity='N', stopbits=1))
      self.master.set_timeout(1.0)
      self._uart_i2c = UART_MODE
  def _detect_device_address(self):
    '''!
      @brief Get sensor address
      @return  Return sensor address
    '''
    rbuf = self._read_reg(0x02,2,0)
    if self._uart_i2c == I2C_MODE:
      data = rbuf[0] | rbuf[1] << 8
    elif self._uart_i2c == UART_MODE:
      data = rbuf[0]
    return data

  def begin(self):
    '''
      @brief Initialize sensor
    '''
    if self._detect_device_address() != DEV_ADDRESS:
      return False
    return True

  def set_mode(self,mode):
    '''
      @brief Set data-collecting mode of module
      @param mode Data-collecting mode select
    '''
    self.mode = mode
    if self._uart_i2c == I2C_MODE:
      buffer=[self.mode,0]
    else:
      buffer = [mode]
    self._write_reg(LTR390UV_HOLDINGREG_MAIN_CTRL,buffer) 
  
  def set_ALS_or_UVS_meas_rate(self,bit,time):#self,bit,time
    '''
      @brief Set resolution and sampling time of module, the sampling time must be greater than the time for collecting resolution
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
    self.gain = bit+time
    self.resolution = (self.gain&0xf0)>>4
    if self._uart_i2c == I2C_MODE:
      buffer=[self.gain,0]
    else:
      buffer = [self.gain]
    self._write_reg(LTR390UV_HOLDINGREG_ALS_UVS_MEAS_RATE,buffer) 
  def set_ALS_or_UVS_gain(self,data):
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
    self.gain = data
    if self._uart_i2c == I2C_MODE:
      buffer=[data,0]
    else:
      buffer = [data]
    self._write_reg(LTR390UV_HOLDINGREG_ALS_UVS_GAIN,buffer)  
  def read_original_data(self):
    '''
      @brief Get raw data
      @return Return the obtained raw data
    '''
    if self._uart_i2c == I2C_MODE:
      if self.mode == ALSMode:
        buffer = self._read_reg(LTR390UV_INPUTREG_ALS_DATA_LOW,4,0)
        data = buffer[2]<<16|buffer[3]<<24|buffer[0]|buffer[1]<<8
        
      elif self.mode == UVSMode:
        buffer = self._read_reg(LTR390UV_INPUTREG_UVS_DATA_LOW,4,0)
        data = buffer[2]<<16|buffer[3]<<24|buffer[0]|buffer[1]<<8
    else:
      if self.mode == ALSMode:
        buffer = self._read_reg(LTR390UV_INPUTREG_ALS_DATA_LOW,2,0)
        data = buffer[0]|buffer[1]<<16
      elif self.mode == UVSMode:
        buffer = self._read_reg(LTR390UV_INPUTREG_UVS_DATA_LOW,2,0)
        data = buffer[0]|buffer[1]<<16
    return data
  
  def read_ALSTrans_form_data(self):
    if self._uart_i2c == I2C_MODE:
      if self.mode == ALSMode:
        buffer = self._read_reg(LTR390UV_INPUTREG_ALS_DATA_LOW,4,0)
        data = buffer[2]<<16|buffer[3]<<24|buffer[0]|buffer[1]<<8
        data = (0.6*data)/(a_gain[self.gain]*a_int[self.resolution])
      else:
        data = 0
    else:
      if self.mode == ALSMode:
        buffer = self._read_reg(LTR390UV_INPUTREG_ALS_DATA_LOW,2,0)
        data = buffer[0]|buffer[1]<<16
      else:
        data = 0
    return data
  
class DFRobot_LTR390UV_I2C(DFRobot_LTR390UV):
  '''!
    @brief An example of an i2c interface module
  '''
  def __init__(self ,bus ,addr):
    self._addr = addr
    DFRobot_LTR390UV.__init__(self,bus,0,I2C_MODE)   
    
  
  def _read_reg(self, reg_addr ,length, state):
    '''!
      @brief read the data from the register
      @param reg_addr register address
      @param length read data
    '''
    self._reg = reg_addr
    rslt = self.i2cbus.read_i2c_block_data(self._addr ,self._reg , length)
    return rslt

  def _write_reg(self, reg_addr ,data):
    '''!
      @brief write the data from the register
      @param reg_addr register address
      @param data Data written to register 
    '''
    self._reg = reg_addr +5
    try:
      rslt = self.i2cbus.write_i2c_block_data(self._addr ,self._reg , data)
    except:
      rslt = -1
    return rslt        


class DFRobot_LTR390UV_UART(DFRobot_LTR390UV):
  '''!
    @brief An example of an UART interface module
  '''
  def __init__(self ,baud, addr):
    self._baud = baud
    self._addr = addr
    try:
      DFRobot_LTR390UV.__init__(self,0,self._baud,UART_MODE)
    except:
      print ("plese get root!")
   
  
  def _read_reg(self, reg_addr ,length):
    '''!
      @brief Read data from the sensor
    '''
    return list(self.master.execute(self._addr, cst.READ_INPUT_REGISTERS, reg_addr, length))
  
  def _write_reg(self, reg_addr ,data):
    '''!
      @brief write data from the sensor
    '''
    return list(self.master.execute(self._addr, cst.WRITE_MULTIPLE_REGISTERS, reg_addr, output_value=data))
