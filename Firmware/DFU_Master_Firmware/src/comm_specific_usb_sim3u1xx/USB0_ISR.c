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
#include "USB0_StandardRequests.h"
#include "USB0_ControlRequests.h"
#include <SI32_USB_A_Type.h>
#include <SI32_USBEP_A_Type.h>
#include <SI32_PBSTD_A_Type.h>
#include <si32_device.h>

//==============================================================================
// Global Variables
//==============================================================================

uint8_t USB0_State;             // Holds the current USB State

Setup_Packet Setup;             // Buffer for current device
                                // request information

uint16_t ReadDataSize;          // Size of data to read
uint16_t ReadDataReceived;      // Amount of data received so far
uint8_t* ReadDataPtr;           // Pointer to data to return

uint16_t WriteDataSize;         // Size of data to return
uint16_t WriteDataSent;         // Amount of data sent so far
const uint8_t* WriteDataPtr;    // Pointer to data to send

// Holds the status for control EP0
uint8_t EP_Status = EP_IDLE;

//==============================================================================
// 2nd Level Interrupt Handler Prototypes
//==============================================================================

void USB0_start_of_frame_handler(void);
void USB0_resume_handler(void);
void USB0_reset_handler(void);
void USB0_suspend_handler(void);
void USB0_ep0_rx(void);
void USB0_ep0_tx(uint32_t ControlReg);
void USB0_ep0_handler(void);

//==============================================================================
// Utility Function Prototypes
//==============================================================================

uint16_t USB0_EP0_read_fifo(uint8_t * dst,  uint16_t count);
uint16_t USB0_EP0_write_fifo(const uint8_t * src, uint16_t count);

//==============================================================================
// 1st LEVEL  INTERRUPT HANDLERS
//==============================================================================

void USB0_IRQHandler(void)
{
  uint32_t usbCommonInterruptMask = SI32_USB_A_read_cmint(SI32_USB_0);
  uint32_t usbEpInterruptMask = SI32_USB_A_read_ioint(SI32_USB_0);

  SI32_USB_A_write_cmint(SI32_USB_0, usbCommonInterruptMask);
  SI32_USB_A_write_ioint(SI32_USB_0, usbEpInterruptMask);

  if (usbEpInterruptMask & SI32_USB_A_IOINT_EP0I_MASK)
  {
    USB0_ep0_handler();
  }

  // Handle Start of Frame Interrupt
  if (usbCommonInterruptMask & SI32_USB_A_CMINT_SOFI_MASK)
  {
    USB0_start_of_frame_handler();
  }

  // Handle Resume Interrupt
  if (usbCommonInterruptMask & SI32_USB_A_CMINT_RESI_MASK)
  {
    USB0_resume_handler();
  }

  // Handle Reset Interrupt
  if (usbCommonInterruptMask & SI32_USB_A_CMINT_RSTI_MASK)
  {
    USB0_reset_handler();
  }

  // Handle Suspend interrupt
  if (usbCommonInterruptMask & SI32_USB_A_CMINT_SUSI_MASK)
  {
    USB0_suspend_handler();
  }
}

//==============================================================================
// 2nd Level Interrupt Handlers
//==============================================================================

void USB0_ep0_tx (uint32_t ControlReg)
{
  // Don't overwrite last IN packet
  if (!(ControlReg & SI32_USB_A_EP0CONTROL_IPRDYI_MASK))
  {
    // Read control register
    ControlReg = SI32_USB_A_read_ep0control(SI32_USB_0);

    // Check to see if Setup End or Out Packet received, if so do not put
    // any new data on FIFO
    if ((!(ControlReg & SI32_USB_A_EP0CONTROL_SUENDI_MASK)) || (!(ControlReg & SI32_USB_A_EP0CONTROL_OPRDYI_MASK)))
    {
      // Limit total number of bytes sent during TX data phase transfer
      uint16_t transferSize = MIN(WriteDataSize, Setup.wLength);

      // Calculate number of bytes to send for the current packet
      uint16_t packetSize = MIN(transferSize - WriteDataSent, EP0_PACKET_SIZE);

      // Write the current packet to the EP0 FIFO
      uint16_t written = USB0_EP0_write_fifo(WriteDataPtr + WriteDataSent, packetSize);

      // Keep track of the total number of bytes sent during this transfer
      WriteDataSent += written;

      // Add In Packet ready flag to bitmask
      ControlReg = SI32_USB_A_EP0CONTROL_IPRDYI_MASK;

      // Data transfer is complete
      if (WriteDataSent == transferSize)
      {
        ControlReg |= SI32_USB_A_EP0CONTROL_DEND_MASK;
        EP_Status = EP_IDLE;
      }

      // Write EP0 control register
      SI32_USB_A_write_ep0control(SI32_USB_0, ControlReg);

      // If the data phase of the control transfer is complete (DEND is set)
      // then notify the DFU application that all of the data has been received
      if (EP_Status == EP_IDLE &&
         (Setup.bmRequestType == DFU_REQUEST_TYPE_IN || Setup.bmRequestType == SLAB_REQUEST_TYPE_IN))
      {
        USB0_TX_Complete(WriteDataSent);
      }
    }
  }
}

