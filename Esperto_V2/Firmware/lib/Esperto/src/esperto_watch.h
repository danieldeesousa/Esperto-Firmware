/*
  ******************************************************************************
  * @file    esperto_watch.h
  * @author  Daniel De Sousa
  * @version V2.1.2
  * @date    21-August-2018
  * @brief   Main Esperto Watch library
  * @note 	 Last revision: Updated accelz thresholds
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

// BLE definitions
#define NOTIF_COUNTER_MAX 10 // How many seconds notification appears on screen for
#define INFO_BUFFER_SIZE 15  // Size of buffers storing BLE data
#define BLE_RX_DATA_SIZE 21  // Size of incoming BLE data buffer
#define BLE_TX_DATA_SIZE 20  // Size of outgoing BLE data buffer
static int connected = FALSE;
static volatile uint8_t set_connectable = 1;
static uint16_t UARTServHandle, UARTTXCharHandle, UARTRXCharHandle;

// Heart rate definitions
#define IR_STANDBY_THRESH 50000   // IR min for device to go into standy mode
#define HEARTRATE_MIN_VALID 40    // Minimum heart rate value which is considered valid
#define HEARTRATE_MAX_VALID 200   // Maximum heart rate value which is considered valid
#define ARRAY_SIZE_HR 5           // Size of array containing latest HR values
#define SPO2_MIN_VALID 80         // Minimum SpO2 value which is considered valid
#define SPO2_BUF_LENGTH 10        // Length of buffer containing last calculated SpO2 values
#define SPO2_LIGHT_BUF_LENGTH 100 // Length of infrared and red light buffers

// FRAM definitions
#define FRAM_SIZE 32768       // Size of FRAM in Bytes
#define FRAM_COUNT_ADDR_H 0   // Address of FRAM count MSB
#define FRAM_COUNT_ADDR_L 1   // Address of FRAM count LSB
#define FRAM_DATA_BASE_ADDR 4 // First address which data can be assigned to
#define DATA_INTERVAL 30      // Amount of time in seconds between data writes (Bluetooth and FRAM)

// Acceleromter definitions
#define INTERRUPT_PIN 7            // Digital 7 - Connected to MPU interrupt pin
#define STEP_MIN_DIFF_THRESHOLD 20 // Minimum difference between step max and min for valid step
#define STEP_MIN_THRESH 100        // Maximum value a gyration trough can be or else considered high freq noise
#define MOTION_WAKE_THRESH 10      // 10mg - Acceleration at which a motion wakeup occurs
#define WAKEUP_FREQ 7              // 7 = 31.25 Hz - Frequency at which the MPU wakes up to read accelerometer

// Display-Acceleromter definitions
// Note: these defintions are used for turning on display with wrist
#define ACCEL_Z_MIN 600   // Minimum Z acceleration allowed for stable wrist after being multiplied by factor
#define ACCEL_Z_MAX 1200   // Maximum Z acceleration allowed for stable wrist after being multiplied by factor
#define ACCEL_FACTOR 1000 // Factor to simplify calculations
#define DISPLAY_TIMEOUT 3 // 3 second timeout used for hysterisis

// Power management definitions
#define CHARGE_PIN 3                  // Digital 3: determine if charging is complete
#define BATTERY_PIN 0                 // Analog 0: determine when battery is low/high
#define REFERENCE_VOLTAGE (3.3)       // Default reference voltage on a 3.3V mcu
#define ADC_RESOLUTION (1023.0)       // 10 bit ADC
#define VOLTAGE_SHUTDOWN_THRESH (3.5) // Voltage at which peripherals / mcu will go to sleep
#define PERIPH_SHUTDOWN 1             // Shutdown peripheral flag 
#define PERIPH_WAKEUP 0               // Wakeup peripheral flag
#define STANDBY_TIMEOUT 30            // 30 secconds - Device goes into standby mode every 30s if no presence

#endif 