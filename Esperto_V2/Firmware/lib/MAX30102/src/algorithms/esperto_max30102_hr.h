/*
  ******************************************************************************
  * @file    esperto_max30102_hr.h
  * @author  Daniel De Sousa
  * @version V2.1.0
  * @date    18-August-2018
  * @brief   Algorithm to detect heart beat using an implementation of the Peripheral Beat Algorithm
  ******************************************************************************
*/

#include "Arduino.h"

bool checkForBeat(int32_t sample);
int16_t averageDCEstimator(int32_t *p, uint16_t x);
int16_t lowPassFIRFilter(int16_t din);
int32_t mul16(int16_t x, int16_t y);
