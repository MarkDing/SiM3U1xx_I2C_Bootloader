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
 *  @file comm_i2c_sim3u1xx.h
 *  @brief I2C communication code header file
 *
 *  @version: 1.0
 *  @date: 31 May 2013
 *  @author: Mark Ding
 */

/**
 *  @def COMM_GET_AHB_CLOCK() 20000000
 *  @brief Get ABH clock, in default 20000000
 */
#define COMM_GET_AHB_CLOCK() 20000000

/**
 * @def I2C_ADDRESS
 * @brief I2C slave address 0xF0
 */
#define I2C_ADDRESS 0xF0

/**
 * @def PACKET_LENGTH
 * @brief packet length
 *
 * Packet format: 0x3A, sequence number, LSB len, MSB len, LSB CRC, MSB CRC
 */
#define PACKET_LENGTH 6

/**
 * @fn void COMM_Init(void)
 * @brief Initialise I2C module
 *
 * Setup Crossbar and I/O for I2C. Set I2C clock rate to 100kHz,
 * Configure timeout with SysTick Timer
 */
void COMM_Init(void);

/**
 * @fn int32_t I2C_handler(uint8_t *buf, uint32_t count)
 * @brief  I2C communication handler
 *
 * It handlers both transmit and receive, read/write data in buf in
 * parameter.
 *
 * @note I2C STAI comes with ACKI, so we need clear ACKI
 * @note I2C TXI and RXI happens, the TXARM and RXARM has been cleared. Set them to 1 for next transition
 * @note Last byte of TXI, we don't need set TXARM = 1.
 * @param buf buffer for transmit and receive data
 * @param count number of bytes to transfer or receive
 * @return 0: success; -1: error
 */
int32_t I2C_handler(uint8_t *buf, uint32_t count);


/**
 * @fn uint32_t COMM_Receive(uint8_t* rx_buff, uint32_t length)
 *
 * Receive data from master MCU through I2C bus
 *
 * @param rx_buff buffer for received data
 * @param length expect received data  length
 * @return length
 */
uint32_t COMM_Receive(uint8_t* rx_buff, uint32_t length);


/**
 * @fn uint32_t COMM_Transmit(uint8_t* tx_buff, uint32_t length)
 *
 * Transmit data to master MCU through I2C bus
 *
 * @param tx_buff buffer for transmit data
 * @param length data transfer length
 * @return length
 */
uint32_t COMM_Transmit(uint8_t* tx_buff, uint32_t length);

#endif
