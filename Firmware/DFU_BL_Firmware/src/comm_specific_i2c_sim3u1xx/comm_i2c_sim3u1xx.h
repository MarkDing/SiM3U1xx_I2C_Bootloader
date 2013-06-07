//------------------------------------------------------------------------------
// Copyright (c) 2013 by Silicon Laboratories. 
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the Silicon Laboratories End User 
// License Agreement which accompanies this distribution, and is available at
// http://developer.silabs.com/legal/version/v10/License_Agreement_v10.htm
// Original content and implementation provided by Silicon Laboratories.
//------------------------------------------------------------------------------
#ifndef __COMM_I2C_SIM3U1XX_H__
#define __COMM_I2C_SIM3U1XX_H__
/**
 *  \file comm_i2c_sim3u1xx.h
 *  \brief I2C communication code header file
 *
 *  \version: 1.0
 *  \date: 31 May 2013
 *  \author: Mark Ding
 */

//------------------------------------------------------------------------------
// Device Specific Macros and Constants
//------------------------------------------------------------------------------
/**
 *  \def COMM_GET_AHB_CLOCK() 20000000
 *  \brief Get ABH clock, in default 20000000
 */
#define COMM_GET_AHB_CLOCK() 20000000

/**
 * \fn void COMM_Init(void)
 * \brief Initialise I2C module
 *
 * Setup Crossbar and I/O for I2C. Set I2C clock rate to 400kHz,
 * Configure timeout with SysTick Timer
 */
void COMM_Init(void);

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
 * \return 0: success; -1: error
 */
int32_t I2C_handler(uint8_t *buf, uint32_t count);

#endif