void USB0_ep0_rx(void)
{
  // Read control register
  uint32_t ControlReg = SI32_USB_A_read_ep0control(SI32_USB_0);

  // Verify packet was received
  if (ControlReg & SI32_USB_A_EP0CONTROL_OPRDYI_MASK)
  {
    // Limit total number of bytes received during the RX data phase transfer
    uint16_t transferSize = MIN(ReadDataSize, Setup.wLength);

    // Calculate number of bytes to receive for the current packet
    uint16_t packetSize = MIN(transferSize - ReadDataReceived, EP0_PACKET_SIZE);

    // Read the current packet from the EP0 FIFO
    uint16_t read = USB0_EP0_read_fifo(ReadDataPtr + ReadDataReceived, packetSize);

    // Keep track of the total number of bytes recevied during this transfer
    ReadDataReceived += read;

    // Add Out Packet ready flag to bitmask
    ControlReg = SI32_USB_A_EP0CONTROL_OPRDYIS_MASK;

    // Read all bytes
    if (ReadDataReceived == transferSize)
    {
      ControlReg |= SI32_USB_A_EP0CONTROL_DEND_MASK;
      EP_Status = EP_IDLE;
    }

    // Write EP0 control register
    SI32_USB_A_write_ep0control(SI32_USB_0, ControlReg);

    // If the data phase of the control transfer is complete (DEND is set)
    // then notify the DFU application that all of the data has been received
    if (EP_Status == EP_IDLE &&
        (Setup.bmRequestType == DFU_REQUEST_TYPE_OUT || Setup.bmRequestType == SLAB_REQUEST_TYPE_OUT))
    {
      USB0_RX_Complete(DFU_SETUP_PACKET_COPY_SIZE + ReadDataReceived);
    }
  }
}

void USB0_ep0_handler(void)
{
  uint32_t ControlReg = SI32_USB_A_read_ep0control(SI32_USB_0);

  // Handle status phase of Set Address command
  if (EP_Status == EP_ADDRESS)
  {
    SI32_USB_A_write_faddr(SI32_USB_0, Setup.wValue);
    EP_Status = EP_IDLE;
  }

  // If last packet was a sent stall, reset STSTL bit
  // and return EP0 to idle state
  if(ControlReg & SI32_USB_A_EP0CONTROL_STSTLI_MASK)
  {
    SI32_USB_A_clear_stall_sent_ep0(SI32_USB_0);
    EP_Status = EP_IDLE;
    return;
  }

  // If last Setup transaction was ended prematurely
  if(ControlReg & SI32_USB_A_EP0CONTROL_SUENDI_MASK)
  {
    // Set data end
    SI32_USB_A_set_data_end_ep0(SI32_USB_0);

    // Serviced setup end bit
    SI32_USB_A_clear_setup_end_early_ep0(SI32_USB_0);

    // Return EP0 to idle state
    EP_Status = EP_IDLE;

    // Notify the DFU application that the transaction has been aborted
    USB0_RX_Complete(0);
    USB0_TX_Complete(0);
  }

  // If EP0 is idle
  if (EP_Status == EP_IDLE)
  {
    // Make sure that EP0 has an out packet ready from host
    // although if EP0 is idle, this should always be the case
    if (ControlReg & SI32_USB_A_EP0CONTROL_OPRDYI_MASK)
    {
      // Get Setup packet from FIFO
      // Multi-byte values are currently stored in little-endian
      uint16_t bytesRead = USB0_EP0_read_fifo((uint8_t*)&Setup, sizeof(Setup_Packet));

      // Invalid setup packet size
      if (bytesRead != 8)
      {
        Force_Stall();
      }
      // Handle standard requests
      else if ((Setup.bmRequestType & REQ_TYPE_BITMASK) == REQ_TYPE_STD_REQ)
      {
        Handle_Standard_Request();
      }
      // Handle vendor requests
      else if ((Setup.bmRequestType & REQ_TYPE_BITMASK) == REQ_TYPE_VENDOR)
      {
        Handle_Class_Request();
      }
      // Handle class requests
      else if ((Setup.bmRequestType & REQ_TYPE_BITMASK) == REQ_TYPE_CLASS_REQ &&
               (Setup.bmRequestType & RECP_BITMASK) == RECP_INTERFACE)
      {
        Handle_Class_Request();
      }
      // Unsupported request type
      else
      {
        Force_Stall();
      }
    }
  }

  // Check if endpoint should transmit
  if (EP_Status == EP_TX)
  {
    USB0_ep0_tx(ControlReg);
  }
  // Check if endpoint should receive
  else if (EP_Status == EP_RX)
  {
    USB0_ep0_rx();
  }
}

