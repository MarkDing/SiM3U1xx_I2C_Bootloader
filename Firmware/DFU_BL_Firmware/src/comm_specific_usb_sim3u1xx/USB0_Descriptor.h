//------------------------------------------------------------------------------
// Copyright (c) 2013 by Silicon Laboratories. 
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the Silicon Laboratories End User 
// License Agreement which accompanies this distribution, and is available at
// http://developer.silabs.com/legal/version/v10/License_Agreement_v10.htm
// Original content and implementation provided by Silicon Laboratories.
//------------------------------------------------------------------------------

#ifndef __MYUSB0DESCRIPTORS_H__
#define __MYUSB0DESCRIPTORS_H__

#include <stdint.h>

//-----------------------------------------------------------------------------
// Type Definitions
//-----------------------------------------------------------------------------

// Standard Device Descriptor Type Defintion
typedef struct Device_Descriptor
{
  uint8_t   bLength;            // Size of this Descriptor in Bytes
  uint8_t   bDescriptorType;    // Descriptor Type (=1)
  uint8_t   bcdUsbLsb;          // USB Spec Release Number in BCD
  uint8_t   bcdUsbMsb;
  uint8_t   bDeviceClass;       // Device Class Code
  uint8_t   bDeviceSubClass;    // Device Subclass Code
  uint8_t   bDeviceProtocol;    // Device Protocol Code
  uint8_t   bMaxPacketSize0;    // Maximum Packet Size for EP0
  uint8_t   idVendorLsb;        // Vendor ID
  uint8_t   idVendorMsb;
  uint8_t   idProductLsb;       // Product ID
  uint8_t   idProductMsb;
  uint8_t   bcdDeviceLsb;       // Device Release Number in BCD
  uint8_t   bcdDeviceMsb;
  uint8_t   iManufacturer;      // Index of String Desc for Manufacturer
  uint8_t   iProduct;           // Index of String Desc for Product
  uint8_t   iSerialNumber;      // Index of String Desc for SerNo
  uint8_t   bNumConfigurations; // Number of possible Configurations

} Device_Descriptor;

// Standard Configuration Descriptor Type Definition
typedef struct Configuration_Descriptor
{
  uint8_t   bLength;            // Size of this Descriptor in Bytes
  uint8_t   bDescriptorType;    // Descriptor Type (=2)
  uint8_t   wTotalLengthLsb;    // Total Length of Data for this Conf
  uint8_t   wTotalLengthMsb;
  uint8_t   bNumInterfaces;     // Number of Interfaces supported by this
                                // Conf
  uint8_t   bConfigurationValue;// Designator Value for *this*
                                // Configuration
  uint8_t   iConfiguration;     // Index of String Desc for this Conf
  uint8_t   bmAttributes;       // Configuration Characteristics (see below)
  uint8_t   bMaxPower;          // Max. Power Consumption in this
                                // Conf (*2mA)
} Configuration_Descriptor;

// Standard Interface Descriptor Type Definition
typedef struct Interface_Descriptor
{
  uint8_t   bLength;            // Size of this Descriptor in Bytes
  uint8_t   bDescriptorType;    // Descriptor Type (=4)
  uint8_t   bInterfaceNumber;   // Number of *this* Interface (0..)
  uint8_t   bAlternateSetting;  // Alternative for this Interface (if any)
  uint8_t   bNumEndpoints;      // No of EPs used by this IF (excl. EP0)
  uint8_t   bInterfaceClass;    // Interface Class Code
  uint8_t   bInterfaceSubClass; // Interface Subclass Code
  uint8_t   bInterfaceProtocol; // Interface Protocol Code
  uint8_t   iInterface;         // Index of String Desc for this Interface

} Interface_Descriptor;

// Standard Endpoint Descriptor Type Definition
typedef struct Endpoint_Descriptor
{
  uint8_t   bLength;            // Size of this Descriptor in Bytes
  uint8_t   bDescriptorType;    // Descriptor Type (=5)
  uint8_t   bEndpointAddress;   // Endpoint Address (Number + Direction)
  uint8_t   bmAttributes;       // Endpoint Attributes (Transfer Type)
  uint8_t   wMaxPacketSizeLsb;  // Max. Endpoint Packet Size
  uint8_t   wMaxPacketSizeMsb;
  uint8_t   bInterval;          // Polling Interval (Interrupt) ms

} Endpoint_Descriptor;

// Includes standard configuration, interface, and endpoint
// descriptors
typedef struct Application_Configuration_Descriptor
{
  Configuration_Descriptor  configuration;
  Interface_Descriptor      interface;

} Application_Configuration_Descriptor;

// Setup Packet Type Definition
typedef struct Setup_Packet
{
  uint8_t   bmRequestType;      // Request recipient, type, and dir.
  uint8_t   bRequest;           // Specific standard request number
  uint16_t  wValue;             // varies according to request
  uint16_t  wIndex;             // varies according to request
  uint16_t  wLength;            // Number of bytes to transfer

} Setup_Packet;

//-----------------------------------------------------------------------------
// External Global Variables
//-----------------------------------------------------------------------------

extern const Device_Descriptor DEVICE_DESC;
extern const Application_Configuration_Descriptor APP_CONFIG_DESC;
extern const uint8_t* const STRING_DESC_TABLE[];

#endif //__MYUSB0DESCRIPTORS_H__
