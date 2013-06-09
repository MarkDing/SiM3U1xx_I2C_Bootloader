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
#include <SI32_PBHD_A_Type.h>
#include <SI32_PBSTD_A_Type.h>
// application
#include "gModes.h"
#include "myRtc0.h"
#include "myCpu.h"

//==============================================================================
// myApplication.
//==============================================================================
int main()
{
   // msTicks increments every 1 ms (1 kHz), driven by the low power oscillator
   // (myCpu.c)
   // msTicks_10 increments every 10 ms (100 Hz), driven by RTC0 (myRtc0.c)
   // The _last variables store the last seen value so the loop knows when to
   // update.
   uint32_t msTicks_last = 0;

   // Enter the default operating mode for this application (gModes.c)
   gModes_enter_my_default_mode();

   // Print a starting message
//   printf("hello world\n");

   // Loop forever...
   while (1)
   {
        if(msTicks_last < msTicks)
        {
					msTicks_last = msTicks + 1000;
           // Invert the state of the LED driver (P1.0)
        	SI32_PBSTD_A_toggle_pins(SI32_PBSTD_1, 1<<0);
           // Invert the state of the LED driver (P1.1)
        	SI32_PBSTD_A_toggle_pins(SI32_PBSTD_1, 1<<1);
           // Invert the state of the LED driver (P2.10)
        	SI32_PBSTD_A_toggle_pins(SI32_PBSTD_2, 1<<10);
	            // Invert the state of the LED driver (P2.11)
        	SI32_PBSTD_A_toggle_pins(SI32_PBSTD_2, 1<<11);
        }
   }// while(1)
}

//---eof------------------------------------------------------------------------
