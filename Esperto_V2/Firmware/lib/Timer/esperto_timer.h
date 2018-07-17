/*
  ******************************************************************************
  * @file    esperto_timer.h
  * @author  Daniel De Sousa
  * @version V2.0.0
  * @date    16-July-2018
  * @brief   This library was developed for use with SAM D21 internal timers
  ******************************************************************************
*/
  
#ifndef SAMDtimer_H
#define SAMDtimer_H

#include <Adafruit_ZeroTimer.h>

class SAMDtimer : public Adafruit_ZeroTimer 
{ public:
    SAMDtimer(byte timerNr, tc_counter_size countersize, byte pin, unsigned period_us, int pulseWidth_us=-1, bool timerEnable=1); // For timer with output
    SAMDtimer(byte timerNr, tc_callback_t _ISR, unsigned period_us, bool ISRenable=1); // For timer interrupt, without output 
    
    void attachInterrupt(tc_callback_t _ISR, bool interruptEnable=1); // attach ISR to a timer with output, or exchange the ISR
    void enableTimer(bool timerEnable);
    void enableInterrupt(bool interruptEnable);
    void setPulseWidth(unsigned pulseWidth_us);
    
  protected:  
    void init(bool enabled); 
    void calc(unsigned period_us, unsigned pulseWidth_us);  

    byte pin;
    tc_callback_t ISR;
    unsigned period_us, periodCounter, PWcounter;
    tc_clock_prescaler prescale;
    tc_counter_size countersize;  
};
#endif
