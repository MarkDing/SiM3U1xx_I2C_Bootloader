//------------------------------------------------------------------------------
// Copyright (c) 2013 by Silicon Laboratories.
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the Silicon Laboratories End User
// License Agreement which accompanies this distribution, and is available at
// http://developer.silabs.com/legal/version/v10/License_Agreement_v10.htm
// Original content and implementation provided by Silicon Laboratories.
//------------------------------------------------------------------------------

/*
 *  dfu.c
 *
 *    Revision: 1.0 (10 MAY 2013)
 *      Author: fbar / cmenke
 */

#include "global.h"

//------------------------------------------------------------------------------
// Exported Function Prototypes
//------------------------------------------------------------------------------
void DFU_Firmware_Update(uint32_t app_image_state);

//------------------------------------------------------------------------------
// Static Module Variables
//------------------------------------------------------------------------------
uint32_t Dfu_State;
uint32_t Dfu_Status;

uint32_t Dfu_Block;
uint32_t Command_Received;
uint32_t GetStatusRequest_Received;
uint32_t upload_length;

//------------------------------------------------------------------------------
// Static Memory Buffers
//------------------------------------------------------------------------------
 uint8_t Dfu_Cmd_Buffer[DFU_CMD_BUFFER_SIZE];
 uint8_t Dfu_GetStatusRequest_Buffer[DFU_GETSTATUSREQUEST_BUFFER_SIZE];

