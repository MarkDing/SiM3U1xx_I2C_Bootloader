/////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GuidUtils.h"

/////////////////////////////////////////////////////////////////////////////
// Global Functions
/////////////////////////////////////////////////////////////////////////////

CString GuidToString(GUID guid)
{
    CString text;

    text.Format(_T("%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x"),
		guid.Data1,
		guid.Data2,
		guid.Data3,
		guid.Data4[0],
		guid.Data4[1],
		guid.Data4[2],
		guid.Data4[3],
		guid.Data4[4],
		guid.Data4[5],
		guid.Data4[6],
		guid.Data4[7]);

    return text;
}

BOOL IsValidGuid(CString text)
{
    // Valid GUID example: 04C77E93-B54D-4714-AC96-15A62BFA307B
    if (text.GetLength() != 36)
        return FALSE;

    for (int i = 0; i < text.GetLength(); i++)
    {
        if (i == 8 || i == 13 || i == 18 || i == 23)
        {
            if (text[i] != '-')
                return FALSE;
        }
        else
        {
            // If not a valid hexadecimal character
            if (CString(text[i]).FindOneOf(_T("0123456789abcdefABCDEF")) == -1)
            {
                return FALSE;
            }
        }
    }

    return TRUE;
}

GUID StringToGuid(CString text)
{
    GUID guid;

    memset(&guid, 0x00, sizeof(GUID));

    if (!IsValidGuid(text))
        return guid;

    guid.Data1      = (DWORD)_tcstoul(text.Mid(0, 8), NULL, 16);
    guid.Data2      = (WORD)_tcstoul(text.Mid(9, 4), NULL, 16);
    guid.Data3      = (WORD)_tcstoul(text.Mid(14, 4), NULL, 16);
    guid.Data4[0]   = (BYTE)_tcstoul(text.Mid(19, 2), NULL, 16);
    guid.Data4[1]   = (BYTE)_tcstoul(text.Mid(21, 2), NULL, 16);
    guid.Data4[2]   = (BYTE)_tcstoul(text.Mid(24, 2), NULL, 16);
    guid.Data4[3]   = (BYTE)_tcstoul(text.Mid(26, 2), NULL, 16);
    guid.Data4[4]   = (BYTE)_tcstoul(text.Mid(28, 2), NULL, 16);
    guid.Data4[5]   = (BYTE)_tcstoul(text.Mid(30, 2), NULL, 16);
    guid.Data4[6]   = (BYTE)_tcstoul(text.Mid(32, 2), NULL, 16);
    guid.Data4[7]   = (BYTE)_tcstoul(text.Mid(34, 2), NULL, 16);

    return guid;
}
