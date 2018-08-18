/*
  ******************************************************************************
  * @file    mpu9250_log.cpp
  * @author  Daniel De Sousa
  * @version V2.1.0
  * @date    18-August-2018
  * @brief   
  ******************************************************************************
*/
#include "mpu9250_log.h"
#include <Arduino.h>
#include <stdarg.h>

#define BUF_SIZE        (256)
#define PACKET_LENGTH   (23)

#define PACKET_DEBUG    (1)
#define PACKET_QUAT     (2)
#define PACKET_DATA     (3)

void logString(char * string) 
{
}

int _MLPrintLog (int priority, const char* tag, const char* fmt, ...)
{
}

void eMPL_send_quat(long *quat)
{
}

void eMPL_send_data(unsigned char type, long *data)
{
}