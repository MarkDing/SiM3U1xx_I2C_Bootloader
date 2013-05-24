//------------------------------------------------------------------------------
// Copyright (c) 2013 by Silicon Laboratories. 
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the Silicon Laboratories End User 
// License Agreement which accompanies this distribution, and is available at
// http://developer.silabs.com/legal/version/v10/License_Agreement_v10.htm
// Original content and implementation provided by Silicon Laboratories.
//------------------------------------------------------------------------------

/*
 *  device.h
 *
 *    Revision: 1.0 (10 MAY 2013)
 *      Author: fbar / cmenke
 */




//------------------------------------------------------------------------------
// Device Specific Macros and Constants
//------------------------------------------------------------------------------
#if (MCU_FAMILY == SiMXXX)
   #include "..\device_specific_simxxxxx\device_simxxxxx.h"
#elif (MCU_FAMILY == SiM3U1)
   #include "..\device_specific_sim3u1xx\device_sim3u1xx.h"
#elif (MCU_FAMILY == SiM3C1)
   #include "..\device_specific_sim3c1xx\device_sim3c1xx.h"
#elif (MCU_FAMILY == SiM3L1)
   #include "..\device_specific_sim3l1xx\device_sim3l1xx.h"
#else
   #error "MCU family not recognized in DEVICE module"
#endif 


//------------------------------------------------------------------------------
// Exported Global Variables
//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
// Exported Function Prototypes
//------------------------------------------------------------------------------
extern void DEVICE_Init(void);
extern void DEVICE_Restore(void); 
extern void DEVICE_InitializeCRC32(void);
extern void DEVICE_UpdateCRC32(uint8_t input);
extern uint32_t DEVICE_ReadCRC32Result(void);
extern void DEVICE_Fill_DeviceID_UUID(uint8_t *buffer);
extern void DEVICE_Reset(void);
extern void DEVICE_RedirectInterrupts(uint32_t address);
