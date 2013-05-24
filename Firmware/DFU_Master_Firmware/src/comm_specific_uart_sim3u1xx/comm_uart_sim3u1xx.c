//------------------------------------------------------------------------------
// Copyright (c) 2013 by Silicon Laboratories.
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the Silicon Laboratories End User
// License Agreement which accompanies this distribution, and is available at
// http://developer.silabs.com/legal/version/v10/License_Agreement_v10.htm
// Original content and implementation provided by Silicon Laboratories.
//------------------------------------------------------------------------------

/*
 *  comm_uart_sim3u1xx.c
 *
 *    Revision: 1.0 (10 MAY 2013)
 *      Author: fbar / cmenke
 */

#include "global.h"

#if (MCU_FAMILY == SiM3U1 && COMM_PROTOCOL == UART)


#include <sim3u1xx.h>
#include <SI32_PBSTD_A_Type.h>
#include <SI32_PBCFG_A_Type.h>
#include <SI32_UART_A_Type.h>
#include <SI32_CRC_A_Type.h>


//------------------------------------------------------------------------------
// Exported Function Prototypes
//------------------------------------------------------------------------------
void COMM_Init(void);
void COMM_Reset_Timeout(uint32_t timeout_ms);
uint32_t COMM_Timeout(void);

uint32_t COMM_Receive(uint8_t* rx_buff, uint32_t length);
uint32_t COMM_Transmit(uint8_t* tx_buff, uint32_t length);

//------------------------------------------------------------------------------
// Static Global Variables
//------------------------------------------------------------------------------
uint32_t U32_Viewer_A, U32_Viewer_B;
uint8_t U8_Viewer_A, U8_Viewer_B;



//------------------------------------------------------------------------------
// COMM_Init
//------------------------------------------------------------------------------
void COMM_Init(void)
{
   uint8_t rx_byte;


   // Setup Crossbar and I/O for UART
   SI32_PBCFG_A_enable_crossbar_0(SI32_PBCFG_0);
   SI32_PBCFG_A_enable_xbar0h_peripherals(SI32_PBCFG_0, SI32_PBCFG_A_XBAR0H_UART0EN);

   // UART PINS TO PROPER CONFIG (TX = PB1.12, RX = PB1.13)
   SI32_PBSTD_A_set_pins_push_pull_output(SI32_PBSTD_1, 0x0001000);
   SI32_PBSTD_A_set_pins_digital_input(SI32_PBSTD_1, 0x00002000);
   SI32_PBSTD_A_write_pbskipen(SI32_PBSTD_0, 0x0000FFFF);
   SI32_PBSTD_A_write_pbskipen(SI32_PBSTD_1, 0x00000FFF);

   // Setup UART for full-duplex mode
   SI32_UART_A_enter_full_duplex_mode(SI32_UART_0);

   // SETUP TX (8-bit, 1stop, no-parity)
   SI32_UART_A_select_tx_data_length(SI32_UART_0, 8);
   SI32_UART_A_enable_tx_start_bit(SI32_UART_0);
   SI32_UART_A_enable_tx_stop_bit(SI32_UART_0);
   SI32_UART_A_disable_tx_parity_bit(SI32_UART_0);
   SI32_UART_A_select_tx_stop_bits(SI32_UART_0, SI32_UART_A_STOP_BITS_1_BIT);
   SI32_UART_A_disable_tx_signal_inversion(SI32_UART_0);
   SI32_UART_A_enable_tx(SI32_UART_0);

   // SETUP RX
   SI32_UART_A_select_rx_data_length(SI32_UART_0, 8);
   SI32_UART_A_enable_rx_start_bit(SI32_UART_0);
   SI32_UART_A_enable_rx_stop_bit(SI32_UART_0);
   SI32_UART_A_select_rx_stop_bits(SI32_UART_0, SI32_UART_A_STOP_BITS_1_BIT);
   SI32_UART_A_disable_rx_signal_inversion(SI32_UART_0);
   SI32_UART_A_select_rx_fifo_threshold_1(SI32_UART_0);
   SI32_UART_A_enable_rx(SI32_UART_0);

   // Enable Receive Autobaud
   SI32_UART_A_enable_rx_autobaud(SI32_UART_0);

   // Wait until Autobaud is configured
   // (courtesy flush to workaround a bug in the hardware)
   SI32_UART_A_flush_rx_fifo(SI32_UART_0);
   SI32_UART_A_flush_rx_fifo(SI32_UART_0);

   do
   {
      // wait for character
      while (SI32_UART_A_read_rx_fifo_count(SI32_UART_0) == 0);

      // read character
      rx_byte = SI32_UART_A_read_data_u8(SI32_UART_0);

   } while(rx_byte != 0x55);

   // Set Transmit Baud Rate equal to Receive Baud Rate
   SI32_UART_0->BAUDRATE.TBAUD = SI32_UART_0->BAUDRATE.RBAUD;

   // Transmit an acknowlegment byte back to the host
   SI32_UART_A_clear_tx_complete_interrupt(SI32_UART_0);
   SI32_UART_A_write_data_u8(SI32_UART_0, 0xAA);
   while(!SI32_UART_A_is_tx_complete(SI32_UART_0));


   // Configure Timeouts
   SysTick->LOAD  = (0x00FFFFFF);
   SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk |
                    SysTick_CTRL_ENABLE_Msk;      // Enable SysTick Timer using the core clock

   SysTick->VAL   = (0x00000000);                 // Reset SysTick Timer and clear timeout flag
}



