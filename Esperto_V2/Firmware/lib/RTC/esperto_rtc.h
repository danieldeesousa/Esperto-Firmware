/*
  ******************************************************************************
  * @file    esperto_rtc.h
  * @author  Daniel De Sousa
  * @version V2.0.0
  * @date    16-July-2018
  * @brief   This library was developed for use with the SAM D21 to use the internal Real Time Clock
  ******************************************************************************
*/

#ifndef ESPERTO_RTC_H
#define ESPERTO_RTC_H

#include "Arduino.h"
#include <time.h>

typedef void(*voidFuncPtr)(void);

class Esperto_RTC {
public:

  Esperto_RTC();
  void begin(bool resetTime = false);

  // System API calls
  void attachInterrupt(voidFuncPtr callback);
  void detachInterrupt();
  void standbyMode();

  // Time / date getters
  uint8_t getSeconds();
  uint8_t getMinutes();
  uint8_t getHours();
  uint8_t getDay();
  uint8_t getMonth();
  uint8_t getYear();

  // Time / date setters
  void setSeconds(uint8_t seconds);
  void setMinutes(uint8_t minutes);
  void setHours(uint8_t hours);
  void setTime(uint8_t hours, uint8_t minutes, uint8_t seconds);
  void setDay(uint8_t day);
  void setMonth(uint8_t month);
  void setYear(uint8_t year);
  void setDate(uint8_t day, uint8_t month, uint8_t year);

  bool isConfigured() {
    _configured = RTC->MODE2.CTRL.reg & RTC_MODE2_CTRL_ENABLE;
    configureClock();
    return _configured;
  }

private:
  bool _configured;
  void config32kOSC(void);
  void configureClock(void);
  void RTCreadRequest();
  bool RTCisSyncing(void);
  void RTCdisable();
  void RTCenable();
  void RTCreset();
  void RTCresetRemove();
};

#endif // ESPERTO_RTC_H
