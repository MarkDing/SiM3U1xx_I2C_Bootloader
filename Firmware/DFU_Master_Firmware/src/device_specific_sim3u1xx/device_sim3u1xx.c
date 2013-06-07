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

#if (MCU_FAMILY == SiM3U1)


#include <sim3u1xx.h>
#include <SI32_CLKCTRL_A_Type.h>
#include <SI32_PBSTD_A_Type.h>
#include <SI32_PBCFG_A_Type.h>
#include <SI32_RSTSRC_A_Type.h>
#include <SI32_WDTIMER_A_Type.h>
#include <SI32_CRC_A_Type.h>


//------------------------------------------------------------------------------
// Exported Global Variables
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Static Global Variables
//------------------------------------------------------------------------------
#if(RESET_SOURCE_CHECK_REQUIRED)
static uint32_t reset_source;
#endif

#if(TRIGGER_PIN_CHECK_REQUIRED)
static uint32_t trigger_pin;
#endif

//------------------------------------------------------------------------------
// Exported Function Prototypes
//------------------------------------------------------------------------------
void DEVICE_Init(void);
void DEVICE_Restore(void);
void DEVICE_InitializeCRC32(void);
void DEVICE_ConfigurePINs(void);
void DEVICE_UpdateCRC32(uint8_t input);
uint32_t DEVICE_ReadCRC32Result(void);
void DEVICE_Fill_DeviceID_UUID(uint8_t *buffer);
void DEVICE_Reset(void);
void DEVICE_RedirectInterrupts(uint32_t address);

//------------------------------------------------------------------------------
// Local Function Prototypes
//------------------------------------------------------------------------------
#if(RESET_SOURCE_CHECK_REQUIRED)
SI32_RSTSRC_Enum_Type get_last_reset_source(void);
#endif

