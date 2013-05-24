//------------------------------------------------------------------------------
// Copyright (c) 2012 by Silicon Laboratories. 
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the Silicon Laboratories End User 
// License Agreement which accompanies this distribution, and is available at
// http://developer.silabs.com/legal/version/v10/License_Agreement_v10.htm
// Original content and implementation provided by Silicon Laboratories.
//------------------------------------------------------------------------------

#ifndef __MYRTC_H__
#define __MYRTC_H__

// Export msTicks for other modules
extern volatile uint32_t msTicks_10;

// Setup RTC0 in the applications default mode
void myRtc0_enter_default_mode(void);

#endif //__MYRTC_H__

//-eof--------------------------------------------------------------------------
