/*
  ******************************************************************************
  * @file    esperto_rtc.cpp
  * @author  Daniel De Sousa
  * @version V2.1.0
  * @date    18-August-2018
  * @brief   
  ******************************************************************************
*/

#include "esperto_rtc.h"

voidFuncPtr RTC_callBack = NULL;

// RTC Constructor
Esperto_RTC::Esperto_RTC()
{
  _configured = false;
}

void Esperto_RTC::begin(bool resetTime)
{
  uint16_t tmp_reg = 0;
  
  PM->APBAMASK.reg |= PM_APBAMASK_RTC; // turn on digital interface clock
  config32kOSC();

  // If the RTC is in clock mode and the reset was not due to POR or BOD, preserve the clock time
  bool validTime = false;
  RTC_MODE2_CLOCK_Type oldTime;

  if ((!resetTime) && (PM->RCAUSE.reg & (PM_RCAUSE_SYST | PM_RCAUSE_WDT | PM_RCAUSE_EXT))) {
    if (RTC->MODE2.CTRL.reg & RTC_MODE2_CTRL_MODE_CLOCK) {
      validTime = true;
      oldTime.reg = RTC->MODE2.CLOCK.reg;
    }
  }

  // Setup clock GCLK2 with OSC32K divided by 32
  configureClock();

  // Disable and reset the RTC
  RTCdisable();
  RTCreset();

  // Set clock configurations
  tmp_reg |= RTC_MODE2_CTRL_MODE_CLOCK; // set clock operating mode
  tmp_reg |= RTC_MODE2_CTRL_PRESCALER_DIV1024; // set prescaler to 1024 for MODE2
  tmp_reg &= ~RTC_MODE2_CTRL_MATCHCLR; // disable clear on match
  
  // According to the datasheet RTC_MODE2_CTRL_CLKREP = 0 for 24h
  tmp_reg &= ~RTC_MODE2_CTRL_CLKREP; // 24h time representation

  // Disable continuously mode
  RTC->MODE2.READREQ.reg &= ~RTC_READREQ_RCONT; 

  // Set RTC control registers
  RTC->MODE2.CTRL.reg = tmp_reg;
  while (RTCisSyncing())
    ;

  // Enable RTC interrupt 
  NVIC_EnableIRQ(RTC_IRQn); 
  NVIC_SetPriority(RTC_IRQn, 0x00);

  // Enable alarm interrupt and set alarm off
  RTC->MODE2.INTENSET.reg |= RTC_MODE2_INTENSET_ALARM0; 
  RTC->MODE2.Mode2Alarm[0].MASK.bit.SEL = RTC_MODE2_MASK_SEL_OFF_Val;
  
  while (RTCisSyncing())
    ;

  RTCenable();
  RTCresetRemove();

  // If desired and valid, restore the time value
  if ((!resetTime) && (validTime)) {
    RTC->MODE2.CLOCK.reg = oldTime.reg;
    while (RTCisSyncing())
      ;
  }

  _configured = true;
}

void RTC_Handler(void)
{
  if (RTC_callBack != NULL) {
    RTC_callBack();
  }

  // Clear flag at end
  RTC->MODE2.INTFLAG.reg = RTC_MODE2_INTFLAG_ALARM0;
}

void Esperto_RTC::attachInterrupt(voidFuncPtr callback)
{
  RTC_callBack = callback;
}

void Esperto_RTC::detachInterrupt()
{
  RTC_callBack = NULL;
}

// Note: Entering standby mode when connected via the native USB port causes issues.
void Esperto_RTC::standbyMode()
{
  SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
  __WFI();
}

void Esperto_RTC::enableAlarm(Alarm_Match match)
{
  if (_configured) {
    RTC->MODE2.Mode2Alarm[0].MASK.bit.SEL = match;
    while (RTCisSyncing())
      ;
  }
}

void Esperto_RTC::disableAlarm()
{
  if (_configured) {
    RTC->MODE2.Mode2Alarm[0].MASK.bit.SEL = 0x00;
    while (RTCisSyncing())
      ;
  }
}

