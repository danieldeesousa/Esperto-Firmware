/*
  ******************************************************************************
  * @file    esperto_fram.h
  * @author  Daniel De Sousa
  * @version V2.1.0
  * @date    18-August-2018
  * @brief   This library was developed for use with the MB85RC256V 32KB FRAM over I2C
  ******************************************************************************
*/

#ifndef _ESPERTO_FRAM_H_
#define _ESPERTO_FRAM_H_

#include <Wire.h>
#include <stdlib.h>
#include <math.h>
#include "Arduino.h"

#define MB85RC_SLAVE_ID 		0xF8
#define MB85RC_DEFAULT_ADDRESS  0x50

class Esperto_FRAM {
 public:
  Esperto_FRAM(void);
  
  boolean  begin(uint8_t addr = MB85RC_DEFAULT_ADDRESS);
  void     write8 (uint16_t framAddr, uint8_t value);
  uint8_t  read8  (uint16_t framAddr);
  void     getDeviceID(uint16_t *manufacturerID, uint16_t *productID);

 private:
  uint8_t i2c_addr;
  boolean _framInitialised;
};

#endif // // ESPERTO_FRAM_H
