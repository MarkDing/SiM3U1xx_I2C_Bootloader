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

#include "USB0_StandardRequests.h"
#include "USB0_Descriptor.h"
#include "USB0_ISR.h"
#include <SI32_USB_A_Type.h>
#include <SI32_USBEP_A_Type.h>
#include <si32_device.h>

//-----------------------------------------------------------------------------
// Static Global Variables
//-----------------------------------------------------------------------------

// These are response packets used for communication with host
static const uint8_t ONES_PACKET[2] = {0x01, 0x00};
static const uint8_t ZERO_PACKET[2] = {0x00, 0x00};

//-----------------------------------------------------------------------------
// Global Functions
//-----------------------------------------------------------------------------

void Handle_Standard_Request(void)
{
  switch (Setup.bRequest)    // Call correct subroutine to handle
  {                          // each kind of standard request
    case GET_STATUS:
      Get_Status ();
      break;
    case CLEAR_FEATURE:
      Clear_Feature ();
      break;
    case SET_FEATURE:
      Set_Feature ();
      break;
    case SET_ADDRESS:
      Set_Address ();
      break;
    case GET_DESCRIPTOR:
      Get_Descriptor ();
      break;
    case GET_CONFIGURATION:
      Get_Configuration ();
      break;
    case SET_CONFIGURATION:
      Set_Configuration ();
      break;
    case GET_INTERFACE:
      Get_Interface ();
      break;
    case SET_INTERFACE:
      Set_Interface ();
      break;
    default:
      Force_Stall ();      // Send stall to host if invalid request
      break;
  }
}

//-----------------------------------------------------------------------------
// Get_Status
//-----------------------------------------------------------------------------
//
// Return Value - None
// Parameters - None
//
// Standard request that should not change for custom HID designs.
//
// This routine returns a two byte status packet to the host.
//
// ----------------------------------------------------------------------------
void Get_Status (void)
{
  if (Setup.wValue != 0 ||
     Setup.wLength != 2)
  {
    Force_Stall ();
  }

  // Determine if recipient was device, interface, or EP
  switch(Setup.bmRequestType)
  {
    // If recipient was device
    case OUT_DEVICE:
      if (Setup.wIndex != 0)
      {
        // Send stall if request is invalid
        Force_Stall ();
      }
      else
      {
  			// Otherwise send 0x00, indicating bus power and no
  			// remote wake-up supported
        WriteDataPtr = ZERO_PACKET;
        WriteDataSize = 2;
      }
      break;

    // See if recipient was interface
    case OUT_INTERFACE:
      // Only valid if device is configured and non-zero index
      if ((USB0_State != DEV_CONFIGURED) || Setup.wIndex != 0)
      {
        // Otherwise send stall to host
        Force_Stall ();
      }
      else
      {
        // Status packet always returns 0x00
        WriteDataPtr = ZERO_PACKET;
        WriteDataSize = 2;
      }
      break;

    // See if recipient was an endpoint
    case OUT_ENDPOINT:
      // Send stall if unexpected data encountered
      Force_Stall ();
      break;

    default:
      Force_Stall ();
      break;
  }

  if (EP_Status != EP_STALL)
  {
    // Set serviced Setup Packet, Endpoint 0 in transmit mode, and
    // reset DataSent counter
    SI32_USB_A_clear_out_packet_ready_ep0(SI32_USB_0);
    EP_Status = EP_TX;
    WriteDataSent = 0;
  }
}

//-----------------------------------------------------------------------------
// Clear_Feature
//-----------------------------------------------------------------------------
//
// Return Value - None
// Parameters - None
//
// Standard request that should not change in custom HID designs.
//
// This routine can clear Halt Endpoint features on endpoint 1
//
//-----------------------------------------------------------------------------
void Clear_Feature (void)
{
  // Send procedural stall
  Force_Stall ();

  if (EP_Status != EP_STALL)
  {
    // Set Serviced Out packet ready and
    // data end to indicate transaction
    // is over
    SI32_USB_A_clear_out_packet_ready_ep0(SI32_USB_0);
    SI32_USB_A_set_data_end_ep0(SI32_USB_0);
  }
}

