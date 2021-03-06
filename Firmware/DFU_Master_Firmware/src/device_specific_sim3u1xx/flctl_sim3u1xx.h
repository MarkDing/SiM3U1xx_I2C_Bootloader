//------------------------------------------------------------------------------
// Copyright (c) 2013 by Silicon Laboratories. 
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the Silicon Laboratories End User 
// License Agreement which accompanies this distribution, and is available at
// http://developer.silabs.com/legal/version/v10/License_Agreement_v10.htm
// Original content and implementation provided by Silicon Laboratories.
//------------------------------------------------------------------------------

/*
 *  flctl_sim3u1xx.h
 *
 *    Revision: 1.0 (10 MAY 2013)
 *      Author: fbar / cmenke
 */

#if (MCU_FAMILY == SiM3U1)

#include "flctl_userconfig_sim3u1xx.h"


#define FLCTL_Get_FlashKey_A()  (0x000000A5 ^ DFU_UPLOAD)
#define FLCTL_Get_FlashKey_B()  (0x000000F1 ^ DFU_UPLOAD)

#define FLCTL_Get_Sector_Size()  1024
#define FLCTL_Get_Lock_Word_Address()  0x0003FFFC

#endif

