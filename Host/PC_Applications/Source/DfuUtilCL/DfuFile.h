/////////////////////////////////////////////////////////////////////////////
// DfuFile.h
/////////////////////////////////////////////////////////////////////////////

#pragma once

/////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////

#include <Windows.h>
#include <string>
#include <vector>

/////////////////////////////////////////////////////////////////////////////
// Definitions
/////////////////////////////////////////////////////////////////////////////

#define DFU_FILE_REV_MAJOR              0x01
#define DFU_FILE_REV_MINOR              0x00

#define DFU_FILE_SIGNATURE_MASK         0xFF0000FF
#define DFU_FILE_SIGNATURE              0xA5FFFF5A

/////////////////////////////////////////////////////////////////////////////
// Enumerations
/////////////////////////////////////////////////////////////////////////////

enum DfuCode
{
    DfuOk,
    DfuInvalidPart,
    DfuInvalidHexRecord,
    DfuInvalidCrc,
    DfuInvalidSignature,
    DfuInvalidSize
};

/////////////////////////////////////////////////////////////////////////////
// Structures
/////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
// DFU_Flash_Type
//-----------------------------------------------------------------------------
// Persistant array stored in Flash and forms part of the information page
// inside a DFU firmware image file. 
//
typedef struct DFU_Flash_Struct
{
   BYTE bDfuFileRevision_Minor;
   BYTE bDfuFileRevision_Major;
   BYTE bAppRevision_Minor;
   BYTE bAppRevision_Major;   
   char sAppName[16];      // null terminated string
   char sTargetFamily[16]; // null terminated string
   BYTE bReserved[12];
   DWORD wAppSize;
   DWORD wCrc;
   DWORD wSignature;
   DWORD wLock;
   
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
   
   DWORD wAppStartAddress;
   DWORD wBlockSize;
   DWORD wFlashKey_A; // Only valid for DNLOAD
   DWORD wFlashKey_B; // Only valid for DNLOAD
    
} DFU_File_Type;

#define SIZEOF_DFU_File_Type (SIZEOF_DFU_Flash_Type + 16)

/////////////////////////////////////////////////////////////////////////////
// Global Function Prototypes
/////////////////////////////////////////////////////////////////////////////

DfuCode ConvertHexToDfu(
    const std::string& hexFileString,
    BYTE** dfuFile,
    DWORD* dfuFileSize,
    const std::string& partNumber, 
    const std::string& family,
    const std::string& appName,
    BYTE appMajor,
    BYTE appMinor,
    BOOL locked);