//------------------------------------------------------------------------------
// DEVICE_Init
//------------------------------------------------------------------------------
void DEVICE_Init(void)
{
   uint32_t crc;
   uint32_t signature;
   uint32_t crc_golden;
   uint32_t app_length;

   //---------------------------------------------------------------------------
   // Perform device initialization
   //---------------------------------------------------------------------------

   // Disable the watchdog timer
   SI32_WDTIMER_A_stop_counter(SI32_WDTIMER_0);

   // Enable APB clock to all modules
   SI32_CLKCTRL_A_enable_apb_to_all_modules(SI32_CLKCTRL_0);

   DEVICE_ConfigurePINs();
   //---------------------------------------------------------------------------
   // Set the flash size (including the 4-byte lock word )
   //---------------------------------------------------------------------------
   if((DERIVATIVE1 & FLASH_SIZE_MASK) == FLASH_32K)
   {
      flash_size = 0x8004;

   } else
   if((DERIVATIVE1 & FLASH_SIZE_MASK) == FLASH_64K)
   {
     flash_size = 0x10004;

   } else
   if((DERIVATIVE1 & FLASH_SIZE_MASK) == FLASH_128K)
   {
      flash_size = 0x20004;

   } else
   //if((DERIVATIVE1 & FLASH_SIZE_MASK) == FLASH_256K)
   {
      flash_size = 0x40000;
   }

   //---------------------------------------------------------------------------
   // Set the ram size
   //---------------------------------------------------------------------------
   if((DERIVATIVE1 & RAM_SIZE_MASK) == RAM_8K)
   {
      ram_size = 0x2000;

   } else
   if((DERIVATIVE1 & RAM_SIZE_MASK) == RAM_16K)
   {
     ram_size = 0x4000;

   } else
   if((DERIVATIVE1 & RAM_SIZE_MASK) == RAM_24K)
   {
      ram_size = 0x6000;

   } else
   //if((DERIVATIVE1 & RAM_SIZE_MASK) == RAM_32K)
   {
      ram_size = 0x8000;
   }


   //---------------------------------------------------------------------------
   // Check for internal and external triggers
   //---------------------------------------------------------------------------

   // Clear variables
   trigger = 0;
   #if(RESET_SOURCE_CHECK_REQUIRED)
   reset_source = SI32_RESET_ERROR;
   #endif
   #if(TRIGGER_PIN_CHECK_REQUIRED)
   trigger_pin = 0;
   #endif

   // Check the last reset source if necessary
   #if (RESET_SOURCE_CHECK_REQUIRED)
      reset_source = get_last_reset_source();
   #endif

   // Read the trigger_pin if necessary
   #if (TRIGGER_PIN_CHECK_REQUIRED)

      if((DERIVATIVE1 & PACKAGE_OPTION_MASK) == PACKAGE_80_90_PIN)
      {
         if(read_trigger_pin_package_80_90_pin() == 0x0000)
         {
            trigger_pin = 1;
         }

      } else
      if((DERIVATIVE1 & PACKAGE_OPTION_MASK) == PACKAGE_64_PIN)
      {
         if(read_trigger_pin_package_64_pin() == 0x0000)
         {
            trigger_pin = 1;
         }

      } else
      //if((DERIVATIVE1 & PACKAGE_OPTION_MASK) == PACKAGE_40_PIN)
      {
         if(read_trigger_pin_package_40_pin() == 0x0000)
         {
            trigger_pin = 1;
         }
      }
      trigger_pin = 1;
   #endif

   // Check all enabled triggers
   #if (ANYRESET_AND_GPIO_TRIGGER_EN)
      if(trigger_pin)
      {
         trigger |= EXTERNAL_TRIGGER;
      }
   #endif

   #if (PINRESET_AND_GPIO_TRIGGER_EN)
      if((reset_source == SI32_PIN_RESET) && trigger_pin)
      {
         trigger |= EXTERNAL_TRIGGER;
      }
   #endif

   #if (PORRESET_AND_GPIO_TRIGGER_EN)
      if((reset_source == SI32_POR_RESET) && trigger_pin)
      {
         trigger |= EXTERNAL_TRIGGER;
      }
   #endif

   #if (PINRESET_TRIGGER_EN)
      if(reset_source == SI32_PIN_RESET)
      {
         trigger |= EXTERNAL_TRIGGER;
      }
   #endif

   #if (SYSRESETREQ_TRIGGER_EN)
      if(reset_source == SI32_CORE_RESET)
      {
         trigger |= INTERNAL_TRIGGER;
      }
   #endif

   #if (SWRESET_TRIGGER_EN)
      if(reset_source == SI32_SW_RESET)
      {
         trigger |= INTERNAL_TRIGGER;
      }
   #endif

   #if (SWRESET_AND_RAMADDR0_TRIGGER_EN)
      if((reset_source == SI32_SW_RESET)  &&
         ((RAMADDR0 & RAMADDR0_TRIGGER_MASK) == RAMADDR0_TRIGGER_VALUE))
      {
         trigger |= INTERNAL_TRIGGER;
      }
   #endif

   //---------------------------------------------------------------------------
   // Check for automatic triggers
   //
   //
   //---------------------------------------------------------------------------

   /////////////////////////////////////////////////////////
   // Stack Pointer and Reset Vector Check
   //
   #if (BOUNDS_CHECKING == ADVANCED)
      // Verifiy that the stack pointer is pointing to RAM
      if(USER_APP_STACKPTR_INITVAL < RAMADDR0_ADDR ||
         USER_APP_STACKPTR_INITVAL > (RAMADDR0_ADDR + ram_size))
      {
         trigger |= AUTO_TRIGGER;
      }

      // Verifiy that the stack pointer is pointing to RAM
      if(USER_APP_RESETVECT_INITVAL <  USER_APP_START_ADDR ||
         USER_APP_RESETVECT_INITVAL >  flash_size)
      {
         trigger |= AUTO_TRIGGER;
      }
   #else
      if(USER_APP_STACKPTR_INITVAL == 0xFFFFFFFF ||
         USER_APP_RESETVECT_INITVAL == 0xFFFFFFFF)
      {
         trigger |= AUTO_TRIGGER;
      }
   #endif

   /////////////////////////////////////////////////////////
   // Signature Check
   //
   // <flash_size> contains number of bytes up to and including the lock word
   //
   // The configuration word is the last 4 bytes of valid user Flash
   //
   // A valid configuration word is 0xA5xxyy5A and is stored in Flash
   // using little endian format.  xxyy is 0xFFFF for unconfigured state.
   //
   //      LSB              ....              MSB
   //      0x5A       yy         xx           0xA5
   //      BIT7-0    BIT15-8    BIT24-16     BIT31-25
   //
   signature = (*((uint32_t *) (flash_size - 8)));

   if((signature & 0xFF0000FF) == 0xA500005A)
   {

      /////////////////////////////////////////////////////////
      // CRC Check
      //

      crc_golden = (*((uint32_t *) (flash_size - 12)));
      app_length = (*((uint32_t *) (flash_size - 16)));

      // Initialize Hardware CRC Engine
      DEVICE_InitializeCRC32();

      if(app_length <= (flash_size - USER_APP_START_ADDR - SIZEOF_DFU_Flash_Type))
      {
         for(int i = 0; i < app_length; i++)
         {
           DEVICE_UpdateCRC32 (*((uint8_t *) (USER_APP_START_ADDR + i)));
         }

         crc = DEVICE_ReadCRC32Result();

         if(crc != crc_golden)
         {
            trigger |= AUTO_TRIGGER;
         }

      } else
      {
        trigger |= AUTO_TRIGGER;
      }

   } else
   {
      trigger |= AUTO_TRIGGER;
   }
}

