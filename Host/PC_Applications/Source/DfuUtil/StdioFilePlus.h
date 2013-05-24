// StdioFilePlus.h: interface for the CStdioFilePlus class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_STDIOFILEPLUS_H__F93D0E79_DAC2_45D2_9611_B323A4497DB6__INCLUDED_)
#define AFX_STDIOFILEPLUS_H__F93D0E79_DAC2_45D2_9611_B323A4497DB6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////
// Includes
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// CStdioFilePlus Class:
// Enhanced version of the CStdioFile MFC class
// which uses ReadLine() and WriteLine() to read/write
// various data types to an individual line in a text file
//////////////////////////////////////////////////////////////////////

class CStdioFilePlus : public CStdioFile  
{
// Constructor/Destructor
public:
	CStdioFilePlus();
	virtual ~CStdioFilePlus();

// Public Methods
public:
	// Read Methods
	BOOL ReadCString(CString& value);

	BOOL ReadBool(BOOL& value);

	BOOL ReadChar(char& value);
	BOOL ReadShort(short& value);
	BOOL ReadInt(int& value);
	BOOL ReadLong(long& value);
	
	BOOL ReadByte(BYTE& value);
	BOOL ReadWord(WORD& value);
	BOOL ReadUint(UINT& value);
	BOOL ReadDword(DWORD& value);
	
	BOOL ReadHexByte(BYTE& value);
	BOOL ReadHexWord(WORD& value);
	BOOL ReadHexUint(UINT& value);
	BOOL ReadHexDword(DWORD& value);
	
	BOOL ReadDouble(double& value);

	// Write methods
	void WriteCString(CString value);

	void WriteBool(BOOL value);

	void WriteChar(char value);
	void WriteShort(short value);
	void WriteInt(int value);
	void WriteLong(long value);
	
	void WriteByte(BYTE value);
	void WriteWord(WORD value);
	void WriteUint(UINT value);
	void WriteDword(DWORD value);
	
	void WriteHexByte(BYTE value);
	void WriteHexWord(WORD value);
	void WriteHexUint(UINT value);
	void WriteHexDword(DWORD value);
	
	void WriteDouble(double value);
};

#endif // !defined(AFX_STDIOFILEPLUS_H__F93D0E79_DAC2_45D2_9611_B323A4497DB6__INCLUDED_)
