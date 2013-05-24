//------------------------------------------------------------------------------
// Copyright (c) 2013 by Silicon Laboratories. 
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the Silicon Laboratories End User 
// License Agreement which accompanies this distribution, and is available at
// http://developer.silabs.com/legal/version/v10/License_Agreement_v10.htm
// Original content and implementation provided by Silicon Laboratories.
//------------------------------------------------------------------------------

/*
 *  device_sim3u1xx.h
 *
 *    Revision: 1.0 (10 MAY 2013)
 *      Author: fbar / cmenke
 */

#if (MCU_FAMILY == SiM3U1)

#include "device_userconfig_sim3u1xx.h"


//-----------------------------------------------
// Derivative Register Definition and Bit Masks
//
// The DERIVATIVE1 register contains information
// about the flash size and the package option --
// parameters which are determined at run time.
//
// The bit masks below refer to the specific bits
// in the DERIVATIVE1 register.
//
#define DERIVATIVE1  (*((volatile unsigned long *) 0x40049080))

#define PACKAGE_OPTION_MASK 0x0003
#define PACKAGE_80_90_PIN   0x0003
#define PACKAGE_64_PIN      0x0002
#define PACKAGE_40_PIN      0x0001

#define FLASH_SIZE_MASK     0x0030
#define FLASH_32K           0x0000
#define FLASH_64K           0x0010
#define FLASH_128K          0x0020
#define FLASH_256K          0x0030

#define RAM_SIZE_MASK       0x0300
#define RAM_8K              0x0000
#define RAM_16K             0x0100
#define RAM_24K             0x0200
#define RAM_32K             0x0300

//-----------------------------------------------
// RAM Address 0
//
// The first word of RAM is used to trigger a
// firmware update when this trigger source is
// enabled.
//
#define RAMADDR0_ADDR 0x20000000
#define RAMADDR0  (*((volatile unsigned long *) RAMADDR0_ADDR))


//-----------------------------------------------
// Other
//
//

#define RESET_SOURCE_CHECK_REQUIRED          \
   (PINRESET_AND_GPIO_TRIGGER_EN       ||    \
    PORRESET_AND_GPIO_TRIGGER_EN       ||    \
    PINRESET_TRIGGER_EN                ||    \
    SYSRESETREQ_TRIGGER_EN             ||    \
    SWRESET_TRIGGER_EN                 ||    \
    SWRESET_AND_RAMADDR0_TRIGGER_EN)
         
#define TRIGGER_PIN_CHECK_REQUIRED           \
   (ANYRESET_AND_GPIO_TRIGGER_EN       ||    \
    PINRESET_AND_GPIO_TRIGGER_EN       ||    \
    PORRESET_AND_GPIO_TRIGGER_EN)
    

#endif
