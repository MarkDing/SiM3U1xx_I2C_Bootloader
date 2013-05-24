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

#include "USB0_ControlRequests.h"
#include "USB0_ISR.h"
#include <SI32_USB_A_Type.h>
#include <SI32_USBEP_A_Type.h>
#include <si32_device.h>

//-----------------------------------------------------------------------------
// Static Global Variables
//-----------------------------------------------------------------------------

// Store data received from the host in an OUT control transfer (ie DFU_DNLOAD)
static uint8_t* RxBuffer = 0;
static uint16_t RxBufferSize = 0;
static uint16_t RxBufferReceived = 0;
static bool RxComplete = true;

// Data to send to the host in an IN control transfer (ie DFU_UPLOAD)
static uint16_t TxBufferSent = 0;
static bool TxComplete = true;

//-----------------------------------------------------------------------------
// Static Function Prototypes
//-----------------------------------------------------------------------------

static void Handle_Setup (void);
static void Finish_Setup (void);

//-----------------------------------------------------------------------------
// Global Functions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Handle_Class_Request
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   : None
//
// Handle class-specific control requests
// - Decode Incoming Setup requests
// - Wait until Class_Out_Start() is called, providing a RX buffer to receive
//   data
//
//-----------------------------------------------------------------------------
void Handle_Class_Request (void)
{
  // Make sure device is configured
  if (USB0_State != DEV_CONFIGURED)
  {
    Force_Stall();
    return;
  }

  Handle_Setup();
}

// Handle reception of a control setup packet on EP0
// - If no rx_buff is available (passed through Class_Out_Start)
//   then go to a wait state
// - Otherwise copy the setup packet and receive data phase (if applicable)
//   to the rx_buff
void Handle_Setup ()
{
  // Buffer is not ready yet
  if (RxComplete)
  {
    EP_Status = EP_WAIT_RX;
  }
  else
  {
    Finish_Setup();
  }
}

// Finish handling the setup packet and move on to the IN/OUT data phase
// of the control request
void Finish_Setup(void)
{
  // Copy selected bytes from the Setup packet
  for (int i = 0; i < DFU_SETUP_PACKET_COPY_SIZE; i++)
  {
    RxBuffer[i] = ((uint8_t*)&Setup)[i + DFU_SETUP_PACKET_COPY_OFFSET];
  }

  // Set Serviced Out packet ready
  SI32_USB_A_clear_out_packet_ready_ep0(SI32_USB_0);

  // Handle out class requests
  if ((Setup.bmRequestType & DIR_BITMASK) == DIR_OUT)
  {
    // No data phase, end the control transfer
    if (Setup.wLength == 0)
    {
      // Set Serviced Out packet ready and
      // data end to indicate transaction
      // is over
      SI32_USB_A_set_data_end_ep0(SI32_USB_0);

      // Return to IDLE state
      EP_Status = EP_IDLE;

      // Control transfer complete
      USB0_RX_Complete(DFU_SETUP_PACKET_COPY_SIZE);
    }
    else
    {
      // Set the DataPtr to store data in the ClassOutBuffer (after the Setup packet data)
      ReadDataPtr = RxBuffer + DFU_SETUP_PACKET_COPY_SIZE;
      ReadDataSize = RxBufferSize - DFU_SETUP_PACKET_COPY_SIZE;
      ReadDataReceived = 0;

      // Setup the RX phase of the control transfer
      EP_Status = EP_RX;
    }
  }
  // Handle in class requests
  else
  {
    // Wait until TX data is ready from the App
    EP_Status = EP_WAIT_TX;

    // Control transfer setup complete
    USB0_RX_Complete(DFU_SETUP_PACKET_COPY_SIZE);
  }
}

// Pass in the RX buffer used to store the setup packet and
// OUT data phase
void USB0_RX_Start (uint8_t* buffer, uint16_t size)
{
  __disable_irq();

  RxBuffer = buffer;
  RxBufferSize = size;
  RxComplete = false;

  // Already received setup packet before this function was called
  if (EP_Status == EP_WAIT_RX)
  {
    Finish_Setup();
  }

  __enable_irq();
}

// Called by the USB ISR when the OUT control request has finished, meaning
// the Setup packet and RxBuffer are populated
void USB0_RX_Complete (uint16_t bytesReceived)
{
  RxBufferReceived = bytesReceived;
  RxComplete = true;
}

// Returns true if a control OUT transfer has completed
bool USB0_Is_RX_Complete (uint16_t* bytesReceived)
{
  bool complete;

  __disable_irq();

  complete = RxComplete;
  if (complete)
  {
    *bytesReceived = RxBufferReceived;
  }
  __enable_irq();

  return complete;
}

// Pass in the TX buffer used to send the IN data phase
void USB0_TX_Start (uint8_t* buffer, uint16_t size)
{
  __disable_irq();

  WriteDataPtr = buffer;
  WriteDataSize = size;
  WriteDataSent = 0;      // Reset Data Sent counter
  TxComplete = false;

  // Set Serviced Out packet ready
  SI32_USB_A_clear_out_packet_ready_ep0(SI32_USB_0);

  // Setup the TX phase of the control transfer
  EP_Status = EP_TX;

  __enable_irq();
}

// Called by the USB ISR when the IN control request has finished, meaning
// the TxBuffer has been sent
void USB0_TX_Complete (uint16_t bytesSent)
{
  TxBufferSent = bytesSent;
  TxComplete = true;
}

// Returns true if a control IN transfer has completed
bool USB0_Is_TX_Complete (uint16_t* bytesSent)
{
  bool complete;

  __disable_irq();
  complete = TxComplete;
  if (complete)
     *bytesSent = TxBufferSent;
  __enable_irq();

  return complete;
}

#endif
