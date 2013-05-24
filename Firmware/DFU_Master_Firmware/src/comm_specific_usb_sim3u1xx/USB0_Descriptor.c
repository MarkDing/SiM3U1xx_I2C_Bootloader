//------------------------------------------------------------------------------
// Copyright (c) 2013 by Silicon Laboratories.
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the Silicon Laboratories End User
// License Agreement which accompanies this distribution, and is available at
// http://developer.silabs.com/legal/version/v10/License_Agreement_v10.htm
// Original content and implementation provided by Silicon Laboratories.
//------------------------------------------------------------------------------

#include "global.h"

#if (MCU_FAMILY == SiM3U1 && COMM_PROTOCOL == USB)

#include "USB0_Descriptor.h"
#include "USB0_ISR.h"
#include "USB0_ControlRequests.h"
#include <si32_device.h>

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------

// Note: Multi-byte fields are expressed in little-endian (LSB first)

const Device_Descriptor DEVICE_DESC =
{
  0x12,                // bLength
  DSC_DEVICE,          // bDescriptorType
  LE_ARRAY(0x0100),    // bcdUSB
  0x00,                // bDeviceClass
  0x00,                // bDeviceSubClass
  0x00,                // bDeviceProtocol
  EP0_PACKET_SIZE,     // bMaxPacketSize0
  LE_ARRAY(0x10C4),    // idVendor
  LE_ARRAY(0x888E),    // idProduct
  LE_ARRAY(0x0100),    // bcdDevice
  0x01,                // iManufacturer
  0x02,                // iProduct
  0x00,                // iSerialNumber
  0x01                 // bNumConfigurations
};

const Application_Configuration_Descriptor APP_CONFIG_DESC =
{
  .configuration =
  {
    0x09,                // bLength
    DSC_CONFIG,          // bDescriptorType
    LE_ARRAY(0x0012),    // wTotalLength
    0x01,                // bNumInterfaces
    0x01,                // bConfigurationValue
    0x00,                // iConfiguration
    0x80,                // bmAttributes (Bus-powered)
    0x64                 // bMaxPower (200 mA)
  },

  .interface =
  {
    0x09,                // bLength
    DSC_INTERFACE,       // bDescriptorType
    0x00,                // bInterfaceNumber
    0x00,                // bAlternateSetting
    0x00,                // bNumEndpoints (Excluding EP0)
    0xFE,                // bInterfaceClass (DFU Class)
    0x01,                // bInterfaceSubClass (DFU Sub Class)
    0x02,                // bInterfaceProcotol (DFU mode protocol)
    0x00                 // iInterface
  }
};

#define STRING0_LEN 4

const uint8_t STRING0_DESC[STRING0_LEN] =
{
   STRING0_LEN, DSC_STRING, 0x09, 0x04
};

// Note: sizeof("") returns an additional +1 for the null-terminator,
// which in this case is used in place of the first two bytes
// in the string descriptor
#define STRING1_LEN sizeof ("Silicon Laboratories Inc.") * 2

const uint8_t STRING1_DESC[STRING1_LEN] =
{
   STRING1_LEN, DSC_STRING,
   'S', 0,
   'i', 0,
   'l', 0,
   'i', 0,
   'c', 0,
   'o', 0,
   'n', 0,
   ' ', 0,
   'L', 0,
   'a', 0,
   'b', 0,
   'o', 0,
   'r', 0,
   'a', 0,
   't', 0,
   'o', 0,
   'r', 0,
   'i', 0,
   'e', 0,
   's', 0,
   ' ', 0,
   'I', 0,
   'n', 0,
   'c', 0,
   '.', 0
};

// Note: sizeof("") returns an additional +1 for the null-terminator,
// which in this case is used in place of the first two bytes
// in the string descriptor
#define STRING2_LEN sizeof("DFU Bootloader") * 2

const uint8_t STRING2_DESC[STRING2_LEN] =
{
   STRING2_LEN, DSC_STRING,
   'D', 0,
   'F', 0,
   'U', 0,
   ' ', 0,
   'B', 0,
   'o', 0,
   'o', 0,
   't', 0,
   'l', 0,
   'o', 0,
   'a', 0,
   'd', 0,
   'e', 0,
   'r', 0
};

const uint8_t* const STRING_DESC_TABLE[] =
{
   STRING0_DESC,
   STRING1_DESC,
   STRING2_DESC
};

#endif

