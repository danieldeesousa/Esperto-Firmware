/*
  ******************************************************************************
  * @file    mpu9250_clk.h
  * @author  Daniel De Sousa
  * @version V2.1.0
  * @date    18-August-2018
  * @brief   
  ******************************************************************************
*/
#ifndef _MPU9250_CLK_H_
#define _MPU9250_CLK_H_

int arduino_get_clock_ms(unsigned long *count);
int arduino_delay_ms(unsigned long num_ms);

#endif // _MPU9250_CLK_H_