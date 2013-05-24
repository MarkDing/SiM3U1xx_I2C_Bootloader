//------------------------------------------------------------------------------
// Copyright (c) 2013 by Silicon Laboratories.
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the Silicon Laboratories End User
// License Agreement which accompanies this distribution, and is available at
// http://developer.silabs.com/legal/version/v10/License_Agreement_v10.htm
// Original content and implementation provided by Silicon Laboratories.
//------------------------------------------------------------------------------

/*
 *  flctl_sim3u1xx.c
 *
 *    Revision: 1.0 (10 MAY 2013)
 *      Author: fbar / cmenke
 */

#include "global.h"

#if (MCU_FAMILY == SiM3U1)

#include <sim3u1xx.h>
#include <SI32_FLASHCTRL_A_Type.h>
#include <SI32_VMON_A_Type.h>
#include <SI32_RSTSRC_A_Type.h>

//------------------------------------------------------------------------------
// Exported Function Prototypes
//------------------------------------------------------------------------------
void FLCTL_Init(void);
void FLCTL_SetFlashKeys(uint32_t key_A, uint32_t key_B);
void FLCTL_DestroyFlashKeys(void);
uint32_t FLCTL_Write(uint8_t* buffer, uint32_t address, uint32_t length, uint32_t operation);
uint32_t FLCTL_PageErase(uint32_t address, uint32_t operation);

//------------------------------------------------------------------------------
// Module Variables
//------------------------------------------------------------------------------
uint32_t Flash_Key_A;
uint32_t Flash_Key_B;


//------------------------------------------------------------------------------
// FLCTL_Init
//------------------------------------------------------------------------------
void FLCTL_Init(void)
{
   // Destroy the Flash Keys
   FLCTL_DestroyFlashKeys();

   // 1. Enable VDD Supply Monitor and set as a reset source
   SI32_VMON_A_enable_vdd_supply_monitor(SI32_VMON_0);
   SI32_RSTSRC_A_enable_vdd_monitor_reset_source(SI32_RSTSRC_0);

}

void FLCTL_SetFlashKeys(uint32_t key_A, uint32_t key_B)
{
   Flash_Key_A = key_A ^ DFU_DNLOAD;
   Flash_Key_B = key_B ^ DFU_DNLOAD;
}

void FLCTL_DestroyFlashKeys(void)
{
   Flash_Key_A = 0x00000000;
   Flash_Key_B = 0x00000000;
}

//------------------------------------------------------------------------------
// FLCTL_Write
//------------------------------------------------------------------------------
uint32_t FLCTL_Write(uint8_t* buffer, uint32_t address, uint32_t length, uint32_t operation)
{
   uint32_t i;
   uint16_t data_to_write;

   #if (CHECK_FLASH_ADDRESS_ALIGNMENT)
   // Verify that address is a multiple of 2
   if(address & 0x0001) return 0;
   #endif

   #if (CHECK_FLASH_WRITELENGTH_ALIGNMENT)
   // Verify that length is a multiple of 2
   if(length & 0x0001) return 0;
   #endif

   for( i = 0; i < length; i += 2)
   {
      // Determine the data to be written to the half-word
      data_to_write = *((uint16_t*) buffer);

      // Handle byte-aligned data
      if(i + 1 >= length)
      {
         // Keep only the LSB and clear the MSB to 0xFF (uninitialized Flash)
         data_to_write |= 0xFF00;
      }


      // Write the address of the half-word to WRADDR
      SI32_FLASHCTRL_A_write_wraddr(SI32_FLASHCTRL_0, address);

      // Unlock the flash for a single write/erase operation
      SI32_FLASHCTRL_A_write_flash_key(SI32_FLASHCTRL_0, Flash_Key_A ^ operation);
      SI32_FLASHCTRL_A_write_flash_key(SI32_FLASHCTRL_0, Flash_Key_B ^ operation);

      // Initiate the write
      SI32_FLASHCTRL_A_write_wrdata(SI32_FLASHCTRL_0, data_to_write );

      // (optional) poll BUSYF if executing code from other than Flash Memory
      // We are executing code from Flash, so no need to poll.

      // Advance the buffer and the address
      buffer += 2;
      address += 2;
   }
   return i;
}

//------------------------------------------------------------------------------
// FLCTL_PageErase
//------------------------------------------------------------------------------
uint32_t FLCTL_PageErase(uint32_t address, uint32_t operation)
{
   // Write the address of the flash page to WRADDR
   SI32_FLASHCTRL_A_write_wraddr(SI32_FLASHCTRL_0, address);

   // Enter flash erase mode
   SI32_FLASHCTRL_A_enter_flash_erase_mode(SI32_FLASHCTRL_0);

   // Unlock the flash for a single write/erase operation
   SI32_FLASHCTRL_A_write_flash_key(SI32_FLASHCTRL_0, 0xA5);
   SI32_FLASHCTRL_A_write_flash_key(SI32_FLASHCTRL_0, 0xF1);

   // Initiate the page erase
   SI32_FLASHCTRL_A_write_wrdata(SI32_FLASHCTRL_0, 0x0000);

   // (optional) poll BUSYF if executing code from other than Flash Memory
   // We are executing code from Flash, so no need to poll.

   // Exit flash erase mode
   SI32_FLASHCTRL_A_exit_flash_erase_mode(SI32_FLASHCTRL_0);


   return OK;
}

#endif  // EOF
