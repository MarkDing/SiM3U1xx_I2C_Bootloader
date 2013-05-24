//------------------------------------------------------------------------------
// Copyright (c) 2013 by Silicon Laboratories.
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the Silicon Laboratories End User
// License Agreement which accompanies this distribution, and is available at
// http://developer.silabs.com/legal/version/v10/License_Agreement_v10.htm
// Original content and implementation provided by Silicon Laboratories.
//------------------------------------------------------------------------------

/*
 *  comm_usb_sim3u1xx.c
 *
 *    Revision: 1.0 (10 MAY 2013)
 *      Author: fbar / cmenke
 */

#include "global.h"

#if (MCU_FAMILY == SiM3U1 && COMM_PROTOCOL == USB)


#include <sim3u1xx.h>
#include <SI32_CLKCTRL_A_Type.h>
#include <SI32_USB_A_Type.h>
#include <SI32_FLASHCTRL_A_Type.h>
#include "USB0_ISR.h"
#include "USB0_ControlRequests.h"

#ifdef DEBUG
#include <SI32_PBCFG_A_Type.h>
#include <SI32_PBSTD_A_Type.h>
#include <SI32_PBHD_A_Type.h>
#include <stdio.h>
#endif // DEBUG

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
   // High speed flash mode
   SI32_FLASHCTRL_A_select_flash_speed_mode(SI32_FLASHCTRL_0, 2);

   // Set system clock to 48 MHz
   SI32_USB_A_enable_usb_oscillator(SI32_USB_0);
   SI32_CLKCTRL_A_select_ahb_source_usb_oscillator(SI32_CLKCTRL_0);

   USB0_Init();

#ifdef DEBUG
   // Enable SWV

   // PB1 Setup
   SI32_PBSTD_A_set_pins_push_pull_output(SI32_PBSTD_1, 0x0008);
   SI32_PBSTD_A_write_pbskipen(SI32_PBSTD_1, 0x0008);

   // Enable Crossbar0 signals & set properties
   SI32_PBCFG_A_enable_crossbar_0(SI32_PBCFG_0);

   // UPDATE ITM DIVIDER
   *((uint32_t *) 0xE0040010) = ((50 * (uint32_t)48000000) / 20000000) - 1;

   printf("COMM_Init\n");
#endif // DEBUG
}

//------------------------------------------------------------------------------
// COMM_Receive
//------------------------------------------------------------------------------
uint32_t COMM_Receive(uint8_t* rx_buff, uint32_t length)
{
   uint16_t bytes = 0;

   USB0_RX_Start(rx_buff, length);

   while (!USB0_Is_RX_Complete(&bytes));

#ifdef DEBUG
   printf("Received %u bytes\n", bytes);
#endif // DEBUG

   return bytes;
}

//------------------------------------------------------------------------------
// COMM_Transmit
//------------------------------------------------------------------------------
uint32_t COMM_Transmit(uint8_t* tx_buff, uint32_t length)
{
   uint16_t bytes = 0;

   USB0_TX_Start(tx_buff, length);

   while (!USB0_Is_TX_Complete(&bytes));

#ifdef DEBUG
   printf("Transmitted %u bytes\n", bytes);
#endif // DEBUG

   return bytes;
}



#endif