//------------------------------------------------------------------------------
// COMM_Receive
//------------------------------------------------------------------------------
uint32_t COMM_Receive(uint8_t* rx_buff, uint32_t length)
{
   uint8_t rx_byte;
   uint8_t low_byte;

   uint32_t payload_length;
   uint16_t crc_received;

   uint32_t i;

   while(1)
   {

      //-----------------------------------------------------------
      // Start of Frame ':'
      //-----------------------------------------------------------
      do
      {
         // Read a character (blocking)
         while (SI32_UART_A_read_rx_fifo_count(SI32_UART_0) == 0);
         rx_byte = SI32_UART_A_read_data_u8(SI32_UART_0);

         // If a capital 'U' is received, respond with 0xAA
         if(rx_byte == 0x55)
         {
            while (SI32_UART_A_read_tx_fifo_count(SI32_UART_0) >= 4);
            SI32_UART_A_write_data_u8(SI32_UART_0, 0xAA);
         }

      } while (rx_byte != ':');

      // 16-bit CRC-CCITT (poly: 0x1021, init: 0xFFFF)
      SI32_CRC_A_enable_module (SI32_CRC_0);
      SI32_CRC_A_select_polynomial_16_bit_1021 (SI32_CRC_0);
      SI32_CRC_A_enable_bit_reversal(SI32_CRC_0);
      SI32_CRC_A_select_byte_mode (SI32_CRC_0);
      SI32_CRC_A_initialize_seed_to_one (SI32_CRC_0);

      SI32_CRC_A_write_data (SI32_CRC_0, rx_byte);

      //-----------------------------------------------------------
      // Sequence Number (8-bit)
      //-----------------------------------------------------------

      // Read a character (blocking)
      SysTick->VAL   = (0x00000000);  // Reset SysTick Timer and clear timeout flag
      while (SI32_UART_A_read_rx_fifo_count(SI32_UART_0) == 0)
      {
         if((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk))
         {
            return 0;
         }
      }
      rx_byte = SI32_UART_A_read_data_u8(SI32_UART_0);

      SI32_CRC_A_write_data (SI32_CRC_0, rx_byte);


      //-----------------------------------------------------------
      // Payload Length (16-bit)
      //-----------------------------------------------------------

      // Read a character (blocking)
      SysTick->VAL   = (0x00000000);  // Reset SysTick Timer and clear timeout flag
      while (SI32_UART_A_read_rx_fifo_count(SI32_UART_0) == 0)
      {
         if((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk))
         {
            return 0;
         }
      }
      rx_byte = SI32_UART_A_read_data_u8(SI32_UART_0);

      SI32_CRC_A_write_data (SI32_CRC_0, rx_byte);

      // Low Byte
      low_byte = rx_byte;


      // Read a character (blocking)
      SysTick->VAL   = (0x00000000);  // Reset SysTick Timer and clear timeout flag
      while (SI32_UART_A_read_rx_fifo_count(SI32_UART_0) == 0)
      {
         if((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk))
         {
            return 0;
         }
      }
      rx_byte = SI32_UART_A_read_data_u8(SI32_UART_0);

      SI32_CRC_A_write_data (SI32_CRC_0, rx_byte);

       // High Byte
      payload_length = rx_byte;
      payload_length *= 256;
      payload_length += low_byte;

      U8_Viewer_A = rx_byte;
      U8_Viewer_B = low_byte;

      if(payload_length > length)
      {
         U32_Viewer_A = payload_length;
         U32_Viewer_B = length;
         continue;
      }

      //-----------------------------------------------------------
      // Payload
      //-----------------------------------------------------------

      for(i = 0; i < payload_length && i < length; i++)
      {
         // Read a character (blocking)
         SysTick->VAL   = (0x00000000);  // Reset SysTick Timer and clear timeout flag
         while (SI32_UART_A_read_rx_fifo_count(SI32_UART_0) == 0)
         {
            if((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk))
            {
               return 0;
            }
         }
         rx_byte = SI32_UART_A_read_data_u8(SI32_UART_0);

         rx_buff[i] = rx_byte;

         SI32_CRC_A_write_data (SI32_CRC_0, rx_byte);

      }

      //-----------------------------------------------------------
      // Verify CRC
      //-----------------------------------------------------------

      // Read a character (blocking)
      SysTick->VAL   = (0x00000000);  // Reset SysTick Timer and clear timeout flag
      while (SI32_UART_A_read_rx_fifo_count(SI32_UART_0) == 0)
      {
         if((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk))
         {
            return 0;
         }
      }
      rx_byte = SI32_UART_A_read_data_u8(SI32_UART_0);

      crc_received = rx_byte;

      // Read a character (blocking)
      SysTick->VAL   = (0x00000000);  // Reset SysTick Timer and clear timeout flag
      while (SI32_UART_A_read_rx_fifo_count(SI32_UART_0) == 0)
      {
         if((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk))
         {
            return 0;
         }
      }
      rx_byte = SI32_UART_A_read_data_u8(SI32_UART_0);

      crc_received |= ((uint16_t)rx_byte << 8);

      if (SI32_CRC_A_read_result (SI32_CRC_0) != crc_received)
      {
         // CRC Failed -- Transmit NACK Continue at top of while loop
         while (SI32_UART_A_read_tx_fifo_count(SI32_UART_0) >= 4);
         SI32_UART_A_write_data_u8(SI32_UART_0, 0xFF);
         continue;
      } else
      {
         // CRC Passed -- Transmit ACK and break out of while loop
         while (SI32_UART_A_read_tx_fifo_count(SI32_UART_0) >= 4);
         SI32_UART_A_write_data_u8(SI32_UART_0, 0x00);
         break;
      }
   }

   return i;
}

