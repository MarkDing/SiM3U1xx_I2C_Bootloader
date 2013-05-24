//------------------------------------------------------------------------------
// Copyright (c) 2013 by Silicon Laboratories. 
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the Silicon Laboratories End User 
// License Agreement which accompanies this distribution, and is available at
// http://developer.silabs.com/legal/version/v10/License_Agreement_v10.htm
// Original content and implementation provided by Silicon Laboratories.
//------------------------------------------------------------------------------

/*
 *  userconfig.h
 *
 *    Revision: 1.0 (10 MAY 2013)
 *      Author: fbar / cmenke
 */

//-----------------------------------------------
// Select MCU Family
//
// Symbol:
// MCU_FAMILY
//
//	Available Values:
//	SiM3U1, SiM3C1, SiM3L1
//
// This symbol is defined in the uVision project file.

//-----------------------------------------------
// Select Comm Protocol
//
// Symbol:
// COMM_PROTOCOL
//
//	Available Values:
//	UART, USB, I2C, SPI
//
// This symbol is defined in the uVision project file.



//-----------------------------------------------
// User Application Starting Address
//
// Symbol:
// USER_APP_START
//
//	Available Values:
//	Any valid 32-bit address containing the entry point
// of the user code
//
#define USER_APP_START_ADDR 0x00002000



