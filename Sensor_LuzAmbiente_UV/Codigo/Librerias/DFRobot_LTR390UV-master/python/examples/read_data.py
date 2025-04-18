# -*- coding: utf-8 -*
'''!
  @file  read_data.py
  @brief Run the routine to get ambient light intensity, and change the mode to get UV intensity
  @copyright   Copyright (c) 2021 DFRobot Co.Ltd (http://www.dfrobot.com)
  @license     The MIT License (MIT)
  @author      TangJie(jie.tang@dfrobot.com)
  @version     V1.0
  @date        2021-08-31
  @url         https://github.com/DFRobor/DFRobot_LTR390UV
'''
from __future__ import print_function
import sys
import os
sys.path.append("../")
import time
import RPi.GPIO as GPIO

from DFRobot_LTR390UV import *

ctype=0

ADDRESS = 0x1c 
I2C_1   = 0x01


if ctype==0:
  LTR390UV = DFRobot_LTR390UV_I2C(I2C_1 ,ADDRESS)
else:
  LTR390UV = DFRobot_LTR390UV_UART(9600, ADDRESS)


def setup():
  while (LTR390UV.begin() == False):
    print("Sensor initialize failed!!")
    time.sleep(1)
  print("Sensor  initialize success!!")
  LTR390UV.set_ALS_or_UVS_meas_rate(e18bit,e100ms)#Set resolution and sampling time of module
  LTR390UV.set_ALS_or_UVS_gain(eGain1)#Set gain
  LTR390UV.set_mode(ALSMode)#Set as ambient light mode, UVSMode (UV light mode)
  
def loop():


  data = LTR390UV.read_original_data()#Get raw data of ambient light and UV light
  #uvsData = ltr390.read_UVS_transform_data()#Get UV data within 0-10 converted from raw data
  print("-----------------------\r\n")
  print("ALS:" + str(data) + "\r\n")
  print("-----------------------\r\n")
  time.sleep(1)

if __name__ == "__main__":
  setup()
  while True:
    loop()
