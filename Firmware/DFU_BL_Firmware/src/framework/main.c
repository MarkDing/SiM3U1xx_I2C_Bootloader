//------------------------------------------------------------------------------
// Copyright (c) 2013 by Silicon Laboratories.
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the Silicon Laboratories End User
// License Agreement which accompanies this distribution, and is available at
// http://developer.silabs.com/legal/version/v10/License_Agreement_v10.htm
// Original content and implementation provided by Silicon Laboratories.
//------------------------------------------------------------------------------

/*
 * main.c
 *
 *  Created on: February 4, 2013
 *      Author: fbar / cmenke
 */

#include "global.h"

//------------------------------------------------------------------------------
// Function Prototypes
//------------------------------------------------------------------------------
int main(void);
#ifdef PRECISION32
void user_app_jump(void);
#else
__asm void user_app_jump(void);
#endif
//------------------------------------------------------------------------------
// Global Variables
//------------------------------------------------------------------------------
uint32_t trigger;
uint32_t flash_size;
uint32_t ram_size;

/**
 *
 */
void mySystemInit(void)
{
    return;
}

//------------------------------------------------------------------------------
// Main Routine
//------------------------------------------------------------------------------
int main(void)
{
    // Initialize device and execute boot handler
    DEVICE_Init();

    // Update Firmware or Jump to User Application
    if (trigger) {
        //Update Firmware
        DFU_Firmware_Update(trigger & AUTO_TRIGGER);
    } else {
        // Restore all registers to reset values
        DEVICE_Restore();

        // Relocate the interrupt vector base address
        DEVICE_RedirectInterrupts(USER_APP_START_ADDR);

        // Jump to the user application
        user_app_jump();

    }

    while (1)
        ;

}

//------------------------------------------------------------------------------
// user_app_jump()
//------------------------------------------------------------------------------
#ifdef PRECISION32
void user_app_jump(void)
{
    __asm volatile ("ldr r0, =0x00002000");
    __asm volatile ("ldr sp, [r0]");
    __asm volatile ("ldr pc, [r0, #4] ");
}
#else
__asm void user_app_jump(void)
{
    LDR R0, =USER_APP_START_ADDR
    LDR SP, [R0]; Initialze stack pointer
    LDR PC, [R0,#4] ; Initialize program counter
}
#endif