//-----------------------------------------------------------------------------
// Set_Feature
//-----------------------------------------------------------------------------
//
// Return Value - None
// Parameters - None
//
// Standard request that should not change in custom HID designs.
//
// This routine will set the EP Halt feature for endpoint 1
//
//-----------------------------------------------------------------------------
void Set_Feature (void)
{
  // Send procedural stall
  Force_Stall ();

  if (EP_Status != EP_STALL)
  {
    // Indicate Setup packet has been serviced
    SI32_USB_A_clear_out_packet_ready_ep0(SI32_USB_0);
    SI32_USB_A_set_data_end_ep0(SI32_USB_0);
  }
}

//-----------------------------------------------------------------------------
// Set_Address
//-----------------------------------------------------------------------------
//
// Return Value - None
// Parameters - None
//
// Standard request that should not change in custom HID designs.
//
// Set new function address
//
//-----------------------------------------------------------------------------
void Set_Address (void)
{
  // Request must be directed to device
  // with index and length set to zero.
  if ((Setup.bmRequestType != IN_DEVICE) ||
     (Setup.wIndex != 0) ||
     (Setup.wLength != 0) ||
     (Setup.wValue == 0x0000) ||
     (Setup.wValue > 0x007F))
  {
    // Send stall if Setup data invalid
    Force_Stall ();
  }

  // Set endpoint zero to update address next status phase
  EP_Status = EP_ADDRESS;

  if (LOBYTE(Setup.wValue) != 0)
  {
    // Indicate that device state is now address
    USB0_State = DEV_ADDRESS;
  }
  else
  {
    // If new address was 0x00, return device to default state
    USB0_State = DEV_DEFAULT;
  }

  if (EP_Status != EP_STALL)
  {
    // Indicate Setup packet has been serviced
    SI32_USB_A_clear_out_packet_ready_ep0(SI32_USB_0);
    SI32_USB_A_set_data_end_ep0(SI32_USB_0);
  }
}

//-----------------------------------------------------------------------------
// Get_Descriptor
//-----------------------------------------------------------------------------
//
// Return Value - None
// Parameters - None
//
// Standard request that should not change in custom HID designs.
//
// This routine sets the data pointer and size to correct descriptor and
// sets the endpoint status to transmit
//
//-----------------------------------------------------------------------------
void Get_Descriptor (void)
{
  // Determine which type of descriptor
  // was requested, and set data ptr and
  // size accordingly
  switch (HIBYTE(Setup.wValue))
  {
    case DSC_DEVICE:
      WriteDataPtr = (const uint8_t*)&DEVICE_DESC;
      WriteDataSize = DEVICE_DESC.bLength;
      break;

    case DSC_CONFIG:
      WriteDataPtr = (const uint8_t*)&APP_CONFIG_DESC.configuration;
      WriteDataSize = (APP_CONFIG_DESC.configuration.wTotalLengthLsb) +
                      (APP_CONFIG_DESC.configuration.wTotalLengthMsb << 8);
      break;

	  case DSC_STRING:
		// NOTE: if strings are added to this project, the hard-coded
      // value of 2 will need to be increased
      if (LOBYTE(Setup.wValue) > 2)
      {
        // Invalid string descriptor index
        Force_Stall();
      }
      else
      {
        WriteDataPtr = STRING_DESC_TABLE[LOBYTE(Setup.wValue)];
        WriteDataSize = WriteDataPtr[0];
      }
      break;

    case DSC_INTERFACE:
      WriteDataPtr = (const uint8_t*)&APP_CONFIG_DESC.interface;
      WriteDataSize = APP_CONFIG_DESC.interface.bLength;
      break;

    case DSC_ENDPOINT:
      Force_Stall();
      break;

    default:
      // Send Stall if unsupported request
      Force_Stall ();
      break;
  }

  // Make sure endpoint not in stall mode
  if (EP_Status != EP_STALL)
  {
    SI32_USB_A_clear_out_packet_ready_ep0(SI32_USB_0);
    EP_Status = EP_TX;          // Put endpoint in transmit mode
    WriteDataSent = 0;               // Reset Data Sent counter
  }
}

