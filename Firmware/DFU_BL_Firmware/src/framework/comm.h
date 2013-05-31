//------------------------------------------------------------------------------
// Copyright (c) 2013 by Silicon Laboratories. 
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the Silicon Laboratories End User 
// License Agreement which accompanies this distribution, and is available at
// http://developer.silabs.com/legal/version/v10/License_Agreement_v10.htm
// Original content and implementation provided by Silicon Laboratories.
//------------------------------------------------------------------------------

/*
 *  comm.h
 *
 *    Revision: 1.0 (10 MAY 2013)
 *      Author: fbar / cmenke
 */



//------------------------------------------------------------------------------
// Device Specific Macros and Constants
//------------------------------------------------------------------------------
#if (MCU_FAMILY == SiMXXX)
   #if(COMM_PROTOCOL == PROTOCOL)
      #include "..\comm_specific_protocol_simxxxxx\comm_protocol_simxxxxx.h"
   #else
      #error "Protocol not recognized in COMM module"
   #endif
#elif (MCU_FAMILY == SiM3U1)
   #if (COMM_PROTOCOL == UART)
      #include "..\comm_specific_uart_sim3u1xx\comm_uart_sim3u1xx.h"
   #elif (COMM_PROTOCOL == I2C)
      #include "..\comm_specific_i2c_sim3u1xx\comm_i2c_sim3u1xx.h"
   #elif (COMM_PROTOCOL == USB)
      #include "..\comm_specific_usb_sim3u1xx\comm_usb_sim3u1xx.h"
   #else
      #error "Protocol not recognized in COMM module"
   #endif
#else
   #error "MCU family not recognized in COMM module"
#endif 


//------------------------------------------------------------------------------
// Exported Function Prototypes
//------------------------------------------------------------------------------
extern void COMM_Init(void);
extern void COMM_Reset_Timeout(uint32_t timeout_ms);
extern uint32_t COMM_Timeout(void);

extern uint32_t COMM_Receive(uint8_t* rx_buff, uint32_t length); 
extern uint32_t COMM_Receive_Async(uint8_t* rx_buff, uint32_t length); 
extern uint32_t COMM_Is_Data_Available(void);

extern uint32_t COMM_Transmit(uint8_t* tx_buff, uint32_t length);
extern uint32_t COMM_Transmit_Async(uint8_t* tx_buff, uint32_t length);
extern uint32_t COMM_Is_Transmit_Complete(void);

