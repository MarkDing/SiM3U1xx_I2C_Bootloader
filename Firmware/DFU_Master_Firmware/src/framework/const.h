//------------------------------------------------------------------------------
// Copyright (c) 2013 by Silicon Laboratories. 
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the Silicon Laboratories End User 
// License Agreement which accompanies this distribution, and is available at
// http://developer.silabs.com/legal/version/v10/License_Agreement_v10.htm
// Original content and implementation provided by Silicon Laboratories.
//------------------------------------------------------------------------------

/*
 *  const.h
 *
 *    Revision: 1.0 (10 MAY 2013)
 *      Author: fbar / cmenke
 */


// MCU Families
#define SiMXXX             0
#define SiM3U1    			1
#define SiM3C1					2
#define SiM3L1  				3

// Comm Protocol
#define PROTOCOL           0
#define UART       			1
#define USB  					2
#define I2C     				3
#define SPI     				4

// Bit Masks
#define BIT0             0x0001
#define BIT1             0x0002
#define BIT2             0x0004
#define BIT3             0x0008
#define BIT4             0x0010
#define BIT5             0x0020
#define BIT6             0x0040
#define BIT7             0x0080
#define BIT8             0x0100
#define BIT9             0x0200
#define BIT10            0x0400
#define BIT11            0x0800
#define BIT12            0x1000
#define BIT13            0x2000
#define BIT14            0x4000
#define BIT15            0x8000

// Trigger Types
#define EXTERNAL_TRIGGER   0x01
#define INTERNAL_TRIGGER   0x02
#define AUTO_TRIGGER       0x04

// Bounds Checking Types
#define BASIC             0x01
#define ADVANCED          0x02
