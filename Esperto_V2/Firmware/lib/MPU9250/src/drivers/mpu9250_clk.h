/*
  ******************************************************************************
  * @file    mpu9250_clk.h
  * @author  Daniel De Sousa
  * @version V2.0.0
  * @date    25-July-2018
  * @brief   
  ******************************************************************************
*/
#ifndef _MPU9250_CLK_H_
#define _MPU9250_CLK_H_

int arduino_get_clock_ms(unsigned long *count);
int arduino_delay_ms(unsigned long num_ms);

#endif // _MPU9250_CLK_H_