//------------------------------------------------------------------------------
// DFU_Firmware_Update
//------------------------------------------------------------------------------
//
// Parameters:
//
//    app_image_state
//       <0>         the application image is valid
//       <non-zero>  the application image is invalid
//
void DFU_Firmware_Update(uint32_t app_image_state)
{
   DFU_Cmd_Packet           *cmd;
   DFU_State_Reply_Packet   *state_reply;
   DFU_Status_Reply_Packet  *status_reply;
   DFU_GetInfo_Reply_Packet *getinfo_reply;
   DFU_GetStatusRequest_Packet *getstatus_request;
   uint32_t num_bytes;


   // Initialize File Manager and Communication Interface
   FILEMGR_Init();
   COMM_Init();

   // Initialize Packet Pointers
   cmd = (DFU_Cmd_Packet*) Dfu_Cmd_Buffer;
   state_reply = (DFU_State_Reply_Packet*)Dfu_Cmd_Buffer;
   status_reply = (DFU_Status_Reply_Packet*)Dfu_Cmd_Buffer;
   getinfo_reply = (DFU_GetInfo_Reply_Packet*)Dfu_Cmd_Buffer;

   getstatus_request = (DFU_GetStatusRequest_Packet*) Dfu_GetStatusRequest_Buffer;


   // Initialize DFU State Machine
   if(app_image_state)
   {
      Dfu_State = dfuERROR;
      Dfu_Status = errVERIFY;
   } else
   {
      Dfu_State = dfuIDLE;
      Dfu_Status = OK;
   }

   // Initialize local module variables
   Command_Received = 0;
   GetStatusRequest_Received = 0;
   Dfu_Block = 0; // This initialization is not really required.

   // Start the DFU State Machine
   while(1)
   {
      //----------------------------------------------------
      // Get Commands
      //----------------------------------------------------
      if(Dfu_State == dfuDNLOAD_SYNC)
      {
         if(COMM_Receive(Dfu_GetStatusRequest_Buffer, DFU_GETSTATUSREQUEST_BUFFER_SIZE))
         {
            if(getstatus_request->bRequest != DFU_GETSTATUS)
            {
               for(int i = 0; i < DFU_GETSTATUSREQUEST_BUFFER_SIZE; i++)
               {
                  Dfu_Cmd_Buffer[i] = Dfu_GetStatusRequest_Buffer[i];
               }
               Command_Received = 1;
            } else
            {
               GetStatusRequest_Received = 1;
            }
         } else
         {
            Dfu_State = dfuERROR;
            Dfu_Status = errCOMM;
         }
      } else
      {
         if(COMM_Receive(Dfu_Cmd_Buffer, DFU_CMD_BUFFER_SIZE))
         {
            Command_Received = 1;
         } else
         {
            Dfu_State = dfuERROR;
            Dfu_Status = errCOMM;
         }
      }

      //----------------------------------------------------
      // Process Commands available in all states
      //----------------------------------------------------
      if(Command_Received)
      {

         // Handle the DFU_GETSTATE command for all states
         if(cmd->bRequest == DFU_GETSTATE)
         {
            Command_Received = 0;   // Clear Command Received Flag

            state_reply->bState = Dfu_State;

            num_bytes = COMM_Transmit((uint8_t*)state_reply, SIZEOF_DFU_State_Reply_Packet);
            if(num_bytes == 0)
            {
               Dfu_State = dfuERROR;
               Dfu_Status = errCOMM;
            }

            continue;
         }

         // Handle the DFU_GETINFO command for all states
         else if (cmd->bRequest == DFU_GETINFO)
         {
            Command_Received = 0;   // Clear Command Received Flag

            getinfo_reply->length = (uint16_t) SIZEOF_DFU_GetInfo_Reply_Packet;
            getinfo_reply->bBootloaderRevision_Major = BOOTLOADER_REVISION_MAJOR;
            getinfo_reply->bBootloaderRevision_Minor = BOOTLOADER_REVISION_MINOR;
            getinfo_reply->bmAttributes = 0x07;
            getinfo_reply->wBlockSize = FILEMGR_Get_Block_Size();
            getinfo_reply->maxAppSize = flash_size - USER_APP_START_ADDR - SIZEOF_DFU_Flash_Type;

            // Copy 16-byte device ID and 16-byte UUID from DEVICE module
            DEVICE_Fill_DeviceID_UUID(&(getinfo_reply->bDeviceID[0]));

            num_bytes = COMM_Transmit((uint8_t*)getinfo_reply, SIZEOF_DFU_GetInfo_Reply_Packet);
            if(num_bytes == 0)
            {
               Dfu_State = dfuERROR;
               Dfu_Status = errCOMM;
            }
            continue;
         }

         ///////////////////////////////////////////////////
         // DFU_RESET COMMAND
         //
         else if(cmd->bRequest == DFU_RESET)
         {
            Command_Received = 0;   // Clear Command Received Flag

            // Perform a software reset on the MCU
            DEVICE_Reset();
         }
      }

      //----------------------------------------------------
      // Process state-based commands
      //----------------------------------------------------
      //if(Command_Received || Wait_For_GetStatusRequest) // remove this if statement...
      {
         switch(Dfu_State)
         {

            //----------------------------------------------------
            // dfuIDLE State (2)
            //
            //
            //----------------------------------------------------
            case dfuIDLE:
            {
               ///////////////////////////////////////////////////
               // DFU_GETSTATUS COMMAND
               //
               if(Command_Received && cmd->bRequest == DFU_GETSTATUS)
               {
                  Command_Received = 0;   // Clear Command Received Flag

                  status_reply->bStatus = Dfu_Status;
                  status_reply->wPollTimeout_Low = 0;
                  status_reply->bPollTimeout_High = 0;
                  status_reply->bState = Dfu_State;
                  status_reply->iString = 0;
                  num_bytes = COMM_Transmit((uint8_t*)status_reply, SIZEOF_DFU_Status_Reply_Packet);
                  if(num_bytes == 0)
                  {
                     Dfu_State = dfuERROR;
                     Dfu_Status = errCOMM;
                  }
               } else

               ///////////////////////////////////////////////////
               // DFU_DNLOAD COMMAND
               //
               if(Command_Received && cmd->bRequest == DFU_DNLOAD)
               {
                  Command_Received = 0;   // Clear Command Received Flag

                  Dfu_Status = FILEMGR_Validate_Dnload ((uint8_t*) &cmd->Data, cmd->wLength);

                  if(Dfu_Status == OK)
                  {
                     // Set DFU state to DNLOAD_SYNC with a DFU Block Number of zero
                     // to indicate the start of a new download operation
                     Dfu_State = dfuDNLOAD_SYNC;
                     Dfu_Block = 0x00;

                  } else
                  {
                     Dfu_State = dfuERROR;
                  }

               } else

               ///////////////////////////////////////////////////
               // DFU_UPLOAD COMMAND
               //
               if(Command_Received && cmd->bRequest == DFU_UPLOAD)
               {
                  Command_Received = 0;   // Clear Command Received Flag

                  // Initialize Upload Length to zero
                  // If the request passes validation, then the upload
                  // length will increase, otherwise we will respond
                  // with a zero-length packet to indicate that there
                  // is nothing to upload.
                  upload_length = 0;

                  // Verify that the upload is starting with block zero
                  // On an UPLOAD request, the block number is received
                  // in the wValue field
                  if(cmd->wValue == 0x0000)
                  {
                     // Reset the expected DFU Block Number
                     Dfu_Block = 0;

                     // Fill the outgoing command buffer with Block0
                     upload_length = FILEMGR_Start_Upload ((uint8_t*) &cmd->Data, FILEMGR_Get_Block_Size());
                  }

                  // Transmit the UPLOAD data
                  num_bytes = COMM_Transmit((uint8_t*) &cmd->Data, upload_length);
                  if(num_bytes == 0)
                  {
                     Dfu_State = dfuERROR;
                     Dfu_Status = errCOMM;
                  }

                  // Verify that upload length is a full block.  Anything less than a block should
                  // terminate the upload operation and the dfu state should return to idle.
                  // Note: Since we are only checking block 0, the following test should always pass.
                  if(upload_length == FILEMGR_Get_Block_Size())
                  {
                     Dfu_State = dfuUPLOAD_IDLE;
                     Dfu_Status = OK;

                     Dfu_Block++;                  // Increment the expected block number

                  } else
                  {
                     Dfu_State = dfuIDLE;
                     Dfu_Status = OK;
                  }
               }

            } break;

            //----------------------------------------------------
            // dfuERROR State (10)
            //
            //
            //----------------------------------------------------
            case dfuERROR:
            {
               ///////////////////////////////////////////////////
               // DFU_GETSTATUS COMMAND
               //
               if(Command_Received && cmd->bRequest == DFU_GETSTATUS)
               {
                  Command_Received = 0;   // Clear Command Received Flag

                  status_reply->bStatus = Dfu_Status;
                  status_reply->wPollTimeout_Low = 0;
                  status_reply->bPollTimeout_High = 0;
                  status_reply->bState = Dfu_State;
                  status_reply->iString = 0;

                  num_bytes = COMM_Transmit((uint8_t*)status_reply, SIZEOF_DFU_Status_Reply_Packet);
                  if(num_bytes == 0)
                  {
                     Dfu_State = dfuERROR;
                     Dfu_Status = errCOMM;
                  }
               } else

               ///////////////////////////////////////////////////
               // DFU_CLRSTATUS COMMAND
               //
               if(Command_Received && cmd->bRequest == DFU_CLRSTATUS)
               {
                  Command_Received = 0;   // Clear Command Received Flag

                  Dfu_State = dfuIDLE;
                  Dfu_Status = OK;
               }
            } break;

            //----------------------------------------------------
            // dfuDNLOAD_SYNC State (3)
            //
            //
            //----------------------------------------------------
            case dfuDNLOAD_SYNC:
            {
               ///////////////////////////////////////////////////
               // DFU_GETSTATUS COMMAND
               //
               if(GetStatusRequest_Received && getstatus_request->bRequest == DFU_GETSTATUS)
               {
                  GetStatusRequest_Received = 0;

                  if(Dfu_Block == 0)
                  {
                     // Initialize the Flash memory
                     Dfu_Status = FILEMGR_Initialize_Dnload();
                  }  else
                  {
                     // Process the DFU Block
                     Dfu_Status = FILEMGR_Continue_Dnload ((uint8_t*) &cmd->Data, cmd->wLength);
                  }

                  // Increment the DFU block number to the next expected block
                  Dfu_Block++;

                  // Switch to the dfuDNLOAD_IDLE state
                  if(Dfu_Status == OK)
                  {
                     Dfu_State = dfuDNLOAD_IDLE;
                  } else
                  {
                      Dfu_State = dfuERROR;
                  }

                  // Send the status reply
                  status_reply->bStatus = Dfu_Status;
                  status_reply->wPollTimeout_Low = 0;
                  status_reply->bPollTimeout_High = 0;
                  status_reply->bState = Dfu_State;
                  status_reply->iString = 0;
                  num_bytes = COMM_Transmit((uint8_t*)status_reply, SIZEOF_DFU_Status_Reply_Packet);
                  if(num_bytes == 0)
                  {
                     Dfu_State = dfuERROR;
                     Dfu_Status = errCOMM;
                  }
               } else

               ///////////////////////////////////////////////////
               // DFU_ABORT COMMAND
               //
               if(Command_Received && cmd->bRequest == DFU_ABORT)
               {
                  Command_Received = 0;   // Clear Command Received Flag

                  Dfu_State = dfuIDLE;
                  Dfu_Status = OK;
               }
            } break;

            //----------------------------------------------------
            // dfuDNLOAD_IDLE State (5)
            //
            //
            //----------------------------------------------------
            case dfuDNLOAD_IDLE:
            {
               ///////////////////////////////////////////////////
               // DFU_DNLOAD COMMAND
               //
               if(Command_Received && cmd->bRequest == DFU_DNLOAD)
               {
                  Command_Received = 0;   // Clear Command Received Flag

                  if(cmd->wLength > 0)
                  {
                     Dfu_State = dfuDNLOAD_SYNC;


                  }else
                  {

                     Dfu_State = dfuMANIFEST_SYNC;

                  }
               } else

               ///////////////////////////////////////////////////
               // DFU_ABORT COMMAND
               //
               if(Command_Received && cmd->bRequest == DFU_ABORT)
               {
                  Command_Received = 0;   // Clear Command Received Flag

                  Dfu_State = dfuIDLE;
                  Dfu_Status = OK;
               } else

               ///////////////////////////////////////////////////
               // DFU_GETSTATUS COMMAND
               //
               if(Command_Received && cmd->bRequest == DFU_GETSTATUS)
               {
                  Command_Received = 0;   // Clear Command Received Flag

                  status_reply->bStatus = Dfu_Status;
                  status_reply->wPollTimeout_Low = 0;
                  status_reply->bPollTimeout_High = 0;
                  status_reply->bState = Dfu_State;
                  status_reply->iString = 0;

                  num_bytes = COMM_Transmit((uint8_t*)status_reply, SIZEOF_DFU_Status_Reply_Packet);
                  if(num_bytes == 0)
                  {
                     Dfu_State = dfuERROR;
                     Dfu_Status = errCOMM;
                  }
               }

            } break;

            //----------------------------------------------------
            // dfuMANIFEST_SYNC State (6)
            //
            //
            //----------------------------------------------------
            case dfuMANIFEST_SYNC:
            {
               ///////////////////////////////////////////////////
               // DFU_GETSTATUS COMMAND
               //
               if(Command_Received && cmd->bRequest == DFU_GETSTATUS)
               {

                  Command_Received = 0;   // Clear Command Received Flag


                  Dfu_Status = FILEMGR_Finish_Dnload();

                  if(Dfu_Status == OK)
                  {
                     Dfu_State = dfuIDLE;

                  } else
                  {
                     Dfu_State = dfuERROR;
                  }

                  // Send the status reply
                  status_reply->bStatus = Dfu_Status;
                  status_reply->wPollTimeout_Low = 0;
                  status_reply->bPollTimeout_High = 0;
                  status_reply->bState = Dfu_State;
                  status_reply->iString = 0;
                  num_bytes = COMM_Transmit((uint8_t*)status_reply, SIZEOF_DFU_Status_Reply_Packet);
                  if(num_bytes == 0)
                  {
                     Dfu_State = dfuERROR;
                     Dfu_Status = errCOMM;
                  }
               } else

               ///////////////////////////////////////////////////
               // DFU_ABORT COMMAND
               //
               if(cmd->bRequest == DFU_ABORT)
               {
                  Command_Received = 0;   // Clear Command Received Flag

                  Dfu_State = dfuIDLE;
                  Dfu_Status = OK;
               }
            } break;

            //----------------------------------------------------
            // dfuUPLOAD_IDLE State (9)
            //
            //
            //----------------------------------------------------
            case dfuUPLOAD_IDLE:
            {
               ///////////////////////////////////////////////////
               // DFU_UPLOAD COMMAND
               //
               if(Command_Received && cmd->bRequest == DFU_UPLOAD)
               {
                  Command_Received = 0;   // Clear Command Received Flag

                  // Initialize Upload Length to zero
                  // If the request passes validation, then the upload
                  // length will increase, otherwise we will respond
                  // with a zero-length packet to indicate that there
                  // is nothing to upload.
                  upload_length = 0;

                  // Verify that the requested block number matches the
                  // expected block number. On an UPLOAD request, the block
                  // number is received in the wValue field
                  if(cmd->wValue == Dfu_Block)
                  {
                     // Fill the outgoing command buffer
                     upload_length = FILEMGR_Continue_Upload ((uint8_t*) &cmd->Data, FILEMGR_Get_Block_Size());
                  } else
                  {
                     Dfu_State = dfuERROR;
                     Dfu_Status = errADDRESS;
                  }

                  // Transmit the UPLOAD data
                  num_bytes = COMM_Transmit((uint8_t*) &cmd->Data, upload_length);
                  if(num_bytes == 0)
                  {
                     Dfu_State = dfuERROR;
                     Dfu_Status = errCOMM;
                  }

                  // Verify that upload length is a full block.  Anything less than a block should
                  // terminate the upload operation and the dfu state should return to idle unless
                  // an error has occured, it which case, it should stay in the error state
                  if(upload_length == FILEMGR_Get_Block_Size())
                  {
                     Dfu_State = dfuUPLOAD_IDLE;
                     Dfu_Status = OK;

                     Dfu_Block++;                  // Increment the expected block number
                  } else
                  if(Dfu_State != dfuERROR)
                  {
                     Dfu_State = dfuIDLE;
                     Dfu_Status = OK;
                  }

               } else

               ///////////////////////////////////////////////////
               // DFU_GETSTATUS COMMAND
               //
               if(Command_Received && cmd->bRequest == DFU_GETSTATUS)
               {

                  Command_Received = 0;   // Clear Command Received Flag

                  status_reply->bStatus = Dfu_Status;
                  status_reply->wPollTimeout_Low = 0;
                  status_reply->bPollTimeout_High = 0;
                  status_reply->bState = Dfu_State;
                  status_reply->iString = 0;
                  num_bytes = COMM_Transmit((uint8_t*)status_reply, SIZEOF_DFU_Status_Reply_Packet);
                  if(num_bytes == 0)
                  {
                     Dfu_State = dfuERROR;
                     Dfu_Status = errCOMM;
                  }
               } else

               ///////////////////////////////////////////////////
               // DFU_ABORT COMMAND
               //
               if(Command_Received && cmd->bRequest == DFU_ABORT)
               {
                  Command_Received = 0;   // Clear Command Received Flag

                  Dfu_State = dfuIDLE;
                  Dfu_Status = OK;
               }

            } break;
         }
      } // if Comm_Receive

   } // DFU State Machine
}
