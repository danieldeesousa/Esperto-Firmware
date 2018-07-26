/*
  ******************************************************************************
  * @file    mpu9250_clk.cpp
  * @author  Daniel De Sousa
  * @version V2.0.0
  * @date    25-July-2018
  * @brief   
  ******************************************************************************
*/
#include "mpu9250_clk.h"
#include <Arduino.h>

int arduino_get_clock_ms(unsigned long *count)
{
	*count = millis();
	return 0;
}

int arduino_delay_ms(unsigned long num_ms)
{
	delay(num_ms);
	return 0;
}