//-----------------------------------------------------------------------------
// Get_Configuration
//-----------------------------------------------------------------------------
//
// Return Value - None
// Parameters - None
//
// Standard request that should not change in custom HID designs.
//
// This routine returns current configuration value
//
//-----------------------------------------------------------------------------
void Get_Configuration (void)
{
  // This request must be directed to the device
  // With value word set to zero
  // And index set to zero
  // And Setup length set to one
  if ((Setup.bmRequestType != OUT_DEVICE) ||
      (Setup.wValue != 0) ||
      (Setup.wIndex != 0) ||
      (Setup.wLength != 1))
  {
    // Otherwise send a stall to host
    Force_Stall ();
  }

  else
  {
    // If the device is configured, then
    // return value 0x01 since this software
    // only supports one configuration
    if (USB0_State == DEV_CONFIGURED)
    {
      WriteDataPtr = ONES_PACKET;
      WriteDataSize = 1;
    }
    // If the device is in address state, it
    // is not configured, so return 0x00
    if (USB0_State == DEV_ADDRESS)
    {
      WriteDataPtr = ZERO_PACKET;
      WriteDataSize = 1;
    }
  }
  if (EP_Status != EP_STALL)
  {
    // Set Serviced Out Packet bit
    SI32_USB_A_clear_out_packet_ready_ep0(SI32_USB_0);
    EP_Status = EP_TX;         // Put endpoint into transmit mode
    WriteDataSent = 0;              // Reset Data Sent counter to zero
  }
}

//-----------------------------------------------------------------------------
// Set_Configuration
//-----------------------------------------------------------------------------
//
// Return Value - None
// Parameters - None
//
// Standard request that should not change in custom HID designs.
//
// This routine allows host to change current device configuration value
//
//-----------------------------------------------------------------------------
void Set_Configuration (void)
{
  // Device must be addressed before configured
  // and request recipient must be the device
  // the index and length words must be zero
  // This software only supports config = 0,1
  if ((USB0_State == DEV_DEFAULT) ||
      (Setup.bmRequestType != IN_DEVICE) ||
      (Setup.wIndex != 0) ||
      (Setup.wLength != 0) ||
      (Setup.wValue > 1))
  {
    // Send stall if Setup data is invalid
    Force_Stall ();
  }
  else
  {
    // Any positive configuration request
    // results in configuration being set
    // to 1
    if (LOBYTE(Setup.wValue) > 0)
    {
      USB0_State = DEV_CONFIGURED;
    }
    else
    {
      USB0_State = DEV_ADDRESS;   // Unconfigures device by setting state
    }
  }

  if (EP_Status != EP_STALL)
  {
    // Indicate Setup packet has been serviced
    SI32_USB_A_clear_out_packet_ready_ep0(SI32_USB_0);
    SI32_USB_A_set_data_end_ep0(SI32_USB_0);
  }
}

//-----------------------------------------------------------------------------
// Get_Interface
//-----------------------------------------------------------------------------
//
// Return Value - None
// Parameters - None
//
// Standard request that should not change in custom HID designs.
//
// This routine returns 0x00, since only one interface is supported by
// this firmware
//
//-----------------------------------------------------------------------------
void Get_Interface (void)
{
  // If device is not configured
  // or recipient is not an interface
  // or non-zero value or index fields
  // or data length not equal to one
  if ((USB0_State != DEV_CONFIGURED) ||
      (Setup.bmRequestType != OUT_INTERFACE) ||
      (Setup.wValue != 0) ||
      (Setup.wIndex != 0) ||
      (Setup.wLength != 1))
  {
    // Then return stall due to invalid request
    Force_Stall ();
  }
  else
  {
    // Otherwise, return 0x00 to host
    WriteDataPtr = ZERO_PACKET;
    WriteDataSize = 1;
  }

  if (EP_Status != EP_STALL)
  {
    // Set Serviced Setup packet, put endpoint in transmit mode and reset
    // Data sent counter
    SI32_USB_A_clear_out_packet_ready_ep0(SI32_USB_0);
    EP_Status = EP_TX;
    WriteDataSent = 0;
  }
}

//-----------------------------------------------------------------------------
// Set_Interface
//-----------------------------------------------------------------------------
//
// Return Value - None
// Parameters - None
//
// Standard request that should not change in custom HID designs.
//
//-----------------------------------------------------------------------------
void Set_Interface (void)
{
  // Make sure request is directed at interface
  // and all other packet values are set to zero
  if ((Setup.bmRequestType != IN_INTERFACE) ||
      (Setup.wLength != 0) ||
      (Setup.wValue != 0) ||
      (Setup.wIndex != 0))
  {
    // Othewise send a stall to host
    Force_Stall ();
  }

  if (EP_Status != EP_STALL)
  {
    // Indicate Setup packet has been
    // serviced
    SI32_USB_A_clear_out_packet_ready_ep0(SI32_USB_0);
    SI32_USB_A_set_data_end_ep0(SI32_USB_0);
  }
}
#endif
