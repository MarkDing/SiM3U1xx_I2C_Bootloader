//------------------------------------------------------------------------------
// Copyright (c) 2013 by Silicon Laboratories. 
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the Silicon Laboratories End User 
// License Agreement which accompanies this distribution, and is available at
// http://developer.silabs.com/legal/version/v10/License_Agreement_v10.htm
// Original content and implementation provided by Silicon Laboratories.
//------------------------------------------------------------------------------

/*
 *  comm_uart_sim3u1xx.h
 *
 *    Revision: 1.0 (10 MAY 2013)
 *      Author: fbar / cmenke
 */
/**
 *  \file comm_uart_sim3u1xx.h
 *  \brief UART communication code header file
 *
 *  \version: 1.0
 *  \date: 31 May 2013
 *  \author: Mark Ding
 */

/**
 * @defgroup group_comm_UART UART communication module
 * @brief UART communication protocol code
 *
 *  @section Sec_ModDescription Module Description
 *  UART communication module . This module contains transmit function
 *  @ref COMM_Transmit(),  receive function @ref COMM_Receive(),
 *  communication initialization function @ref COMM_Init()
 */

#ifndef __COMM_UART_SIM3U1XX_H__
#define __COMM_UART_SIM3U1XX_H__

//------------------------------------------------------------------------------
// Device Specific Macros and Constants
//------------------------------------------------------------------------------

#define COMM_GET_AHB_CLOCK() 20000000


/**
 * @fn void COMM_Init(void)
 * @brief Initialise UART module
 *
 * Setup Crossbar and I/O for UART. Auto detect UART baudrate,
 * Configure timeout with SysTick Timer
 */
void COMM_Init(void);



/**
 * @fn uint32_t COMM_Receive(uint8_t* rx_buff, uint32_t length)
 *
 * Receive data from PC through UART interface
 *
 * @param rx_buff buffer for received data
 * @param length expect received data  length
 * @return length
 */
uint32_t COMM_Receive(uint8_t* rx_buff, uint32_t length);


/**
 * @fn uint32_t COMM_Transmit(uint8_t* tx_buff, uint32_t length)
 *
 * Transmit data to PC through UART interface
 *
 * @param tx_buff buffer for transmit data
 * @param length data transfer length
 * @return length
 */
uint32_t COMM_Transmit(uint8_t* tx_buff, uint32_t length);


#endif