//------------------------------------------------------------------------------
// DEVICE_Restore
//------------------------------------------------------------------------------
void DEVICE_Restore(void)
{
   // Start the watchdog timer
   SI32_WDTIMER_A_start_counter (SI32_WDTIMER_0);

   // Restore CRC Registers
   SI32_CRC_A_disable_module (SI32_CRC_0);
   SI32_CRC_A_select_polynomial_32_bit_04C11DB7 (SI32_CRC_0);
   SI32_CRC_A_disable_bit_reversal(SI32_CRC_0);
   SI32_CRC_A_select_word_mode (SI32_CRC_0);
   SI32_CRC_A_initialize_seed_to_one (SI32_CRC_0);

   // Restore APB clock gates to reset value
   SI32_CLKCTRL_0->APBCLKG0.U32 = 0x00000000;
   SI32_CLKCTRL_0->APBCLKG1.U32 = 0x00000002;
}

void DEVICE_ConfigurePINs(void)
{
    // Setup Crossbar and I/O for UART/I2C
    SI32_PBCFG_A_enable_crossbar_0(SI32_PBCFG_0);
    SI32_PBCFG_A_enable_xbar0l_peripherals(SI32_PBCFG_0, SI32_PBCFG_A_XBAR0L_I2C0EN);
    SI32_PBCFG_A_enable_xbar0h_peripherals(SI32_PBCFG_0, SI32_PBCFG_A_XBAR0H_UART0EN);

    // UART PINS TO PROPER CONFIG (TX = PB1.12, RX = PB1.13) SMV = PB1.3
    // initialize PB0.0 (SDA) and PB0.2 (SCL) as digital input
    SI32_PBSTD_A_set_pins_push_pull_output(SI32_PBSTD_1, 0x0001008);
    SI32_PBSTD_A_set_pins_digital_input(SI32_PBSTD_1, 0x00002000);
    SI32_PBSTD_A_write_pbskipen(SI32_PBSTD_0, 0x0000FFFA);
    SI32_PBSTD_A_write_pbskipen(SI32_PBSTD_1, 0x00000FFF);
}
//------------------------------------------------------------------------------
// DEVICE_InitializeCRC32
//------------------------------------------------------------------------------
void DEVICE_InitializeCRC32(void)
{
   // 32-bit CRC-32 (poly: 0x04C11DB7, init: 0xFFFFFFFF, XOR final with 0xFFFFFFFF)
   SI32_CRC_A_enable_module (SI32_CRC_0);
   SI32_CRC_A_select_polynomial_32_bit_04C11DB7 (SI32_CRC_0);
   SI32_CRC_A_disable_bit_reversal(SI32_CRC_0);
   SI32_CRC_A_select_byte_mode (SI32_CRC_0);
   SI32_CRC_A_initialize_seed_to_one (SI32_CRC_0);
}

//------------------------------------------------------------------------------
// DEVICE_UpdateCRC32
//------------------------------------------------------------------------------
void DEVICE_UpdateCRC32(uint8_t input)
{
   SI32_CRC_A_write_data(SI32_CRC_0, input);
}

//------------------------------------------------------------------------------
// DEVICE_ReadCRC32Result
//------------------------------------------------------------------------------
uint32_t DEVICE_ReadCRC32Result(void)
{
   uint32_t crc;

   // Hardware is backwards for the CRC-32 polynomial
   crc = SI32_CRC_A_read_bit_reversed_result(SI32_CRC_0);

   // XOR result to get final CRC-32 value
   crc ^= 0xFFFFFFFF;

   return crc;
}


