//------------------------------------------------------------------------------
// Copyright (c) 2013 by Silicon Laboratories.
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the Silicon Laboratories End User
// License Agreement which accompanies this distribution, and is available at
// http://developer.silabs.com/legal/version/v10/License_Agreement_v10.htm
// Original content and implementation provided by Silicon Laboratories.
//------------------------------------------------------------------------------

/**
 *  \file comm_i2c_sim3u1xx.c
 *  \brief I2C communication code
 *
 *  \version: 1.0
 *  \date: 31 May 2013
 *  \author: Mark Ding
 */

#include "global.h"

#if (MCU_FAMILY == SiM3U1 && COMM_PROTOCOL == I2C)

#include <sim3u1xx.h>
#include <SI32_PBSTD_A_Type.h>
#include <SI32_PBCFG_A_Type.h>
#include <SI32_I2C_A_Type.h>
#include <SI32_CRC_A_Type.h>

/**
 * \def I2C_ADDRESS
 * \brief I2C slave address 0xF0
 */
#define I2C_ADDRESS 0xF0
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
/**
 * \fn void COMM_Init(void)
 * \brief Initialise I2C module
 *
 * Setup Crossbar and I/O for I2C. Set I2C clock rate to 400kHz,
 * Configure timeout with SysTick Timer
 */
void COMM_Init(void)
{
    // Setup Crossbar and I/O for I2C
    SI32_PBCFG_A_enable_crossbar_0(SI32_PBCFG_0);
    SI32_PBCFG_A_enable_xbar0l_peripherals(SI32_PBCFG_0, SI32_PBCFG_A_XBAR0L_I2C0EN);

    // I2C PIN SETUP
    // initialize PB0.14 (SDA) and PB0.15 (SCL) as digital input
    SI32_PBSTD_A_write_pbskipen(SI32_PBSTD_0, 0x3FFF);

    // SETUP MODULE
    // Fi2c = Fapb / (64 - SCALER) = 20Mhz
    SI32_I2C_A_set_scaler_value(SI32_I2C_0, 0x3F);
    // Tscl_low = (256-SCLL)/Fi2c = 1.2uS
    SI32_I2C_A_set_scl_low_period_reload(SI32_I2C_0, 0xE8);
    // Tscl_high = (256-T1RL)/Fi2c = 1.2uS
    SI32_I2C_A_set_timer1_reload(SI32_I2C_0, 0xE8);
    // I2C speed = 1 /(1.2+1.2) = 416kHz

    // Tbus_free = (256 - T0RL)/Fi2c = 12.75uS
    SI32_I2C_A_set_timer0_u8 (SI32_I2C_0, 0x00);
    SI32_I2C_A_set_timer0_reload (SI32_I2C_0, 0x01);

    // Tscl_to = (2^20 - (16x[T3RL:T2RL]))/Fi2c = 27.2384mS
    SI32_I2C_A_set_timer2_reload(SI32_I2C_0, 0x00);
    SI32_I2C_A_set_timer3_reload(SI32_I2C_0, 0x7B);

    // set SETUP time to non-zero value for repeated starts to function correctly
    // Tsetup = (17-SETUP)/Fi2c = 0.8uS
    SI32_I2C_A_set_extended_data_setup_time(SI32_I2C_0, 0x01);

    // ENABLE MODULE
    SI32_I2C_A_enable_module(SI32_I2C_0);

    // Configure Timeouts
    SysTick->LOAD = (0x00FFFFFF);
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk; // Enable SysTick Timer using the core clock

    SysTick->VAL = (0x00000000); // Reset SysTick Timer and clear timeout flag
}

/** \fn int32_t I2C_hander(uint8_t *buf, uint32_t count)
 *  \brief  I2C communication handler
 *
 *  It handlers both transmit and receive, read/write data in buf in
 *  parameter.
 *
 * \note I2C STAI comes with ACKI, so we need clear ACKI
 * \note I2C STOI comes with RXI, so we need to clear RXI
 * \note I2C TXI and RXI happens, the TXARM and RXARM has been cleared. Set them to 1 for next transition
 * \note Last byte of TXI, we don't need set TXARM = 1.
 * \param buf buffer for transmit and receive data
 * \param count number of bytes to transfer or receive
 * \return 0: success; 1: error
 */
