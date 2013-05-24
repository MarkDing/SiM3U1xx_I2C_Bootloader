/////////////////////////////////////////////////////////////////////////////
// HexFile.h
/////////////////////////////////////////////////////////////////////////////

#pragma once

/////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////

#include <map>
#include <vector>
#include <string>

enum AddressMode
{
    _16bit, // Record type 0x00
    _20bit, // Record type 0x02 - Extended Segment Address
    _32bit  // Record type 0x04 - Extended Linear Address
};

/////////////////////////////////////////////////////////////////////////////
// Type Definitions
/////////////////////////////////////////////////////////////////////////////

typedef std::vector<BYTE>           BlockData;
typedef std::map<int, BlockData>    BlockMap;

/////////////////////////////////////////////////////////////////////////////
// Structures
/////////////////////////////////////////////////////////////////////////////

enum HexFileReaderStatus
{
    HexOk,
    HexInvalidFile,
    HexInvalidRecord
};

/////////////////////////////////////////////////////////////////////////////
// CHexFile Class
/////////////////////////////////////////////////////////////////////////////

class CHexFile
{
// Constructor/Destructor
public:
    CHexFile(int blockSize, BOOL createEmptyBlocks = TRUE, BYTE fillValue = 0xFF);
    ~CHexFile();

// Public Reader Methods
public:
    HexFileReaderStatus     LoadFromString(const std::string& hexFileString, BOOL removeLock = FALSE, DWORD address = 0);
    HexFileReaderStatus     LoadFromFile(const std::string& hexFilePath, BOOL removeLock = FALSE, DWORD address = 0);
    int                     GetNumBlocks();
    BlockData               GetBlock(int blockID);
    void                    Clear();

    DWORD                   GetMinAddress() { return m_minAddress; }
    DWORD                   GetMaxAddress() { return m_maxAddress; }

    DWORD                   GetLock();

// Public Writer Methods
public:
    void                    SetBlock(int blockID, const BlockData& data);
    void                    SetMap(const BlockMap& map);
    void                    SaveToString(std::string& hexFileString);
    BOOL                    SaveToFile(const std::string& hexFilePath);

// Protected Reader Methods
protected:
    HexFileReaderStatus     ParseHexRecord(const std::string& line, AddressMode& addressMode, WORD& linearAddress, WORD& segmentAddress);
    void                    AddDataByte(DWORD address, BYTE value);

// Protected Writer Methods
protected:
    std::string             GenerateHexString();

// Protected Members
protected:
    int                     m_blockSize;
    BOOL                    m_createEmptyBlocks;
    BYTE                    m_fillValue;
    BlockMap                m_blocks;
    
    DWORD                   m_minAddress;
    DWORD                   m_maxAddress;

    BOOL                    m_removeLock;
    DWORD                   m_lockAddress;
    BOOL                    m_isLockPresent;
    DWORD                   m_lockValue;
};
