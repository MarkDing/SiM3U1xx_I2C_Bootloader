//------------------------------------------------------------------------------
// Copyright (c) 2013 by Silicon Laboratories. 
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the Silicon Laboratories End User 
// License Agreement which accompanies this distribution, and is available at
// http://developer.silabs.com/legal/version/v10/License_Agreement_v10.htm
// Original content and implementation provided by Silicon Laboratories.
//------------------------------------------------------------------------------

/*
 *  config.h
 *
 *    Revision: 1.0 (10 MAY 2013)
 *      Author: fbar / cmenke
 */


//-----------------------------------------------
// Enable Bootloader Triggers
//
// Symbols:
// All trigger sources listed below.  Disable unneeded
// trigger sources to save code space.
//
//	Available Values:
//	<1> - Enabled
// <0> - Disabled
//
#define ANYRESET_AND_GPIO_TRIGGER_EN         1
#define PINRESET_AND_GPIO_TRIGGER_EN         0
#define PORRESET_AND_GPIO_TRIGGER_EN         0
#define PINRESET_TRIGGER_EN                  0
#define SYSRESETREQ_TRIGGER_EN               0
#define SWRESET_TRIGGER_EN                   0
#define SWRESET_AND_RAMADDR0_TRIGGER_EN      0


//-----------------------------------------------
// Trigger pin macros
//
// After the package option is determined at run
// time, one of the following macros are used
// to read the state of the trigger pin.
//
// For the SiM3U1xx, the trigger pins are defined
// as follows:
//
// 80/90 pin package - PB1.7
// 64 pin package - PB1.3
// 40 pin package - PB0.13
//
#define read_trigger_pin_package_80_90_pin()      	\
	(SI32_PBSTD_A_read_pins(SI32_PBSTD_2) & BIT8)
#define read_trigger_pin_package_64_pin() 			\
	(SI32_PBSTD_A_read_pins(SI32_PBSTD_1) & BIT3)
#define read_trigger_pin_package_40_pin() 			\
	(SI32_PBSTD_A_read_pins(SI32_PBSTD_0) & BIT14)


//-----------------------------------------------
// RAM Address 0 Trigger Mask and Value
//
// A '1' in any bit location in the mask enables
// the fist word of RAM to be checked against the
// corresponding bit in the trigger value word
// specified below.
//
// The default mask value of 0xFFFFFFFF checks
// all the bits first word of RAM against the
// trigger value.
//
#define RAMADDR0_TRIGGER_MASK        0xFFFFFFFF
#define RAMADDR0_TRIGGER_VALUE       0x805AA501

//-----------------------------------------------
// Bounds Checking
//
// Symbol:
// BOUNDS_CHECKING
//
//	Available Values:
// BASIC, ADVANCED
//
//	Basic bounds checking only checks to ensure that the 
// stack pointer and reset vector of the application 
// image are not 0xFFFFFFFF.  Advanced bounds checking
// verifies that the addresses are valid based on the
// amount of flash and ram in the device. 
//
#define BOUNDS_CHECKING   BASIC






