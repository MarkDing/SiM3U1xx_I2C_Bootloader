//------------------------------------------------------------------------------
// Copyright (c) 2013 by Silicon Laboratories. 
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the Silicon Laboratories End User 
// License Agreement which accompanies this distribution, and is available at
// http://developer.silabs.com/legal/version/v10/License_Agreement_v10.htm
// Original content and implementation provided by Silicon Laboratories.
//------------------------------------------------------------------------------

#ifndef __MYUSB0_CONTROL_REQUESTS_H__
#define __MYUSB0_CONTROL_REQUESTS_H__

#include <stdint.h>
#include <stdbool.h>

//==============================================================================
// Definitions
//==============================================================================

#define DFU_REQUEST_TYPE_IN                 (DIR_IN | REQ_TYPE_CLASS_REQ | RECP_INTERFACE)
#define DFU_REQUEST_TYPE_OUT                (DIR_OUT | REQ_TYPE_CLASS_REQ | RECP_INTERFACE)
#define SLAB_REQUEST_TYPE_IN                (DIR_IN | REQ_TYPE_VENDOR | RECP_INTERFACE)
#define SLAB_REQUEST_TYPE_OUT               (DIR_OUT | REQ_TYPE_VENDOR | RECP_INTERFACE)

#define DFU_DETACH_REQUEST                  0x00
#define DFU_DNLOAD_REQUEST                  0x01
#define DFU_UPLOAD_REQUEST                  0x02
#define DFU_GETSTATUS_REQUEST               0x03
#define DFU_CLRSTATUS_REQUEST               0x04
#define DFU_GETSTATE_REQUEST                0x05
#define DFU_ABORT_REQUEST                   0x06
#define SLAB_GETINFO_REQUEST                0x07

#define DFU_DETACH_LENGTH                   0
//#define DFU_DNLOAD_LENGTH                 n
//#define DFU_UPLOAD_LENGTH                 n
#define DFU_GETSTATUS_LENGTH                6
#define DFU_CLRSTATUS_LENGTH                0
#define DFU_GETSTATE_LENGTH                 1
#define DFU_ABORT_LENGTH                    0
#define SLAB_GETINFO_LENGTH                 25

// Number of bytes in the setup packet to copy
#define DFU_SETUP_PACKET_COPY_OFFSET        1
#define DFU_SETUP_PACKET_COPY_SIZE          7

//==============================================================================
// External Global Variables
//==============================================================================

//==============================================================================
// Function Prototypes
//==============================================================================

void Handle_Class_Request (void);

void USB0_RX_Start (uint8_t* buffer, uint16_t size);
void USB0_RX_Complete (uint16_t bytesReceived);
bool USB0_Is_RX_Complete (uint16_t* bytesReceived);

void USB0_TX_Start (uint8_t* buffer, uint16_t size);
void USB0_TX_Complete (uint16_t bytesSent);
bool USB0_Is_TX_Complete (uint16_t* bytesSent);

#endif // __MYUSB0_CONTROL_REQUESTS_H__
