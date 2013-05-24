/////////////////////////////////////////////////////////////////////////////
// DfuFile.cpp
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DfuFile.h"
#include "HexFile.h"
#include "Crc.h"
#include "StringUtils.h"
#include "DeviceInfo.h"

/////////////////////////////////////////////////////////////////////////////
// Namespaces
/////////////////////////////////////////////////////////////////////////////

using namespace std;

/////////////////////////////////////////////////////////////////////////////
// Global Functions
/////////////////////////////////////////////////////////////////////////////

// Convert an Intel hex image store in a string to a binary array representing
// the DFU file.
//
// Insert the parameters into the DFU file.
//
// Use the block size to generate a clean hex image in the DFU data section.
DfuCode ConvertHexToDfu(
    const string& hexFileString,
    BYTE** dfuFile,
    DWORD* dfuFileSize,
    const string& partNumber, 
    const string& family,
    const string& appName,
    BYTE appMajor,
    BYTE appMinor,
    BOOL locked)
{
    DfuCode             code = DfuOk;
    DeviceInformation   devInfo;

    // Get information about the part
    if (!GetDeviceInfo(partNumber, &devInfo))
    {
        return DfuInvalidPart;
    }

    CHexFile                hex(devInfo.wBlockSize, TRUE, 0xFF);
    HexFileReaderStatus     status;

    // Read the hex input file
    status = hex.LoadFromString(hexFileString, TRUE, devInfo.wFlashLockAddress);

    if (status == HexOk)
    {
        DWORD minAddress = hex.GetMinAddress();
        DWORD maxAddress = hex.GetMaxAddress();

        ///////////////////////////////
        // Build block 0 (info page) //
        ///////////////////////////////

        DFU_File_Type info;

        memset(&info, 0x00, sizeof(info));

        info.flash.bDfuFileRevision_Minor   = DFU_FILE_REV_MINOR;
        info.flash.bDfuFileRevision_Major   = DFU_FILE_REV_MAJOR;        
        info.flash.bAppRevision_Minor       = appMinor;
        info.flash.bAppRevision_Major       = appMajor;        

        strcpy_s(info.flash.sAppName, sizeof(info.flash.sAppName), appName.c_str());
        strcpy_s(info.flash.sTargetFamily, sizeof(info.flash.sTargetFamily), family.c_str());
        memset(info.flash.bReserved, 0xFF, sizeof(info.flash.bReserved));

        info.flash.wAppSize                 = (maxAddress - minAddress + 1);
        info.flash.wCrc                     = 0;    // Calculate this later
        info.flash.wSignature               = DFU_FILE_SIGNATURE;
        info.flash.wLock                    = locked ? 0x11111111 : hex.GetLock();

        info.wAppStartAddress               = minAddress;
        info.wBlockSize                     = devInfo.wBlockSize;
        info.wFlashKey_A                    = devInfo.wFlashKey_A;
        info.wFlashKey_B                    = devInfo.wFlashKey_B;

        ////////////////////////////////
        // Build the DFU binary array //
        ////////////////////////////////

        int hexNumBlocks    = hex.GetNumBlocks();
        int appStartBlock   = minAddress / devInfo.wBlockSize;
        int appNumBlocks    = hexNumBlocks - appStartBlock;

        DWORD imageSize = (appNumBlocks + 1) * devInfo.wBlockSize;
        BYTE* image = new BYTE[imageSize];
        int imageBlock = 0;

        // Initialize the image to 0xCD
        memset(image, 0xCD, imageSize);

        // Copy block 0 into the image (pad block 0 with 0x00)
        memset(image, 0x00, devInfo.wBlockSize);
        memcpy(image, &info, sizeof(info));
        imageBlock++;

        // Copy each block from the hex file into the image
        for (int i = appStartBlock; i < hexNumBlocks; i++)
        {
            BlockData block = hex.GetBlock(i);

            for (size_t j = 0; j < block.size(); j++)
            {
                image[imageBlock * devInfo.wBlockSize + j] = block[j];
            }

            imageBlock++;
        }

        // Initialize the CRC accumulator
        DWORD crc = 0xFFFFFFFF;

        // The offset between hex addresses and indexes in the DFU image
        int hex_dfu_offset = info.wAppStartAddress - devInfo.wBlockSize;

        // The CRC includes wAppSize bytes starting at wAppStart
        for (DWORD i = info.wAppStartAddress; i < (info.wAppStartAddress + info.flash.wAppSize); i++)
        {
            _crc(crc, image[i - hex_dfu_offset]);
        }

        crc ^= 0xFFFFFFFF;

        // Store the CRC in the info page
        ((DFU_File_Struct*)image)->flash.wCrc = crc;

        *dfuFile = image;
        *dfuFileSize = imageSize;
    }
    else if (status == HexInvalidRecord)
    {
        code = DfuInvalidHexRecord;
    }
    else if (status == HexInvalidFile)
    {
        code = DfuInvalidHexRecord;
    }

    return code;
}