//------------------------------------------------------------------------------
// DEVICE_Fill_DeviceID_UUID
//------------------------------------------------------------------------------
//
// The DFU module will pass a 32-byte buffer to be filled by this function.
//
void DEVICE_Fill_DeviceID_UUID(uint8_t *buffer)
{
   int i = 0;

   buffer[i++] = *((uint8_t*) &SI32_DEVICEID_0->DEVICEID3.U32 + 0);
   buffer[i++] = *((uint8_t*) &SI32_DEVICEID_0->DEVICEID3.U32 + 1);
   buffer[i++] = *((uint8_t*) &SI32_DEVICEID_0->DEVICEID3.U32 + 2);
   buffer[i++] = *((uint8_t*) &SI32_DEVICEID_0->DEVICEID3.U32 + 3);

   buffer[i++] = *((uint8_t*) &SI32_DEVICEID_0->DEVICEID2.U32 + 0);
   buffer[i++] = *((uint8_t*) &SI32_DEVICEID_0->DEVICEID2.U32 + 1);
   buffer[i++] = *((uint8_t*) &SI32_DEVICEID_0->DEVICEID2.U32 + 2);
   buffer[i++] = *((uint8_t*) &SI32_DEVICEID_0->DEVICEID2.U32 + 3);

   buffer[i++] = *((uint8_t*) &SI32_DEVICEID_0->DEVICEID1.U32 + 0);
   buffer[i++] = *((uint8_t*) &SI32_DEVICEID_0->DEVICEID1.U32 + 1);
   buffer[i++] = *((uint8_t*) &SI32_DEVICEID_0->DEVICEID1.U32 + 2);
   buffer[i++] = *((uint8_t*) &SI32_DEVICEID_0->DEVICEID1.U32 + 3);

   buffer[i++] = *((uint8_t*) &SI32_DEVICEID_0->DEVICEID0.U32 + 0);
   buffer[i++] = *((uint8_t*) &SI32_DEVICEID_0->DEVICEID0.U32 + 1);
   buffer[i++] = *((uint8_t*) &SI32_DEVICEID_0->DEVICEID0.U32 + 2);
   buffer[i++] = *((uint8_t*) &SI32_DEVICEID_0->DEVICEID0.U32 + 3);

   #define UUID_BASE_ADDR 0x00040380

   buffer[i++] = *((uint8_t*) UUID_BASE_ADDR + 0);
   buffer[i++] = *((uint8_t*) UUID_BASE_ADDR + 1);
   buffer[i++] = *((uint8_t*) UUID_BASE_ADDR + 2);
   buffer[i++] = *((uint8_t*) UUID_BASE_ADDR + 3);

   buffer[i++] = *((uint8_t*) UUID_BASE_ADDR + 4);
   buffer[i++] = *((uint8_t*) UUID_BASE_ADDR + 5);
   buffer[i++] = *((uint8_t*) UUID_BASE_ADDR + 6);
   buffer[i++] = *((uint8_t*) UUID_BASE_ADDR + 7);

   buffer[i++] = *((uint8_t*) UUID_BASE_ADDR + 8);
   buffer[i++] = *((uint8_t*) UUID_BASE_ADDR + 9);
   buffer[i++] = *((uint8_t*) UUID_BASE_ADDR + 10);
   buffer[i++] = *((uint8_t*) UUID_BASE_ADDR + 11);

   buffer[i++] = *((uint8_t*) UUID_BASE_ADDR + 12);
   buffer[i++] = *((uint8_t*) UUID_BASE_ADDR + 13);
   buffer[i++] = *((uint8_t*) UUID_BASE_ADDR + 14);
   buffer[i++] = *((uint8_t*) UUID_BASE_ADDR + 15);

}

//------------------------------------------------------------------------------
// DEVICE_Reset
//------------------------------------------------------------------------------
//
// Perform a device reset
//
void DEVICE_Reset(void)
{
   // System clock is running at COMM_GET_AHB_CLOCK()
   // Configure systick to reload every 1 ms

   uint32_t ticks = COMM_GET_AHB_CLOCK() / 1000;

   SysTick->LOAD  = (ticks & SysTick_LOAD_RELOAD_Msk) - 1;      /* set reload register */
   SysTick->VAL   = 0;                                          /* Load the SysTick Counter Value */
   SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk |
                    SysTick_CTRL_ENABLE_Msk;                    /* Enable SysTick Timer using the core clock */

   // Wait 10 ms for the comm interface to ack the reset command
   // before resetting the device so the host doesn't see an error
   for (uint32_t i = 0; i < 10; i++)
   {
      // Wait for the count flag to go high (every 1 ms)
      // Reading the control register clears the bit
      while ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0);
   }

   SI32_RSTSRC_A_generate_software_reset(SI32_RSTSRC_0);
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
   SCB->VTOR = address;
}

