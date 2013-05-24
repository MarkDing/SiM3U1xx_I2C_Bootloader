//------------------------------------------------------------------------------
// Copyright (c) 2013 by Silicon Laboratories. 
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the Silicon Laboratories End User 
// License Agreement which accompanies this distribution, and is available at
// http://developer.silabs.com/legal/version/v10/License_Agreement_v10.htm
// Original content and implementation provided by Silicon Laboratories.
//------------------------------------------------------------------------------

/*
 *  flctl.h
 *
 *    Revision: 1.0 (10 MAY 2013)
 *      Author: fbar / cmenke
 */


//------------------------------------------------------------------------------
// Device Specific Macros and Constants
//------------------------------------------------------------------------------
#if (MCU_FAMILY == SiMXXX)
   #include "..\device_specific_simxxxxx\flctl_simxxxxx.h"
#elif (MCU_FAMILY == SiM3U1)
   #include "..\device_specific_sim3u1xx\flctl_sim3u1xx.h"
#elif (MCU_FAMILY == SiM3C1)
   #include "..\device_specific_sim3c1xx\flctl_sim3c1xx.h"
#elif (MCU_FAMILY == SiM3L1)
   #include "..\device_specific_sim3l1xx\flctl_sim3l1xx.h"
#else
   #error "MCU family not recognized in FLCTL module"
#endif 


//------------------------------------------------------------------------------
// Exported Function Prototypes
//------------------------------------------------------------------------------
extern void FLCTL_Init(void);
extern uint32_t FLCTL_Write(uint8_t* buffer, uint32_t address, uint32_t length, uint32_t operation); 
extern uint32_t FLCTL_PageErase(uint32_t address, uint32_t operation); 
extern void FLCTL_SetFlashKeys(uint32_t key_A, uint32_t key_B);
extern void FLCTL_DestroyFlashKeys(void);

