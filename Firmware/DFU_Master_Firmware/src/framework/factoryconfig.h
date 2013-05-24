//------------------------------------------------------------------------------
// Copyright (c) 2013 by Silicon Laboratories. 
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the Silicon Laboratories End User 
// License Agreement which accompanies this distribution, and is available at
// http://developer.silabs.com/legal/version/v10/License_Agreement_v10.htm
// Original content and implementation provided by Silicon Laboratories.
//------------------------------------------------------------------------------

/*
 *  factoryconfig.h
 *
 *    Revision: 1.0 (10 MAY 2013)
 *      Author: fbar / cmenke
 */


#define BOOTLOADER_REVISION_MAJOR 1
#define BOOTLOADER_REVISION_MINOR 0

//-----------------------------------------------
// Build Options
//
#define INTERRUPT_SUPPORT             0
#define RAM_EXECUTION_SUPPORT         0

//-----------------------------------------------
// DFU Memory Buffer Sizes
//
#define DFU_CMD_BUFFER_SIZE               7+1024
#define DFU_GETSTATUSREQUEST_BUFFER_SIZE  7


//-----------------------------------------------
// Stack Pointer and Reset Vector
//
#define USER_APP_STACKPTR_INITVAL  (*((volatile unsigned long *) USER_APP_START_ADDR))
#define USER_APP_RESETVECT_INITVAL  (*((volatile unsigned long *) (USER_APP_START_ADDR+4)))


