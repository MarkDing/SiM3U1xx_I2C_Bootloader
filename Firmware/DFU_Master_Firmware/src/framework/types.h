//------------------------------------------------------------------------------
// Copyright (c) 2013 by Silicon Laboratories. 
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the Silicon Laboratories End User 
// License Agreement which accompanies this distribution, and is available at
// http://developer.silabs.com/legal/version/v10/License_Agreement_v10.htm
// Original content and implementation provided by Silicon Laboratories.
//------------------------------------------------------------------------------

/*
 *  types.h
 *
 *    Revision: 1.0 (10 MAY 2013)
 *      Author: fbar / cmenke
 */


typedef enum DFU_State_Enum
{
   dfuIDLE              = 2,
   dfuDNLOAD_SYNC       = 3,
   dfuDNLOAD_IDLE       = 5,
   dfuMANIFEST_SYNC     = 6,
   dfuUPLOAD_IDLE       = 9,
   dfuERROR             = 10
} DFU_State_Type;

typedef enum DFU_Command_Enum
{
   DFU_DNLOAD           = 1,
   DFU_UPLOAD           = 2,
   DFU_GETSTATUS        = 3,
   DFU_CLRSTATUS        = 4,
   DFU_GETSTATE         = 5,
   DFU_ABORT            = 6,
   DFU_GETINFO          = 7,
   DFU_RESET            = 8
} DFU_Command_Type;

typedef enum DFU_Status_Enum
{
   OK                   = 0x00,
   errTARGET            = 0x01,
   errFILE              = 0x02,
   errWRITE             = 0x03,
   errERASE             = 0x04,
   errCHECK_ERASED      = 0x05,
   errPROG              = 0x06,
   errVERIFY            = 0x07,
   errADDRESS           = 0x08,
   errNOTDONE           = 0x09,
   errFIRMWARE          = 0x0A,
   errVENDOR            = 0x0B,
   errUSBR              = 0x0C,
   errPOR               = 0x0D,
   errUNKNOWN           = 0x0E,
   errSTALLEDPKT        = 0x0F,
   errCOMM              = 0x10
} DFU_Status_Type;

#pragma pack(push,1)
typedef struct DFU_Cmd_Packet_Struct
{
   uint8_t bRequest;
   uint16_t wValue;
   uint16_t wIndex;
   uint16_t wLength;  // Size of the Data Field
   uint8_t Data[DFU_CMD_BUFFER_SIZE - 7];
} DFU_Cmd_Packet;
#pragma pack(pop)

#define SIZEOF_DFU_Cmd_Packet (7 + DFU_CMD_BUFFER_SIZE)

#pragma pack(push,1)
typedef struct DFU_GetStatusRequest_Packet_Struct
{
   uint8_t bRequest;
   uint16_t wValue;
   uint16_t wIndex;
   uint16_t wLength;
} DFU_GetStatusRequest_Packet;
#pragma pack(pop)

#define SIZEOF_DFU_GetStatusRequest_Packet 7

#pragma pack(push,1)
typedef struct DFU_Status_Reply_Packet_Struct
{
   uint8_t bStatus;
   uint16_t wPollTimeout_Low;
   uint8_t bPollTimeout_High;
   uint8_t bState;
   uint8_t iString;
} DFU_Status_Reply_Packet;
#pragma pack(pop)

#define SIZEOF_DFU_Status_Reply_Packet 6

#pragma pack(push,1)
typedef struct DFU_State_Reply_Packet_Struct
{
   uint8_t bState;

} DFU_State_Reply_Packet;
#pragma pack(pop)

#define SIZEOF_DFU_State_Reply_Packet 1


//-----------------------------------------------------------------------------
// DFU_GetInfo_Reply_Packet
//-----------------------------------------------------------------------------
//  Response to a DFU_GETINFO request
//
typedef struct DFU_GetInfo_Reply_Packet_Struct
{
   uint16_t length;
   uint8_t bBootloaderRevision_Minor;
   uint8_t bBootloaderRevision_Major;
   uint32_t bmAttributes;     // set to 0x07 for bitManifestationTolerant, bitCanDnload, bitCanUpload
   uint32_t wBlockSize;
   uint32_t maxAppSize;
   uint8_t bDeviceID[16]; // DEVICEID0
   uint8_t bUUID[16];

} DFU_GetInfo_Reply_Packet;

#define SIZEOF_DFU_GetInfo_Reply_Packet 48

//-----------------------------------------------------------------------------
// DFU_Flash_Type
//-----------------------------------------------------------------------------
// Persistant array stored in Flash and forms part of the information page
// inside a DFU firmware image file.
//
typedef struct DFU_Flash_Struct
{
   uint8_t bDfuFileRevision_Minor;
   uint8_t bDfuFileRevision_Major;
   uint8_t bAppRevision_Minor;
   uint8_t bAppRevision_Major;
   char  sAppName[16];  // null terminated string
   char  sTargetFamily[16]; // null terminated string
   uint8_t bReserved[12];
   
   uint32_t wAppSize;
   uint32_t wCrc;
   uint32_t wSignature;
   uint32_t wLock;


} DFU_Flash_Type;

#define SIZEOF_DFU_Flash_Type 64

//-----------------------------------------------------------------------------
// DFU_File_Type
//-----------------------------------------------------------------------------
// The information page of a DFU firmware image file. Contains a copy of
// the array stored in Flash plus additional information compiled into
// the bootloader or available only at runtime.
//
typedef struct DFU_File_Struct
{

   DFU_Flash_Type flash;

   uint32_t wAppStartAddress;
   uint32_t wBlockSize;
   uint32_t wFlashKey_A; 
   uint32_t wFlashKey_B; 

} DFU_File_Type;

#define SIZEOF_DFU_File_Type (SIZEOF_DFU_Flash_Type + 16)






