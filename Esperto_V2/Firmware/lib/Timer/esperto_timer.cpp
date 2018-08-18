/*
  ******************************************************************************
  * @file    esperto_timer.cpp
  * @author  Daniel De Sousa
  * @version V2.1.0
  * @date    18-August-2018
  * @brief   
  * @note Last revision: Refactored ZeroTimer code into this file
  ******************************************************************************
*/

#include "esperto_timer.h"

SAMDtimer::SAMDtimer(byte timerNr, tc_callback_t ISR, unsigned period_us, bool ISRenable)
{ 
  _timernum = timerNr;
  countersize = TC_COUNTER_SIZE_16BIT;
  
  // Calculate prescaler
  calc(period_us, period_us/2);
  
  // Initialize timer
  init();
  
  // Register callback function
  tc_register_callback(&tc_instance, ISR, TC_CALLBACK_CC_CHANNEL1);
  tc_enable_callback(&tc_instance, TC_CALLBACK_CC_CHANNEL1);
}

void SAMDtimer::init()
{ 
  // Configure timer
  configure(prescale, countersize, TC_WAVE_GENERATION_MATCH_PWM);
  
  // Setup counters
  setPeriodMatch(periodCounter, PWcounter);
  
  // Enable timer
  tc_enable(&tc_instance);
}

void SAMDtimer::calc(unsigned period_us, unsigned pulseWidth_us)
{ 
  periodCounter = (F_CPU * (signed)period_us) / 1000000; 
  PWcounter = (F_CPU * (signed)pulseWidth_us) / 1000000; 
  if(periodCounter < 65536) prescale = TC_CLOCK_PRESCALER_DIV1; 
  else if((PWcounter >>= 1, periodCounter >>= 1) < 65536) prescale = TC_CLOCK_PRESCALER_DIV2; // = 256
  else if((PWcounter >>= 1, periodCounter >>= 1) < 65536) prescale = TC_CLOCK_PRESCALER_DIV4; 
  else if((PWcounter >>= 1, periodCounter >>= 1) < 65536) prescale = TC_CLOCK_PRESCALER_DIV8; 
  else if((PWcounter >>= 1, periodCounter >>= 1) < 65536) prescale = TC_CLOCK_PRESCALER_DIV16; 
  else if((PWcounter >>= 2, periodCounter >>= 2) < 65536) prescale = TC_CLOCK_PRESCALER_DIV64; 
  else if((PWcounter >>= 2, periodCounter >>= 2) < 65536) prescale = TC_CLOCK_PRESCALER_DIV256; 
  else if((PWcounter >>= 2, periodCounter >>= 2) < 65536) prescale = TC_CLOCK_PRESCALER_DIV1024; 
}

void SAMDtimer::configure(tc_clock_prescaler prescale, tc_counter_size countersize, tc_wave_generation wavegen, tc_count_direction countdir)
{
  // Check inputs
  if (_timernum > TC_INST_MAX_ID) return;
  
  // Setup timer defaults (if available)
  Tc *const tc_modules[TC_INST_NUM] = TC_INSTS;
  tc_get_config_defaults(&config_tc);

  // Configure timer parameters
  config_tc.clock_prescaler = prescale;
  config_tc.counter_size    = countersize;
  config_tc.wave_generation = wavegen;
  config_tc.count_direction = countdir;

  // Initialize timer
  tc_init(&tc_instance, tc_modules[_timernum - TC_INSTANCE_OFFSET], &config_tc);
}

void SAMDtimer::setPeriodMatch(uint32_t period, uint32_t match) {
	// Setup compares
	setCompare(0, period);
    setCompare(1, match);
}

void SAMDtimer::setCompare(uint8_t channum, uint32_t compare) {
  
  // Check inputs
  if (channum > 1) return; 

  // Setup capture channel
  config_tc.counter_16_bit.compare_capture_channel[channum] = compare;

  // Initialize comparitor
  tc_set_compare_value(&tc_instance, (tc_compare_capture_channel)channum, compare);  
}