//-----------------------------------------------------------------------------
// get_last_reset_source
//-----------------------------------------------------------------------------
#if(RESET_SOURCE_CHECK_REQUIRED)
SI32_RSTSRC_Enum_Type get_last_reset_source(void)
{

   SI32_RSTSRC_Enum_Type last_reset_source;
   SI32_RSTSRC_A_Type * basePointer = SI32_RSTSRC_0;

   // First Check for a POR
   // If this bit is set, all other flags are indeterminate
   if (basePointer->RESETFLAG.PORRF ==
       SI32_RSTSRC_A_RESETFLAG_PORRF_SET_VALUE)
   {
      last_reset_source = SI32_POWER_ON_RESET;
   }
   // Now check VMON
   // If this bit is set, all other flags are indeterminate
   else if (basePointer->RESETFLAG.VMONRF ==
            SI32_RSTSRC_A_RESETFLAG_VMONRF_SET_VALUE)
   {
      last_reset_source = SI32_VDD_MON_RESET;
   }
   // If last reset was caused by neither WAKEUP nor VDD Monitor, check the
   // rest of the flags.
   else
   {
      #if (0)
         // PMU Wake
         if (basePointer->RESETFLAG.WAKERF ==
             SI32_RSTSRC_A_RESETFLAG_WAKERF_SET_VALUE)
         {
            last_reset_source = SI32_PMU_WAKEUP_RESET;
         } else
      #endif

      #if (PINRESET_AND_GPIO_TRIGGER_EN || PINRESET_TRIGGER_EN)
         // Pin
         if (basePointer->RESETFLAG.PINRF ==
                  SI32_RSTSRC_A_RESETFLAG_PINRF_SET_VALUE)
         {
            last_reset_source = SI32_PIN_RESET;
         } else
      #endif

      #if (SYSRESETREQ_TRIGGER_EN)
         // Core
         if (basePointer->RESETFLAG.CORERF ==
                  SI32_RSTSRC_A_RESETFLAG_CORERF_SET_VALUE)
         {
            last_reset_source = SI32_CORE_RESET;
         } else
      #endif

      #if (0)
         // Missing Clock Detector
         if (basePointer->RESETFLAG.MCDRF ==
                  SI32_RSTSRC_A_RESETFLAG_MCDRF_SET_VALUE)
         {
            last_reset_source = SI32_MCD_RESET;
         } else
      #endif

      #if (0)
         // Watchdog Timer
         if (basePointer->RESETFLAG.WDTRF ==
                  SI32_RSTSRC_A_RESETFLAG_WDTRF_SET_VALUE)
         {
            last_reset_source = SI32_WDT_RESET;
         } else
      #endif

      #if (SWRESET_TRIGGER_EN || SWRESET_AND_RAMADDR0_TRIGGER_EN)
         // Software Reset
         if (basePointer->RESETFLAG.SWRF ==
                  SI32_RSTSRC_A_RESETFLAG_SWRF_SET_VALUE)
         {
            last_reset_source = SI32_SW_RESET;
         } else
      #endif

      #if (0)
         // Comparator 0
         if (basePointer->RESETFLAG.CMP0RF ==
                  SI32_RSTSRC_A_RESETFLAG_CMP0RF_SET_VALUE)
         {
            last_reset_source = SI32_CMP0_RESET;
         } else
      #endif

      #if (0)
         // Comparator 1
         if (basePointer->RESETFLAG.CMP1RF ==
                  SI32_RSTSRC_A_RESETFLAG_CMP1RF_SET_VALUE)
         {
            last_reset_source = SI32_CMP1_RESET;
         } else
      #endif

      #if (0)
         // USB0
         if (basePointer->RESETFLAG.USB0RF ==
                  SI32_RSTSRC_A_RESETFLAG_USB0RF_SET_VALUE)
         {
            last_reset_source = SI32_USB0_RESET;
         } else
      #endif

      #if (0)
         // RTC0
         if (basePointer->RESETFLAG.RTC0RF ==
                  SI32_RSTSRC_A_RESETFLAG_RTC0RF_SET_VALUE)
         {
            last_reset_source = SI32_RTC0_RESET;
         } else
      #endif

      // Error Condition
      {
         last_reset_source = SI32_RESET_ERROR;
      }
   }

   return last_reset_source;
}
#endif


#endif  // EOF

