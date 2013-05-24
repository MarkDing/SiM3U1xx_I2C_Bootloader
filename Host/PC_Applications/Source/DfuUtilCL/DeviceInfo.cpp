/////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DeviceInfo.h"
#include <map>

/////////////////////////////////////////////////////////////////////////////
// Namespaces
/////////////////////////////////////////////////////////////////////////////

using namespace std;

/////////////////////////////////////////////////////////////////////////////
// Static Function Prototypes
/////////////////////////////////////////////////////////////////////////////

static BOOL MaskedAreEqual(const string& lhs, const string& rhs, char mask = 'x');
static int MaskedStrLen(const string& str, char mask = 'x');

/////////////////////////////////////////////////////////////////////////////
// Static Global Variables
/////////////////////////////////////////////////////////////////////////////

class DeviceInfoMap
{
// Constructor
public:
    DeviceInfoMap()
    {
        //   Part Number                      Block Size    FLKEY[2]    Lock Address
        //   -----------                      ----------    --------    ------------
        m_map["SiM3U13x"] = DeviceInformation(1024,         0xA5, 0xF1, 0x0003FFFC);
        m_map["SiM3U14x"] = DeviceInformation(1024,         0xA5, 0xF1, 0x0003FFFC);
        m_map["SiM3U15x"] = DeviceInformation(1024,         0xA5, 0xF1, 0x0003FFFC);
        m_map["SiM3U16x"] = DeviceInformation(1024,         0xA5, 0xF1, 0x0003FFFC);
        m_map["SiM3xxxx"] = DeviceInformation(1024,         0xA5, 0xF1, 0x0003FFFC);
    }

// Public Methods
public:
    string ResolvePartNumber(const string& partNum)
    {
        for (map<string, DeviceInformation>::iterator it = m_map.begin(); it != m_map.end(); it++)
        {
            if (MaskedAreEqual(it->first, partNum, 'x'))
            {
                return it->first;
            }
        }

        return "";
    }
    
    BOOL GetDeviceInfo(const string& partNum, DeviceInformation* info)
    {
        string resolvedPartNum = ResolvePartNumber(partNum);

        if (!resolvedPartNum.empty())
        {
            *info = m_map[resolvedPartNum];
            return TRUE;
        }

        return FALSE;
    }

    vector<string> GetDeviceNames(int minNumChars = 7)
    {
        vector<string> names;

        for (map<string, DeviceInformation>::iterator it = m_map.begin(); it != m_map.end(); it++)
        {
            if (MaskedStrLen(it->first) >= minNumChars)
                names.push_back(it->first);
        }

        return names;
    }

// Public Members
public:
    map<string, DeviceInformation>  m_map;
};

static DeviceInfoMap Map;

/////////////////////////////////////////////////////////////////////////////
// Static Functions
/////////////////////////////////////////////////////////////////////////////

// Compare two strings for equality. Return TRUE if they are "equal".
// 1. Case doesn't matter.
// 2. Letters in either string that match the mask character are not used
//    for comparison.
// 3. If the two strings are not the same length, then only compare the first
//    characters based on the minimum string length of the two strings.
static BOOL MaskedAreEqual(const std::string& lhs, const std::string& rhs, char xMask)
{
    size_t len = min(lhs.length(), rhs.length());

    xMask = toupper(xMask);

    // Use the shorter string length for comparison
    // (ignore the rest of the characters)
    for (size_t i = 0; i < len; i++)
    {
        // If either character is the mask character, then
        // the character position is a "don't care"
        if (toupper(lhs[i]) != xMask && toupper(rhs[i]) != xMask)
        {
            if (toupper(lhs[i]) != toupper(rhs[i]))
                return FALSE;
        }
    }

    return TRUE;
}

// Return the length of a string up to, but excluding the first mask character
static int MaskedStrLen(const string& str, char mask)
{
    int len = 0;

    for (size_t i = 0; i < str.length(); i++)
    {
        if (toupper(str[i]) == toupper(mask))
            break;

        len++;
    }

    return len;
}

/////////////////////////////////////////////////////////////////////////////
// Global Functions
/////////////////////////////////////////////////////////////////////////////

string ResolvePartNumber(const string& partNum)
{
    return Map.ResolvePartNumber(partNum);
}

// Return the DeviceInformation structure for a given part number
BOOL GetDeviceInfo(const std::string& partNum, DeviceInformation* info)
{
    return Map.GetDeviceInfo(partNum, info);
}

std::vector<std::string> GetDeviceNames()
{
    return Map.GetDeviceNames();
}