void USB0_start_of_frame_handler(void)
{
  uint32_t ControlReg = SI32_USB_A_read_ep0control(SI32_USB_0);

  // Check if endpoint should transmit
  if (EP_Status == EP_TX)
  {
    USB0_ep0_tx(ControlReg);
  }
  // Check if endpoint should receive
  else if (EP_Status == EP_RX)
  {
    USB0_ep0_rx();
  }
}

void USB0_resume_handler(void)
{
  // Intentionally left blank
}

void USB0_reset_handler(void)
{
  // Set device state to default
  USB0_State = DEV_DEFAULT;

  // Set default Endpoint Status
  EP_Status = EP_IDLE;

  // Enable Endpoint 0/1 interrupts
  SI32_USB_A_write_iointe(SI32_USB_0, 0);
  SI32_USB_A_enable_ep0_interrupt(SI32_USB_0);

  // Enable suspend detection
  SI32_USB_A_enable_suspend_detection(SI32_USB_0);

  // Abort and DFU application transactions
  USB0_RX_Complete(0);
  USB0_TX_Complete(0);
}

void USB0_suspend_handler(void)
{
  /* TODO */
}

void Force_Stall(void)
{
  SI32_USB_A_send_stall_ep0(SI32_USB_0);
  EP_Status = EP_STALL;
}

//==============================================================================
// Configuration Functions
//==============================================================================

void USB0_Init()
{
  // Perform asynchronous reset of the USB module
  SI32_USB_A_reset_module(SI32_USB_0);

  // Enable Transceiver, fullspeed
  SI32_USB_A_write_tcontrol(SI32_USB_0, 0);
  SI32_USB_A_select_transceiver_full_speed(SI32_USB_0);
  SI32_USB_A_enable_transceiver(SI32_USB_0);

  // Enable Reset, Resume, Suspend interrupts
  SI32_USB_A_write_cmintepe(SI32_USB_0, 0);
  SI32_USB_A_enable_suspend_interrupt(SI32_USB_0);
  SI32_USB_A_enable_resume_interrupt(SI32_USB_0);
  SI32_USB_A_enable_reset_interrupt(SI32_USB_0);
  SI32_USB_A_enable_start_of_frame_interrupt(SI32_USB_0);

  // Enable Endpoint 0/1
  SI32_USB_A_enable_ep0(SI32_USB_0);

  // Enable clock recovery, single-step mode disabled
  SI32_USB_A_enable_clock_recovery(SI32_USB_0);
  SI32_USB_A_select_clock_recovery_mode_full_speed(SI32_USB_0);
  SI32_USB_A_select_clock_recovery_normal_cal(SI32_USB_0);

  // Uninhibit the module once all initialization is complete
  SI32_USB_A_enable_module(SI32_USB_0);

  // Enable pull-up resistor when VBUS is present
  SI32_USB_A_enable_internal_pull_up(SI32_USB_0);

  // Enable USB interrupts
  NVIC_EnableIRQ(USB0_IRQn);
}

//==============================================================================
// Utility Functions
//==============================================================================

uint16_t USB0_EP0_read_fifo(uint8_t * dst,  uint16_t count)
{
  uint16_t result;
  result = count = MIN(count, SI32_USB_A_read_ep0_count(SI32_USB_0));

  while (count)
  {
    *dst = SI32_USB_A_read_ep0_fifo_u8(SI32_USB_0);
    dst++;
    count--;
  }

  return result;
}

uint16_t USB0_EP0_write_fifo(const uint8_t * src, uint16_t count)
{
  uint16_t result;
  result = count = MIN(count, EP0_PACKET_SIZE);

  while (count)
  {
    SI32_USB_A_write_ep0_fifo_u8(SI32_USB_0, *src);
    src++;
    count--;
  }

  return result;
}

#endif
