//------------------------------------------------------------------------------
// Copyright (c) 2013 by Silicon Laboratories.
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the Silicon Laboratories End User
// License Agreement which accompanies this distribution, and is available at
// http://developer.silabs.com/legal/version/v10/License_Agreement_v10.htm
// Original content and implementation provided by Silicon Laboratories.
//------------------------------------------------------------------------------

/*
 *  flctl_simxxxxx.c
 *
 *    Revision: 1.0 (10 MAY 2013)
 *      Author: fbar / cmenke
 */

#include "global.h"

#if (MCU_FAMILY == SiMXXX)

//------------------------------------------------------------------------------
// Exported Function Prototypes
//------------------------------------------------------------------------------
void FLCTL_Init(void);
void FLCTL_SetFlashKeys(uint32_t key_A, uint32_t key_B);
void FLCTL_DestroyFlashKeys(void);
uint32_t FLCTL_Write(uint8_t* buffer, uint32_t address, uint32_t length, uint32_t operation);
uint32_t FLCTL_PageErase(uint32_t address, uint32_t operation);

//------------------------------------------------------------------------------
// FLCTL_Init
//------------------------------------------------------------------------------
void FLCTL_Init(void)
{

}

void FLCTL_SetFlashKeys(uint32_t key_A, uint32_t key_B)
{

}

void FLCTL_DestroyFlashKeys(void)
{

}

//------------------------------------------------------------------------------
// FLCTL_Write
//------------------------------------------------------------------------------
uint32_t FLCTL_Write(uint8_t* buffer, uint32_t address, uint32_t length, uint32_t operation)
{

   return 0;
}

//------------------------------------------------------------------------------
// FLCTL_PageErase
//------------------------------------------------------------------------------
uint32_t FLCTL_PageErase(uint32_t address, uint32_t operation)
{

   return OK;
}

#endif  // EOF
