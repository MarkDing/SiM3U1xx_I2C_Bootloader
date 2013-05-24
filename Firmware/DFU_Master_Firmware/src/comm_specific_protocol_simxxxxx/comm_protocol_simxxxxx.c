//------------------------------------------------------------------------------
// Copyright (c) 2013 by Silicon Laboratories.
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the Silicon Laboratories End User
// License Agreement which accompanies this distribution, and is available at
// http://developer.silabs.com/legal/version/v10/License_Agreement_v10.htm
// Original content and implementation provided by Silicon Laboratories.
//------------------------------------------------------------------------------

/*
 *  comm.c
 *
 *    Revision: 1.0 (10 MAY 2013)
 *      Author: fbar / cmenke
 */

#include "global.h"

#if (MCU_FAMILY == SiMXXX && COMM_PROTOCOL == PROTOCOL)

//------------------------------------------------------------------------------
// Exported Function Prototypes
//------------------------------------------------------------------------------
void COMM_Init(void);
void COMM_Reset_Timeout(uint32_t timeout_ms);
uint32_t COMM_Timeout(void);

uint32_t COMM_Receive(uint8_t* rx_buff, uint32_t length);
uint32_t COMM_Transmit(uint8_t* tx_buff, uint32_t length);


//------------------------------------------------------------------------------
// COMM_Init
//------------------------------------------------------------------------------
void COMM_Init(void)
{

}

//------------------------------------------------------------------------------
// COMM_Receive
//------------------------------------------------------------------------------
uint32_t COMM_Receive(uint8_t* rx_buff, uint32_t length)
{
   return 0;
}

//------------------------------------------------------------------------------
// COMM_Transmit
//------------------------------------------------------------------------------
uint32_t COMM_Transmit(uint8_t* tx_buff, uint32_t length)
{
   return 0;
}


void COMM_Reset_Timeout(uint32_t timeout_ms)
{

}

uint32_t COMM_Timeout(void)
{
   return 0;
}

#endif
