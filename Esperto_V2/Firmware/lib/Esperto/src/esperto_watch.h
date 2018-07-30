/*
  ******************************************************************************
  * @file    esperto_watch.h
  * @author  Daniel De Sousa
  * @version V2.0.4
  * @date    29-July-2018
  * @brief   Main Esperto Watch library
  * @note 	 Last revision: Added definitions
  ******************************************************************************
*/
#ifndef __ESPERTO_WATCH_H
#define __ESPERTO_WATCH_H

#include "esperto_watch_inc.h"

// Esperto Watch class instances
static U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0); // OLED Display
static Esperto_FRAM fram = Esperto_FRAM();                // FRAM
static Esperto_RTC rtc;                                   // Real Time Clock
static MAX30102 heartRateSensor;                          // Heart Rate Sensor
static MPU9250_DMP imu;                                   // Acceleromter

// BLE Definitions
#define NOTIF_COUNTER_MAX 10 // how many seconds notification appears on screen for
#define INFO_BUFFER_SIZE 15  // size of buffers storing BLE data
#define BLE_DATA_SIZE 21     // size of incoming BLE data buffer
static int connected = FALSE;
static volatile uint8_t set_connectable = 1;
static uint16_t UARTServHandle, UARTTXCharHandle, UARTRXCharHandle;

// Heart rate definitions
#define ARRAY_SIZE_HR 5 // size of array containing latest HR values

// Acceleromter definitions
#define STEP_MIN_DIFF_THRESHOLD 20 // minimum difference between step max and min for considered step

#endif 