//------------------------------------------------------------------------------
// COMM_Transmit
//------------------------------------------------------------------------------
uint32_t COMM_Transmit(uint8_t* tx_buff, uint32_t length)
{
   uint32_t i;
   uint32_t crc;

   uint8_t rx_byte;
   volatile uint32_t fifo_count;

   uint32_t retransmit_tries = 3;
   uint32_t timeout = 0;

   while(1)
   {

      // Block until Buffer is Empty
      while (SI32_UART_A_read_tx_fifo_count(SI32_UART_0) > 0);

      // 16-bit CRC-CCITT (poly: 0x1021, init: 0xFFFF)
      SI32_CRC_A_enable_module (SI32_CRC_0);
      SI32_CRC_A_select_polynomial_16_bit_1021 (SI32_CRC_0);
      SI32_CRC_A_enable_bit_reversal(SI32_CRC_0);
      SI32_CRC_A_select_byte_mode (SI32_CRC_0);
      SI32_CRC_A_initialize_seed_to_one (SI32_CRC_0);

      // Load the packet header
      SI32_UART_A_write_data_u8(SI32_UART_0, ':');
      SI32_UART_A_write_data_u8(SI32_UART_0, 0x00);
      SI32_UART_A_write_data_u8(SI32_UART_0, length % 256);
      SI32_UART_A_write_data_u8(SI32_UART_0, length / 256);

      SI32_CRC_A_write_data (SI32_CRC_0, ':');
      SI32_CRC_A_write_data (SI32_CRC_0, 0x00);
      SI32_CRC_A_write_data (SI32_CRC_0, length % 256);
      SI32_CRC_A_write_data (SI32_CRC_0, length / 256);

      // Block if the output buffer is greater than 1
      while (SI32_UART_A_read_tx_fifo_count(SI32_UART_0) > 1);

      for(i = 0; i < length; i++)
      {
         // Block if the output buffer is greater than 1
         while (SI32_UART_A_read_tx_fifo_count(SI32_UART_0) > 1);

         // Write character to the output buffer
         SI32_UART_A_write_data_u8(SI32_UART_0, tx_buff[i]);

         // Write character to CRC engine
         SI32_CRC_A_write_data (SI32_CRC_0, tx_buff[i]);

      }

      // Block if the output buffer is greater than 1
      //while (SI32_UART_A_read_tx_fifo_count(SI32_UART_0) > 2);

      // Transmit the CRC
      crc = SI32_CRC_A_read_result (SI32_CRC_0);
      SI32_UART_A_write_data_u8(SI32_UART_0, crc % 256);
      SI32_UART_A_write_data_u8(SI32_UART_0, crc / 256);

      // Wait for ACK
      // Read a character (blocking)
      SysTick->VAL   = (0x00000000);  // Reset SysTick Timer and clear timeout flag
      while (SI32_UART_A_read_rx_fifo_count(SI32_UART_0) == 0)
      {
         timeout = (SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk);
         if(timeout)
         {
            break;
         }
      }
      if (timeout)
      {
         if(retransmit_tries > 0)
         {
            retransmit_tries--;
            continue;
         } else
         {
            return 0;
         }
      }
      rx_byte = SI32_UART_A_read_data_u8(SI32_UART_0);
      //rx_byte = 0x00;

      if(rx_byte == 0x00)
      {
         break;
      }
   }

   return i;
}





void COMM_Reset_Timeout(uint32_t timeout_ms)
{

}

uint32_t COMM_Timeout(void)
{
   return 0;
}


#endif
