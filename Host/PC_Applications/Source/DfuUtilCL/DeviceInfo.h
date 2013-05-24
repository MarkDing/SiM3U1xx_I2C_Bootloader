#pragma once

/////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////

#include <string>
#include <vector>

/////////////////////////////////////////////////////////////////////////////
// Structures
/////////////////////////////////////////////////////////////////////////////

struct DeviceInformation
{
    DWORD   wBlockSize;
    DWORD   wFlashKey_A;
    DWORD   wFlashKey_B;
    DWORD   wFlashLockAddress;

    DeviceInformation()
    {
        wBlockSize = 0;
        wFlashKey_A = 0;
        wFlashKey_B = 0;
        wFlashLockAddress = 0;
    }

    DeviceInformation(DWORD wBlockSize, DWORD wFlashKey_A, DWORD wFlashKey_B, DWORD wFlashLockAddress)
    {
        this->wBlockSize = wBlockSize;
        this->wFlashKey_A = wFlashKey_A;
        this->wFlashKey_B = wFlashKey_B;
        this->wFlashLockAddress = wFlashLockAddress;
    }
};

/////////////////////////////////////////////////////////////////////////////
// Prototypes
/////////////////////////////////////////////////////////////////////////////

std::string ResolvePartNumber(const std::string& partNum);
BOOL GetDeviceInfo(const std::string& partNum, DeviceInformation* info);
std::vector<std::string> GetDeviceNames();
