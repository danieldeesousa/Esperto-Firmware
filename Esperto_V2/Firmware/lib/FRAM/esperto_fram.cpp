/*
  ******************************************************************************
  * @file    esperto_fram.cpp
  * @author  Daniel De Sousa
  * @version V2.0.0
  * @date    16-July-2018
  * @brief   
  ******************************************************************************
*/

#include "esperto_fram.h"

// FRAM Constructor
Esperto_FRAM::Esperto_FRAM(void) 
{
  _framInitialised = false;
}

// Initializes I2C and configures the FRAM
boolean Esperto_FRAM::begin(uint8_t addr) 
{
  // Start I2C bus
  i2c_addr = addr;
  Wire.begin();
  
  // Make sure we're connected
  uint16_t manufID, prodID;
  getDeviceID(&manufID, &prodID);
  if (manufID != 0x00A)
  {
    return false;
  }
  if (prodID != 0x510)
  {
    return false;
  }

  // Init complete
  _framInitialised = true;

  return true;
}


// Writes a byte to the specific FRAM address
void Esperto_FRAM::write8 (uint16_t framAddr, uint8_t value)
{
  Wire.beginTransmission(i2c_addr);
  Wire.write(framAddr >> 8);
  Wire.write(framAddr & 0xFF);
  Wire.write(value);
  Wire.endTransmission();
}

// Reads a byte value from the specified FRAM address
uint8_t Esperto_FRAM::read8 (uint16_t framAddr)
{
  Wire.beginTransmission(i2c_addr);
  Wire.write(framAddr >> 8);
  Wire.write(framAddr & 0xFF);
  Wire.endTransmission();
  Wire.requestFrom(i2c_addr, (uint8_t)1);
  
  return Wire.read();
}

// Reads the Manufacturer ID and the Product ID frm the IC
void Esperto_FRAM::getDeviceID(uint16_t *manufacturerID, uint16_t *productID)
{
  uint8_t a[3] = { 0, 0, 0 };
  uint8_t results;
  
  Wire.beginTransmission(MB85RC_SLAVE_ID >> 1);
  Wire.write(i2c_addr << 1);
  results = Wire.endTransmission(false);

  Wire.requestFrom(MB85RC_SLAVE_ID >> 1, 3);
  a[0] = Wire.read();
  a[1] = Wire.read();
  a[2] = Wire.read();

  // Shift values to separate manuf and prod IDs
  // See p.10 of http://www.fujitsu.com/downloads/MICRO/fsa/pdf/products/memory/fram/MB85RC256V-DS501-00017-3v0-E.pdf
  *manufacturerID = (a[0] << 4) + (a[1]  >> 4);
  *productID = ((a[1] & 0x0F) << 8) + a[2];
}
