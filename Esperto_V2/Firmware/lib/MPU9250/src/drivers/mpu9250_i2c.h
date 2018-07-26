/*
  ******************************************************************************
  * @file    mpu9250_i2c.h
  * @author  Daniel De Sousa
  * @version V2.0.0
  * @date    25-July-2018
  * @brief   
  ******************************************************************************
*/
#ifndef _MPU9250_I2C_H_
#define _MPU9250_I2C_H_

#if defined(__cplusplus) 
extern "C" {
#endif

int arduino_i2c_write(unsigned char slave_addr, unsigned char reg_addr,
                       unsigned char length, unsigned char * data);
int arduino_i2c_read(unsigned char slave_addr, unsigned char reg_addr,
                       unsigned char length, unsigned char * data);

#if defined(__cplusplus) 
}
#endif

#endif // _MPU9250_I2C_H_