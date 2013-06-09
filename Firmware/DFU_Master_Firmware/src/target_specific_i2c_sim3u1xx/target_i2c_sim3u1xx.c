//------------------------------------------------------------------------------
// Copyright (c) 2013 by Silicon Laboratories.
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the Silicon Laboratories End User
// License Agreement which accompanies this distribution, and is available at
// http://developer.silabs.com/legal/version/v10/License_Agreement_v10.htm
// Original content and implementation provided by Silicon Laboratories.
//------------------------------------------------------------------------------

/**
 *  \file target_i2c_sim3u1xx.c
 *  \brief I2C communication code
 *
 *  \version: 1.0
 *  \date: 31 May 2013
 *  \author: Mark Ding
 */

#include "global.h"
#if (MCU_FAMILY == SiM3U1 && COMM_PROTOCOL == UART)

#include <sim3u1xx.h>
#include <SI32_PBSTD_A_Type.h>
#include <SI32_PBCFG_A_Type.h>
#include <SI32_I2C_A_Type.h>
#include <SI32_CRC_A_Type.h>
#include <target_i2c_sim3u1xx.h>
/**
 * \def I2C_ADDRESS
 * \brief I2C slave address 0xF0
 */
#define I2C_ADDRESS 0xF0

void target_comm_init(void)
{
    // SETUP MODULE
    // Fi2c = Fapb / (64 - SCALER) = 20Mhz
    SI32_I2C_A_set_scaler_value(SI32_I2C_0, 0x3F);
    // Tscl_low = (256-SCLL)/Fi2c = 5uS    (232 = 1.2uS)
    SI32_I2C_A_set_scl_low_period_reload(SI32_I2C_0, 156);
    // Tscl_high = (256-T1RL)/Fi2c = 5uS   (232 = 1.2uS)
    SI32_I2C_A_set_timer1_reload(SI32_I2C_0, 156);
    // I2C speed = 1 /(5 + 5) = 100kHz

    // Tbus_free = (256 - T0RL)/Fi2c = 12.75uS
    SI32_I2C_A_set_timer0_u8 (SI32_I2C_0, 0x00);
    SI32_I2C_A_set_timer0_reload (SI32_I2C_0, 0x01);

    // Tscl_to = (2^20 - (16x[T3RL:T2RL]))/Fi2c = 27.2384mS
    SI32_I2C_A_set_timer2_reload(SI32_I2C_0, 0x00);
    SI32_I2C_A_set_timer3_reload(SI32_I2C_0, 0x7B);

    // set SETUP time to non-zero value for repeated starts to function correctly
    // Tsetup = (17-SETUP)/Fi2c = 0.8uS
    SI32_I2C_A_set_extended_data_setup_time(SI32_I2C_0, 0x01);
    // Slave Mode Inhibit
    //  SI32_I2C_A_disable_slave_mode(SI32_I2C_0);
    // ENABLE MODULE
    SI32_I2C_A_enable_module(SI32_I2C_0);

}

