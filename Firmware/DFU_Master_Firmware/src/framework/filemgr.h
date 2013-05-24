//------------------------------------------------------------------------------
// Copyright (c) 2013 by Silicon Laboratories. 
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the Silicon Laboratories End User 
// License Agreement which accompanies this distribution, and is available at
// http://developer.silabs.com/legal/version/v10/License_Agreement_v10.htm
// Original content and implementation provided by Silicon Laboratories.
//------------------------------------------------------------------------------

/*
 *  filemgr.h
 *
 *    Revision: 1.0 (10 MAY 2013)
 *      Author: fbar / cmenke
 */


//------------------------------------------------------------------------------
// Exported Macros
//------------------------------------------------------------------------------
#define FILEMGR_Get_Block_Size() FLCTL_Get_Sector_Size()


//------------------------------------------------------------------------------
// Exported Function Prototypes
//------------------------------------------------------------------------------
extern void FILEMGR_Init(void);
extern uint32_t FILEMGR_Validate_Dnload(uint8_t* buffer, uint32_t length); 
extern uint32_t FILEMGR_Initialize_Dnload(void); 
extern uint32_t FILEMGR_Continue_Dnload(uint8_t* buffer, uint32_t length);
extern uint32_t FILEMGR_Finish_Dnload(void);
extern uint32_t FILEMGR_Start_Upload(uint8_t* buffer, uint32_t length); 
extern uint32_t FILEMGR_Continue_Upload(uint8_t* buffer, uint32_t length);

