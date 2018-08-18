/*
  ******************************************************************************
  * @file    esperto_timer.h
  * @author  Daniel De Sousa
  * @version V2.1.0
  * @date    18-August-2018
  * @brief   This library was developed for use with SAM D21 internal timers
  ******************************************************************************
*/
  
#ifndef esperto_timer_H
#define esperto_timer_H

#include "Arduino.h"
#include "samd21/include/samd21g18a.h"
#include "tc.h"
#include "tc_interrupt.h"

class SAMDtimer { 
public:
    SAMDtimer(byte timerNr, tc_callback_t _ISR, unsigned period_us, bool ISRenable=1);
    
  protected:  
    void init(); 
    void calc(unsigned period_us, unsigned pulseWidth_us);  
	
	void configure(tc_clock_prescaler prescale, tc_counter_size countersize, tc_wave_generation wavegen, tc_count_direction countdir = TC_COUNT_DIRECTION_UP);
	void setPeriodMatch(uint32_t period, uint32_t match);
	void setCompare(uint8_t channum, uint32_t compare);
	
    unsigned periodCounter, PWcounter;
    tc_clock_prescaler prescale;
    tc_counter_size countersize;  
	
    uint8_t _timernum;
    struct tc_config config_tc;
    struct tc_module tc_instance;
};
#endif
