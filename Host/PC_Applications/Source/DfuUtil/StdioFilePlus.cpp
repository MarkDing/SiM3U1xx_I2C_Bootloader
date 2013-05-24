// StdioFilePlus.cpp: implementation of the CStdioFilePlus class.
//
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Includes
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "StdioFilePlus.h"
#include <limits>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Static Global Functions
//////////////////////////////////////////////////////////////////////

template <typename T> static BOOL ReadValue(CStdioFilePlus* file, T& value)
{
	BOOL		retVal		= FALSE;
	bool		isSigned	= std::numeric_limits<T>::is_signed;
	CString		line;

	if (file->ReadString(line))
	{
		retVal = TRUE;
	}

	if (isSigned)
	{
		value = (T)_tcstol(line, NULL, 10);
	}
	else
	{
		value = (T)_tcstoul(line, NULL, 10);
	}

	return retVal;
}

template <typename T> static BOOL ReadHexValue(CStdioFilePlus* file, T& value)
{
	BOOL		retVal = FALSE;
	CString		line;

	if (file->ReadString(line))
	{
		value	= (T)_tcstoul(line, NULL, 16);
		retVal	= TRUE;
	}

	return retVal;
}

template <typename T> static void WriteValue(CStdioFilePlus* file, T value)
{
	CString		line;
	bool		isSigned = std::numeric_limits<T>::is_signed;

	if (isSigned)
	{
		line.Format(_T("%ld\n"), (long)value);
	}
	else
	{
		line.Format(_T("%lu\n"), (unsigned long)value);
	}

	file->WriteString(line);
}

template<typename T> static void WriteHexValue(CStdioFilePlus* file, T value)
{
	CString		line;
	int			width = 2 * sizeof(value);

	// Format the string line:
	// "0x" is inserted as plain text
	// "%0*lX"
	//   0 - pad to the specified width with 0's (ie for a BYTE, output "0x01" instead of "0x1")
	//   * - width is specified as the first parameter (ie for a BYTE, width is 2*1 = 2 characters)
	//   lX - output long capitalized hexadecimal number
	line.Format(_T("0x%0*lX\n"), width, (unsigned long)value);
	file->WriteString(line);
}

//////////////////////////////////////////////////////////////////////
// CStdioFilePlus Class - Constructor/Destructor
//////////////////////////////////////////////////////////////////////

CStdioFilePlus::CStdioFilePlus()
{

}

CStdioFilePlus::~CStdioFilePlus()
{

}

//////////////////////////////////////////////////////////////////////
// CStdioFilePlus Class - Public Read Methods
//////////////////////////////////////////////////////////////////////

BOOL CStdioFilePlus::ReadCString(CString& value)
{
	return ReadString(value);
}

BOOL CStdioFilePlus::ReadBool(BOOL& value)
{
	return ReadValue<BOOL>(this, value);
}

BOOL CStdioFilePlus::ReadChar(char& value)
{
	return ReadValue<char>(this, value);
}

BOOL CStdioFilePlus::ReadShort(short& value)
{
	return ReadValue<short>(this, value);
}

BOOL CStdioFilePlus::ReadInt(int& value)
{
	return ReadValue<int>(this, value);
}

BOOL CStdioFilePlus::ReadLong(long& value)
{
	return ReadValue<long>(this, value);
}

BOOL CStdioFilePlus::ReadByte(BYTE& value)
{
	return ReadValue<BYTE>(this, value);
}

BOOL CStdioFilePlus::ReadWord(WORD& value)
{
	return ReadValue<WORD>(this, value);
}

BOOL CStdioFilePlus::ReadUint(UINT& value)
{
	return ReadValue<UINT>(this, value);
}

BOOL CStdioFilePlus::ReadDword(DWORD& value)
{
	return ReadValue<DWORD>(this, value);
}

BOOL CStdioFilePlus::ReadHexByte(BYTE& value)
{
	return ReadHexValue<BYTE>(this, value);
}

BOOL CStdioFilePlus::ReadHexWord(WORD& value)
{
	return ReadHexValue<WORD>(this, value);
}

BOOL CStdioFilePlus::ReadHexUint(UINT& value)
{
	return ReadHexValue<UINT>(this, value);
}

BOOL CStdioFilePlus::ReadHexDword(DWORD& value)
{
	return ReadHexValue<DWORD>(this, value);
}

BOOL CStdioFilePlus::ReadDouble(double& value)
{
	BOOL		retVal = FALSE;
	CString		line;

	if (ReadString(line))
	{
		value	= _tcstod(line, NULL);
		retVal	= TRUE;
	}

	return retVal;
}

//////////////////////////////////////////////////////////////////////
// CStdioFilePlus Class - Public Write Methods
//////////////////////////////////////////////////////////////////////

void CStdioFilePlus::WriteCString(CString value)
{
	WriteString(value + _T("\n"));
}

void CStdioFilePlus::WriteBool(BOOL value)
{
	WriteValue<BOOL>(this, value);
}

void CStdioFilePlus::WriteChar(char value)
{
	WriteValue<char>(this, value);
}

void CStdioFilePlus::WriteShort(short value)
{
	WriteValue<short>(this, value);
}

void CStdioFilePlus::WriteInt(int value)
{
	WriteValue<int>(this, value);
}

void CStdioFilePlus::WriteLong(long value)
{
	WriteValue<long>(this, value);
}

void CStdioFilePlus::WriteByte(BYTE value)
{
	WriteValue<BYTE>(this, value);
}

void CStdioFilePlus::WriteWord(WORD value)
{
	WriteValue<WORD>(this, value);
}

void CStdioFilePlus::WriteUint(UINT value)
{
	WriteValue<UINT>(this, value);
}

void CStdioFilePlus::WriteDword(DWORD value)
{
	WriteValue<DWORD>(this, value);
}

void CStdioFilePlus::WriteHexByte(BYTE value)
{
	WriteHexValue<BYTE>(this, value);
}

void CStdioFilePlus::WriteHexWord(WORD value)
{
	WriteHexValue<WORD>(this, value);
}

void CStdioFilePlus::WriteHexUint(UINT value)
{
	WriteHexValue<UINT>(this, value);
}

void CStdioFilePlus::WriteHexDword(DWORD value)
{
	WriteHexValue<DWORD>(this, value);
}

void CStdioFilePlus::WriteDouble(double value)
{
	CString line;
	line.Format(_T("%f\n"), value);
	WriteString(line);
}