int32_t I2C_hander(uint8_t *buf, uint32_t count)
{
    uint8_t * ptr = buf;
    uint8_t I2C_data_ready = 0;

    if (count == 0)
        return false;
    do {
        if (SI32_I2C_A_is_start_interrupt_pending(SI32_I2C_0)) { //< I2C start
            uint32_t tmp = SI32_I2C_A_read_data(SI32_I2C_0);
            if ((tmp & 0xFE) == I2C_ADDRESS) {
                SI32_I2C_A_send_ack(SI32_I2C_0); // send an ACK
                SI32_I2C_A_set_byte_count(SI32_I2C_0, 1); // set bytes count(BC)
                if (tmp & 0x01) {
                    SI32_I2C_A_write_data(SI32_I2C_0,*ptr++);
                    SI32_I2C_A_arm_tx(SI32_I2C_0); // Arm transmission(TXARM=1)
                    count--;
                } else {
                    SI32_I2C_A_arm_rx(SI32_I2C_0); // Arm reception(RXARM=1)
                }
            } else {
                SI32_I2C_A_send_nack(SI32_I2C_0); // send an NACK
            }
            // Start bit comes with ACKI
            SI32_I2C_0->CONTROL_CLR = SI32_I2C_A_CONTROL_STAI_MASK | SI32_I2C_A_CONTROL_STA_MASK
                    | SI32_I2C_A_CONTROL_ACKI_MASK;
        }

        if (SI32_I2C_A_is_stop_interrupt_pending(SI32_I2C_0)) { // I2C stop
            SI32_I2C_0->CONTROL_CLR = SI32_I2C_A_CONTROL_STO_MASK | SI32_I2C_A_CONTROL_STOI_MASK;
            if (count)
                break;
            I2C_data_ready = 1;
        }

        if (SI32_I2C_A_is_ack_interrupt_pending(SI32_I2C_0))// Acknowledge for each byte
        {
            SI32_I2C_A_clear_ack_interrupt(SI32_I2C_0);
        }

        if (SI32_I2C_A_is_rx_interrupt_pending(SI32_I2C_0)) { // I2C data receive
            if (count) {
                *ptr++ = SI32_I2C_A_read_data(SI32_I2C_0);
                SI32_I2C_A_send_ack(SI32_I2C_0); // send an ACK
                count--;
            } else {
                SI32_I2C_A_send_nack(SI32_I2C_0); // send an NACK
            }
            if(count)
                SI32_I2C_A_arm_rx(SI32_I2C_0); // Arm reception(RXARM=1)
            SI32_I2C_A_clear_rx_interrupt(SI32_I2C_0);
            SI32_I2C_A_clear_ack_interrupt(SI32_I2C_0);
        }

        if (SI32_I2C_A_is_tx_interrupt_pending(SI32_I2C_0)) { // I2C data transmit
            if (SI32_I2C_A_is_ack_received(SI32_I2C_0)) {
                if (count) {
                    SI32_I2C_A_write_data(SI32_I2C_0,*ptr++);
                    SI32_I2C_A_arm_tx(SI32_I2C_0); // Arm transmission(TXARM=1)
                    count--;
                }
            }
            SI32_I2C_A_clear_ack_interrupt(SI32_I2C_0);
            SI32_I2C_A_clear_tx_interrupt(SI32_I2C_0);
        }

        if (SI32_I2C_A_is_timer3_interrupt_pending(SI32_I2C_0)) {
            SI32_I2C_A_clear_timer3_interrupt(SI32_I2C_0);
             break;
        }
        if (SI32_I2C_A_is_arblost_interrupt_pending(SI32_I2C_0)) {
            SI32_I2C_A_clear_arblost_interrupt(SI32_I2C_0);
            SI32_I2C_A_reset_module(SI32_I2C_0);
            COMM_Init();
            break;
        }
    } while ((I2C_data_ready == 0));
    return (I2C_data_ready ? 0 : -1);
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

        }while (rx_byte != ':');

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
        SysTick->VAL = (0x00000000); // Reset SysTick Timer and clear timeout flag
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
        SysTick->VAL = (0x00000000); // Reset SysTick Timer and clear timeout flag
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
        SysTick->VAL = (0x00000000); // Reset SysTick Timer and clear timeout flag
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
            SysTick->VAL = (0x00000000); // Reset SysTick Timer and clear timeout flag
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
        SysTick->VAL = (0x00000000); // Reset SysTick Timer and clear timeout flag
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
        SysTick->VAL = (0x00000000); // Reset SysTick Timer and clear timeout flag
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
#if 0
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
        SysTick->VAL = (0x00000000); // Reset SysTick Timer and clear timeout flag
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
#endif
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
