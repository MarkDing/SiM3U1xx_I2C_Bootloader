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

#include <sim3u1xx.h>
#include <SI32_PBSTD_A_Type.h>
#include <SI32_PBCFG_A_Type.h>
#include <SI32_I2C_A_Type.h>

/**
 * \def I2C_ADDRESS
 * \brief I2C slave address 0xF0
 */
#define I2C_ADDRESS 0xF0
//------------------------------------------------------------------------------
// Exported Function Prototypes
//------------------------------------------------------------------------------
void target_comm_init(void);

//------------------------------------------------------------------------------
// COMM_Init
//------------------------------------------------------------------------------
/**
 * \fn void target_comm_init(void)
 * \brief Initialise I2C module
 *
 * Setup Crossbar and I/O for I2C. Set I2C clock rate to 400kHz,
 *
 */
void target_comm_init(void)
{
    // Setup Crossbar and I/O for I2C
    SI32_PBCFG_A_enable_crossbar_0(SI32_PBCFG_0);
    SI32_PBCFG_A_enable_xbar0l_peripherals(SI32_PBCFG_0, SI32_PBCFG_A_XBAR0L_I2C0EN);

    // I2C PIN SETUP
    // initialize PB0.0 (SDA) and PB0.2 (SCL) as digital input
    SI32_PBSTD_A_write_pbskipen(SI32_PBSTD_0, 0xFFFA);

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
    // Slave Mode Inhibit
  //  SI32_I2C_A_disable_slave_mode(SI32_I2C_0);
    // ENABLE MODULE
    SI32_I2C_A_enable_module(SI32_I2C_0);

}

/** \fn int32_t I2C_handler(uint8_t *buf, uint32_t count)
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
 * \param rw 1: read, 0: write
 * \return 0: success; 1: error
 */
int32_t I2C_handler(uint8_t *buf, uint32_t count, uint8_t rw)
{
    uint8_t * ptr = buf;
    uint8_t I2C_data_ready = 0;

    if (count == 0)
        return false;
    SI32_I2C_A_set_start(SI32_I2C_0);
    do {
        if (SI32_I2C_A_is_start_interrupt_pending(SI32_I2C_0)) { // I2C start
            uint32_t tmp = I2C_ADDRESS | rw;
            if (SI32_I2C_A_is_master_mode_enabled(SI32_I2C_0)) {// In Master mode
                SI32_I2C_A_write_data(SI32_I2C_0,tmp);
                SI32_I2C_A_set_byte_count(SI32_I2C_0, 1); // set bytes count(BC)
                SI32_I2C_A_arm_tx(SI32_I2C_0); // Arm transmission(TXARM=1)
            }
            // Start bit comes with ACKI
            SI32_I2C_0->CONTROL_CLR = SI32_I2C_A_CONTROL_STA_MASK | SI32_I2C_A_CONTROL_STAI_MASK
                    | SI32_I2C_A_CONTROL_ACKI_MASK;
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
                SI32_I2C_A_send_ack(SI32_I2C_0); // send an ACK
                count--;
            }
            if(count)
                SI32_I2C_A_arm_rx(SI32_I2C_0); // Arm reception(RXARM=1)
            else
                SI32_I2C_A_set_stop (SI32_I2C_0); // Set STO to terminte transfer
            SI32_I2C_A_clear_rx_interrupt(SI32_I2C_0);
            SI32_I2C_A_clear_ack_interrupt(SI32_I2C_0);
        }

        if (SI32_I2C_A_is_tx_interrupt_pending(SI32_I2C_0)) { // I2C data transmit
            if (SI32_I2C_A_is_ack_received(SI32_I2C_0)) {
                if (count) {
                    if (rw) {  // read
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
                SI32_I2C_A_set_stop (SI32_I2C_0);   // Set STO to terminte transfer
                // To reschedule transfer, set START then clear interrupt flags.
                SI32_I2C_A_set_start(SI32_I2C_0);
            }
            SI32_I2C_A_clear_tx_interrupt(SI32_I2C_0);  // clear TXI
            SI32_I2C_A_clear_ack_interrupt(SI32_I2C_0); // clear ACKI
        }

        if (SI32_I2C_A_is_timer3_interrupt_pending(SI32_I2C_0)) {
            SI32_I2C_A_clear_timer3_interrupt(SI32_I2C_0);
           // SI32_I2C_A_reset_module(SI32_I2C_0);
           // target_comm_init();
           // break;
        }
        if (SI32_I2C_A_is_arblost_interrupt_pending(SI32_I2C_0)) {
            SI32_I2C_A_clear_arblost_interrupt(SI32_I2C_0);
            break;
        }
    } while ((I2C_data_ready == 0));
    return (I2C_data_ready ? 0 : -1);
}