/* Time/Date Functions */
/* GET Functions */
uint8_t Esperto_RTC::getSeconds()
{
  RTCreadRequest();
  return RTC->MODE2.CLOCK.bit.SECOND;
}

uint8_t Esperto_RTC::getMinutes()
{
  RTCreadRequest();
  return RTC->MODE2.CLOCK.bit.MINUTE;
}

uint8_t Esperto_RTC::getHours()
{
  RTCreadRequest();
  return RTC->MODE2.CLOCK.bit.HOUR;
}

uint8_t Esperto_RTC::getDay()
{
  RTCreadRequest();
  return RTC->MODE2.CLOCK.bit.DAY;
}

uint8_t Esperto_RTC::getMonth()
{
  RTCreadRequest();
  return RTC->MODE2.CLOCK.bit.MONTH;
}

uint8_t Esperto_RTC::getYear()
{
  RTCreadRequest();
  return RTC->MODE2.CLOCK.bit.YEAR;
}

/* SET Functions */
void Esperto_RTC::setSeconds(uint8_t seconds)
{
  if (_configured) {
    RTC->MODE2.CLOCK.bit.SECOND = seconds;
    while (RTCisSyncing())
      ;
  }
}

void Esperto_RTC::setMinutes(uint8_t minutes)
{
  if (_configured) {
    RTC->MODE2.CLOCK.bit.MINUTE = minutes;
    while (RTCisSyncing())
      ;
  }
}

void Esperto_RTC::setHours(uint8_t hours)
{
  if (_configured) {
    RTC->MODE2.CLOCK.bit.HOUR = hours;
    while (RTCisSyncing())
      ;
  }
}

void Esperto_RTC::setTime(uint8_t hours, uint8_t minutes, uint8_t seconds)
{
  if (_configured) {
    setSeconds(seconds);
    setMinutes(minutes);
    setHours(hours);
  }
}

void Esperto_RTC::setDay(uint8_t day)
{
  if (_configured) {
    RTC->MODE2.CLOCK.bit.DAY = day;
    while (RTCisSyncing())
      ;
  }
}

void Esperto_RTC::setMonth(uint8_t month)
{
  if (_configured) {
    RTC->MODE2.CLOCK.bit.MONTH = month;
    while (RTCisSyncing())
      ;
  }
}

void Esperto_RTC::setYear(uint8_t year)
{
  if (_configured) {
    RTC->MODE2.CLOCK.bit.YEAR = year;
    while (RTCisSyncing())
      ;
  }
}

void Esperto_RTC::setDate(uint8_t day, uint8_t month, uint8_t year)
{
  if (_configured) {
    setDay(day);
    setMonth(month);
    setYear(year);
  }
}

/* Alarm Functions */
/* GET Functions */
uint8_t Esperto_RTC::getAlarmSeconds()
{
  return RTC->MODE2.Mode2Alarm[0].ALARM.bit.SECOND;
}

uint8_t Esperto_RTC::getAlarmMinutes()
{
  return RTC->MODE2.Mode2Alarm[0].ALARM.bit.MINUTE;
}

uint8_t Esperto_RTC::getAlarmHours()
{
  return RTC->MODE2.Mode2Alarm[0].ALARM.bit.HOUR;
}

uint8_t Esperto_RTC::getAlarmDay()
{
  return RTC->MODE2.Mode2Alarm[0].ALARM.bit.DAY;
}

uint8_t Esperto_RTC::getAlarmMonth()
{
  return RTC->MODE2.Mode2Alarm[0].ALARM.bit.MONTH;
}

uint8_t Esperto_RTC::getAlarmYear()
{
  return RTC->MODE2.Mode2Alarm[0].ALARM.bit.YEAR;
}

/* SET Functions */
void Esperto_RTC::setAlarmSeconds(uint8_t seconds)
{
  if (_configured) {
    RTC->MODE2.Mode2Alarm[0].ALARM.bit.SECOND = seconds;
    while (RTCisSyncing())
      ;
  }
}

void Esperto_RTC::setAlarmMinutes(uint8_t minutes)
{
  if (_configured) {
    RTC->MODE2.Mode2Alarm[0].ALARM.bit.MINUTE = minutes;
    while (RTCisSyncing())
      ;
  }
}

