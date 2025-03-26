/*!
 * @file DFRobot_LTR390UV.cpp
 * @brief This is the method implementation file of LTR390UV
 * @copyright   Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license     The MIT License (MIT)
 * @author [TangJie](jie.tang@dfrobot.com)
 * @version  V1.0
 * @date  2022-05-17
 * @url https://github.com/DFRobor/DFRobot_LTR390UV
 */

#include "DFRobot_LTR390UV.h"


DFRobot_LTR390UV::DFRobot_LTR390UV(uint8_t addr, TwoWire *pWire)
{
  _pWire = pWire;
  _addr = addr;
};

DFRobot_LTR390UV::DFRobot_LTR390UV(uint8_t addr, Stream *s):DFRobot_RTU(s)
{
  _s = s;
  _addr = addr;
}

int8_t DFRobot_LTR390UV::begin(void)
{
  delay(500);
  setTimeoutTimeMs(200);
  if(_pWire){
    _pWire->begin();
  }
  
  if(_addr > 0xF7){
    DBG("Invaild Device addr.");
  }
  if(_addr != 0){
    if(!detectDeviceAddress(_addr)){
      DBG("Device addr Error.");
      return -1;
    }
  }else{
    return -1;
  }
  return 0;
}
void DFRobot_LTR390UV::setMode(eModel_t mode)
{
  _mode = mode;
  uint8_t _sendData[2];
  _sendData[0] = 0;
  _sendData[1] = mode;
 
  writeReg(LTR390UV_HOLDINGREG_MAIN_CTRL,&_sendData,2);
}

void DFRobot_LTR390UV::setALSOrUVSMeasRate(eResolution bit,eMeasurementRate time)
{
  uint8_t _sendData[2];
  uint8_t data = bit+time;
  resolution = (data&0xf0)>>4;
  _sendData[0] = 0;
  _sendData[1] = data;
  writeReg(LTR390UV_HOLDINGREG_ALS_UVS_MEAS_RATE,&_sendData,2);
}

void DFRobot_LTR390UV::setALSOrUVSGain(eGainRange data)
{ 
  uint8_t _sendData[2];
  gain = data;
  _sendData[0] = 0;
  _sendData[1] = data;
  writeReg(LTR390UV_HOLDINGREG_ALS_UVS_GAIN,&_sendData,2);
}

uint32_t DFRobot_LTR390UV::readOriginalData(void)
{
  float data = 0;
  uint32_t originalData = 0;
  uint8_t buffer[4];
  DBG(resolution);
  DBG(gain);
  DBG(a_int[resolution]);
  DBG(a_gain[gain]);
  if(_mode == eModel_t::eALSMode){
    readReg(LTR390UV_INPUTREG_ALS_DATA_LOW,buffer,4,0);
    originalData = (uint32_t)buffer[2]<<24|(uint32_t)buffer[3]<<16|(uint16_t)buffer[0]<<8|buffer[1];
    DBG(0.6*originalData);
    data =originalData;
  }else{
    readReg(LTR390UV_INPUTREG_UVS_DATA_LOW,buffer,4,0);
    originalData = (uint32_t)buffer[2]<<24|(uint32_t)buffer[3]<<16|(uint16_t)buffer[0]<<8|buffer[1];
    data = originalData;
  }
 
  return data;
}

float DFRobot_LTR390UV::readALSTransformData(void)
{
  float data=0.0;
  uint32_t originalData = 0;
  uint8_t buffer[4];
  if(_mode == eModel_t::eALSMode){
    readReg(LTR390UV_INPUTREG_ALS_DATA_LOW,buffer,4,0);
    originalData = (uint32_t)buffer[2]<<24|(uint32_t)buffer[3]<<16|(uint16_t)buffer[0]<<8|buffer[1];
    data = (0.6*originalData)/(a_gain[gain]*a_int[resolution]);
  }
  return data;

}

bool  DFRobot_LTR390UV::detectDeviceAddress(uint8_t addr)
{
  if(_pWire){
  uint8_t buf[2];
  readReg(LTR390UV_INPUTREG_ADDR, buf, 2,0);
  if(addr == ((buf[0] << 8| buf[1]) & 0xFF))
    return true;
  }else{
    uint16_t ret = readInputRegister(addr, LTR390UV_INPUTREG_ADDR);
    
    if((ret & 0xFF) == addr)
      return true;
  }
  return false;
}

uint8_t DFRobot_LTR390UV::readReg(uint16_t reg, void *pBuf, uint8_t size,uint8_t state)
{
  uint8_t* _pBuf = (uint8_t*)pBuf;
  uint8_t _reg  = 0;
    if(pBuf == NULL){
      DBG("data error");
      return 0;
    }
  if(_pWire){
    if(state == 1)
      _reg = reg+5;
    _pWire->beginTransmission(_addr);
    _pWire->write(reg);
    _pWire->endTransmission();
    _pWire->requestFrom(_addr, size);
    for(uint8_t i = 0; i < size; i++)
      _pBuf[i] = _pWire->read();
    for(uint8_t i = 0; i < size;){
      uint8_t temp = _pBuf[i];
      _pBuf[i] = _pBuf[i+1];
      _pBuf[i+1] = temp;
      i+=2;
    }
    return size;
  }else{
    return readInputRegister(_addr, reg, _pBuf, size);
  }
}
uint8_t DFRobot_LTR390UV::writeReg(uint8_t reg, void *pBuf, size_t size)
{
  uint8_t *_pBuf = (uint8_t*)pBuf;

  uint8_t ret = 0;
  if(_pWire){
    uint8_t _reg = reg+5;
    _pWire->beginTransmission(_addr);
    _pWire->write(_reg);
    for(uint8_t i = 0; i < size;){
      uint8_t temp = _pBuf[i];
      _pBuf[i] = _pBuf[i+1];
      _pBuf[i+1] = temp;
      i+=2;
    }
    for(size_t i = 0; i < size; i++){
      _pWire->write(_pBuf[i]);
    }
    _pWire->endTransmission();
  }else{
    ret = writeHoldingRegister(_addr,reg,_pBuf,size);
  }
  return ret;
}
