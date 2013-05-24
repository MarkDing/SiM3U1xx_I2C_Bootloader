//------------------------------------------------------------------------------
// Copyright (c) 2012 by Silicon Laboratories. 
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the Silicon Laboratories End User 
// License Agreement which accompanies this distribution, and is available at
// http://developer.silabs.com/legal/version/v10/License_Agreement_v10.htm
// Original content and implementation provided by Silicon Laboratories.
//------------------------------------------------------------------------------
// library
#include <stdio.h>
// hal
#include <si32_device.h>
#include <SI32_RTC_A_Type.h>
// application
#include "myRtc0.h"

//==============================================================================
// PUBLIC VARIABLES
//==============================================================================
//Variable that incrments 100 times / second
volatile uint32_t msTicks_10 = 0;

//==============================================================================
// 2nd LEVEL INTERRUPT HANDLERS
//==============================================================================
void my_rtc_fail_handler(void){
//   printf("RTC ALARM!!\n");
   while(1);
}

//------------------------------------------------------------------------------
void my_rtc_alarm0_handler(void){
   // Reset the alarm for the next 10 ms interval
   SI32_RTC_A_start_timer_capture(SI32_RTC_0);
   while(SI32_RTC_A_is_timer_capture_in_progress(SI32_RTC_0));
   SI32_RTC_A_write_alarm0(SI32_RTC_0, SI32_RTC_A_read_setcap(SI32_RTC_0)+327);

   // Increment the variable and clear the interrupt
   msTicks_10++;
   SI32_RTC_A_clear_alarm0_interrupt(SI32_RTC_0);
}

//-eof--------------------------------------------------------------------------