void Esperto_RTC::setAlarmHours(uint8_t hours)
{
  if (_configured) {
    RTC->MODE2.Mode2Alarm[0].ALARM.bit.HOUR = hours;
    while (RTCisSyncing())
      ;
  }
}

void Esperto_RTC::setAlarmTime(uint8_t hours, uint8_t minutes, uint8_t seconds)
{
  if (_configured) {
    setAlarmSeconds(seconds);
    setAlarmMinutes(minutes);
    setAlarmHours(hours);
  }
}

void Esperto_RTC::setAlarmDay(uint8_t day)
{
  if (_configured) {
    RTC->MODE2.Mode2Alarm[0].ALARM.bit.DAY = day;
    while (RTCisSyncing())
      ;
  }
}

void Esperto_RTC::setAlarmMonth(uint8_t month)
{
  if (_configured) {
    RTC->MODE2.Mode2Alarm[0].ALARM.bit.MONTH = month;
    while (RTCisSyncing())
      ;
  }
}

void Esperto_RTC::setAlarmYear(uint8_t year)
{
  if (_configured) {
    RTC->MODE2.Mode2Alarm[0].ALARM.bit.YEAR = year;
    while (RTCisSyncing())
      ;
  }
}

void Esperto_RTC::setAlarmDate(uint8_t day, uint8_t month, uint8_t year)
{
  if (_configured) {
    setAlarmDay(day);
    setAlarmMonth(month);
    setAlarmYear(year);
  }
}


/*
 * Private Utility Functions
 */

// Attach peripheral clock to 32k oscillator
void Esperto_RTC::configureClock() {
  GCLK->GENDIV.reg = GCLK_GENDIV_ID(2)|GCLK_GENDIV_DIV(4);
  while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY)
    ;
  GCLK->GENCTRL.reg = (GCLK_GENCTRL_GENEN | GCLK_GENCTRL_SRC_XOSC32K | GCLK_GENCTRL_ID(2) | GCLK_GENCTRL_DIVSEL );
  while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY)
    ;
  GCLK->CLKCTRL.reg = (uint32_t)((GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK2 | (RTC_GCLK_ID << GCLK_CLKCTRL_ID_Pos)));
  while (GCLK->STATUS.bit.SYNCBUSY)
    ;
}

// Configure the 32768Hz Oscillator
void Esperto_RTC::config32kOSC() 
{
  SYSCTRL->XOSC32K.reg = SYSCTRL_XOSC32K_ONDEMAND |
                         SYSCTRL_XOSC32K_RUNSTDBY |
                         SYSCTRL_XOSC32K_EN32K |
                         SYSCTRL_XOSC32K_XTALEN |
                         SYSCTRL_XOSC32K_STARTUP(6) |
                         SYSCTRL_XOSC32K_ENABLE;
}

// Synchronise the CLOCK register for reading
inline void Esperto_RTC::RTCreadRequest() {
  if (_configured) {
    RTC->MODE2.READREQ.reg = RTC_READREQ_RREQ;
    while (RTCisSyncing())
      ;
  }
}

// Wait for sync in write operations
inline bool Esperto_RTC::RTCisSyncing()
{
  return (RTC->MODE2.STATUS.bit.SYNCBUSY);
}

void Esperto_RTC::RTCdisable()
{
  RTC->MODE2.CTRL.reg &= ~RTC_MODE2_CTRL_ENABLE; // disable RTC
  while (RTCisSyncing())
    ;
}

void Esperto_RTC::RTCenable()
{
  RTC->MODE2.CTRL.reg |= RTC_MODE2_CTRL_ENABLE; // enable RTC
  while (RTCisSyncing())
    ;
}

void Esperto_RTC::RTCreset()
{
  RTC->MODE2.CTRL.reg |= RTC_MODE2_CTRL_SWRST; // software reset
  while (RTCisSyncing())
    ;
}

void Esperto_RTC::RTCresetRemove()
{
  RTC->MODE2.CTRL.reg &= ~RTC_MODE2_CTRL_SWRST; // software reset remove
  while (RTCisSyncing())
    ;
}
