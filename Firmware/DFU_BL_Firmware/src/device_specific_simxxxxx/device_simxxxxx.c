//------------------------------------------------------------------------------
// Copyright (c) 2013 by Silicon Laboratories.
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the Silicon Laboratories End User
// License Agreement which accompanies this distribution, and is available at
// http://developer.silabs.com/legal/version/v10/License_Agreement_v10.htm
// Original content and implementation provided by Silicon Laboratories.
//------------------------------------------------------------------------------

/*
 *  device_sim3u1xx.c
 *
 *    Revision: 1.0 (10 MAY 2013)
 *      Author: fbar / cmenke
 */

#include "global.h"

#if (MCU_FAMILY == SiMXXX)


//------------------------------------------------------------------------------
// Exported Global Variables
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Static Global Variables
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Exported Function Prototypes
//------------------------------------------------------------------------------
void DEVICE_Init(void);
void DEVICE_Restore(void);
void DEVICE_InitializeCRC32(void);
void DEVICE_UpdateCRC32(uint8_t input);
uint32_t DEVICE_ReadCRC32Result(void);
void DEVICE_Fill_DeviceID_UUID(uint8_t *buffer);
void DEVICE_Reset(void);
void DEVICE_RedirectInterrupts(uint32_t address);

//------------------------------------------------------------------------------
// Local Function Prototypes
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// DEVICE_Init
//------------------------------------------------------------------------------
void DEVICE_Init(void)
{
   //---------------------------------------------------------------------------
   // Perform device initialization
   //---------------------------------------------------------------------------

   //---------------------------------------------------------------------------
   // Set the flash size (excluding the lock byte)
   //---------------------------------------------------------------------------


   //---------------------------------------------------------------------------
   // Set the ram size
   //---------------------------------------------------------------------------


   //---------------------------------------------------------------------------
   // Check for internal and external triggers
   //---------------------------------------------------------------------------



   //---------------------------------------------------------------------------
   // Check for automatic triggers
   //---------------------------------------------------------------------------


}

//------------------------------------------------------------------------------
// DEVICE_Restore
//------------------------------------------------------------------------------
void DEVICE_Restore(void)
{

}


//------------------------------------------------------------------------------
// DEVICE_InitializeCRC32
//------------------------------------------------------------------------------
void DEVICE_InitializeCRC32(void)
{

}

//------------------------------------------------------------------------------
// DEVICE_UpdateCRC32
//------------------------------------------------------------------------------
void DEVICE_UpdateCRC32(uint8_t input)
{

}

//------------------------------------------------------------------------------
// DEVICE_ReadCRC32Result
//------------------------------------------------------------------------------
uint32_t DEVICE_ReadCRC32Result(void)
{

   return 0;
}

//------------------------------------------------------------------------------
// DEVICE_Fill_DeviceID_UUID
//------------------------------------------------------------------------------
//
// The DFU module will pass a 32-byte buffer to be filled by this function.
//
void DEVICE_Fill_DeviceID_UUID(uint8_t *buffer)
{


}

//------------------------------------------------------------------------------
// DEVICE_Reset
//------------------------------------------------------------------------------
//
// Perform a device reset
//
void DEVICE_Reset(void)
{

}

//------------------------------------------------------------------------------
// DEVICE_RedirectInterrupts
//------------------------------------------------------------------------------
//
// Perform necessary initialization to redirect interrupts to base address
// specified in <address>
//
void DEVICE_RedirectInterrupts(uint32_t address)
{

}
#endif  // EOF
