//------------------------------------------------------------------------------
// Copyright (c) 2013 by Silicon Laboratories. 
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the Silicon Laboratories End User 
// License Agreement which accompanies this distribution, and is available at
// http://developer.silabs.com/legal/version/v10/License_Agreement_v10.htm
// Original content and implementation provided by Silicon Laboratories.
//------------------------------------------------------------------------------

#ifndef __MYUSB0_STANDARD_REQUESTS_H__
#define __MYUSB0_STANDARD_REQUESTS_H__

#include <stdint.h>

//==============================================================================
// Function Prototypes
//==============================================================================

void Handle_Standard_Request(void);

void Get_Status(void);
void Clear_Feature(void);
void Set_Feature(void);
void Set_Address(void);
void Get_Descriptor(void);
void Get_Configuration(void);
void Set_Configuration(void);
void Get_Interface(void);
void Set_Interface(void);

#endif // __MYUSB0_STANDARD_REQUESTS_H__