int32_t I2C_handler(uint8_t *buf, uint32_t count, uint8_t rw)
{
    uint8_t * ptr = buf;
    uint8_t I2C_data_ready = 0 , seconds = 0;

    if (count == 0)
        return 0;
    if( SI32_I2C_0->CONTROL.STA || SI32_I2C_0->CONTROL.STO)
        SI32_I2C_A_reset_module(SI32_I2C_0); // Reset I2C module for abnormal case
    SI32_I2C_A_set_start(SI32_I2C_0);
    do {
        if (SI32_I2C_A_is_start_interrupt_pending(SI32_I2C_0)) { // I2C start
            uint32_t tmp = I2C_ADDRESS | rw;
            SI32_I2C_A_set_byte_count(SI32_I2C_0, 1); // set bytes count(BC)
            SI32_I2C_A_write_data(SI32_I2C_0,tmp);
            SI32_I2C_A_arm_tx(SI32_I2C_0); // Arm transmission(TXARM=1)
            // Start bit comes with ACKI
            SI32_I2C_0->CONTROL_CLR = SI32_I2C_A_CONTROL_STA_MASK | SI32_I2C_A_CONTROL_STAI_MASK;
        }

        if (SI32_I2C_A_is_stop_interrupt_pending(SI32_I2C_0)) { // I2C stop
            SI32_I2C_0->CONTROL_CLR = SI32_I2C_A_CONTROL_STO_MASK | SI32_I2C_A_CONTROL_STOI_MASK;
            if (count)
                break;
            I2C_data_ready = 1;
        }

        if (SI32_I2C_A_is_ack_interrupt_pending(SI32_I2C_0)) {
            SI32_I2C_A_clear_ack_interrupt(SI32_I2C_0);
        }

        if (SI32_I2C_A_is_rx_interrupt_pending(SI32_I2C_0)) { // I2C data receive
            if (count) {
                *ptr++ = SI32_I2C_A_read_data(SI32_I2C_0);
                count--;
            }
            if (count) {
                SI32_I2C_A_send_ack(SI32_I2C_0); // send an ACK
                SI32_I2C_A_arm_rx(SI32_I2C_0); // Arm reception(RXARM=1)
            } else {
                SI32_I2C_A_send_nack(SI32_I2C_0); // send an NACK
                SI32_I2C_A_set_stop (SI32_I2C_0); // Set STO to terminte transfer
            }
            SI32_I2C_A_clear_rx_interrupt(SI32_I2C_0);
            SI32_I2C_A_clear_ack_interrupt(SI32_I2C_0);
        }

        if (SI32_I2C_A_is_tx_interrupt_pending(SI32_I2C_0)) { // I2C data transmit
            if (SI32_I2C_A_is_ack_received(SI32_I2C_0)) {
                if (count) {
                    if (rw) { // read
                        SI32_I2C_A_arm_rx(SI32_I2C_0); // Arm reception(RXARM=1)
                    } else { // write
                        SI32_I2C_A_write_data(SI32_I2C_0,*ptr++);
                        SI32_I2C_A_arm_tx(SI32_I2C_0); // Arm transmission(TXARM=1)
                        count--;
                    }
                } else {
                    SI32_I2C_A_set_stop (SI32_I2C_0); // Set STO to terminte transfer
                }
            } else { // NACK was received
                SI32_I2C_A_set_stop (SI32_I2C_0); // Set STO to terminte transfer
            }
            SI32_I2C_A_clear_tx_interrupt(SI32_I2C_0); // clear TXI
            SI32_I2C_A_clear_ack_interrupt(SI32_I2C_0); // clear ACKI
        }

        if (SI32_I2C_A_is_timer3_interrupt_pending(SI32_I2C_0)) {
            SI32_I2C_A_clear_timer3_interrupt(SI32_I2C_0);
            SI32_I2C_A_reset_module(SI32_I2C_0);
            break;
        }
        if (SI32_I2C_A_is_arblost_interrupt_pending(SI32_I2C_0)) {
            SI32_I2C_A_clear_arblost_interrupt(SI32_I2C_0);
            break;
        }
        if ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk)) {// timeout check
            SysTick->VAL = (0x00000000); // Reset SysTick Timer and clear timeout flag
            seconds++;
            if(seconds > 2) // wait for 2 seconds
                break;
        }
    } while ((I2C_data_ready == 0));

    return (I2C_data_ready ? 0 : -1);
}
#define READ 1
#define WRITE 0
#define PACKET_LENGTH 6
//------------------------------------------------------------------------------
// target_comm_receive
//------------------------------------------------------------------------------
int32_t target_comm_receive(uint8_t* rx_buff, uint32_t length)
{
    // ':',0,lsb_len, msb_len, lsb CRC, msb CRC
    uint8_t packet[PACKET_LENGTH];
    uint32_t payload_length;
    uint16_t crc_received;

    uint32_t i;
    //return length;
    while (1) {
        if (I2C_handler(packet, PACKET_LENGTH, READ))
            return -1;

        //-----------------------------------------------------------
        // Start of Frame ':'
        //-----------------------------------------------------------
        if (packet[0] != ':')
            return -1;

        // 16-bit CRC-CCITT (poly: 0x1021, init: 0xFFFF)
        SI32_CRC_A_enable_module(SI32_CRC_0);
        SI32_CRC_A_select_polynomial_16_bit_1021(SI32_CRC_0);
        SI32_CRC_A_enable_bit_reversal(SI32_CRC_0);
        SI32_CRC_A_select_byte_mode(SI32_CRC_0);
        SI32_CRC_A_initialize_seed_to_one(SI32_CRC_0);

        for (i = 0; i < (PACKET_LENGTH - 2); i++) {
            SI32_CRC_A_write_data(SI32_CRC_0, packet[i]);
        }

        payload_length = (packet[3] << 8) | (packet[2]); // data length
        crc_received = (packet[5] << 8) | packet[4];
        if (payload_length > length)
            payload_length = length;

        //-----------------------------------------------------------
        // Payload
        //-----------------------------------------------------------
        if (I2C_handler(rx_buff, payload_length, READ))
            return -1;
        for (i = 0; i < payload_length; i++) {
            SI32_CRC_A_write_data(SI32_CRC_0, rx_buff[i]);
        }

        if (SI32_CRC_A_read_result(SI32_CRC_0) != crc_received) {
            // CRC Failed -- Transmit NACK Continue at top of while loop
            packet[0] = 0xFF;
            if (I2C_handler(packet, 1, WRITE))
                return -1;
            return 0;
        } else {
            // CRC Passed -- Transmit ACK and break out of while loop
            packet[0] = 0x00;
            if (I2C_handler(packet, 1, WRITE))
                return -1;
            break;
        }
    }
    return 0;
}

//------------------------------------------------------------------------------
// target_comm_transmit
//------------------------------------------------------------------------------
int32_t target_comm_transmit(uint8_t* tx_buff, uint32_t length)
{
    uint8_t packet[PACKET_LENGTH];

    uint32_t i;
    uint32_t crc;
    //return length;
    // 16-bit CRC-CCITT (poly: 0x1021, init: 0xFFFF)
    SI32_CRC_A_enable_module(SI32_CRC_0);
    SI32_CRC_A_select_polynomial_16_bit_1021(SI32_CRC_0);
    SI32_CRC_A_enable_bit_reversal(SI32_CRC_0);
    SI32_CRC_A_select_byte_mode(SI32_CRC_0);
    SI32_CRC_A_initialize_seed_to_one(SI32_CRC_0);

    // Load the packet header
    packet[0] = ':';
    packet[1] = 0x00;
    packet[2] = length & 0xFF;
    packet[3] = (length >> 8) & 0xFF;

    for (i = 0; i < (PACKET_LENGTH - 2); i++) {
        SI32_CRC_A_write_data(SI32_CRC_0, packet[i]);
    }

    for (i = 0; i < length; i++) {
        SI32_CRC_A_write_data(SI32_CRC_0, tx_buff[i]);
    }
    // Transmit the CRC
    crc = SI32_CRC_A_read_result(SI32_CRC_0);
    packet[4] = crc & 0xFF;
    packet[5] = (crc >> 8) & 0xff;

    if (I2C_handler(packet, PACKET_LENGTH, WRITE))
        return -1;
    if (I2C_handler(tx_buff, length, WRITE))
        return -1;

    // Wait for ACK
    if (I2C_handler(packet, 1, READ))
        return -1;
    if (packet[0] == 0xFF)
        return -1;

    return 0;
}
#endif
