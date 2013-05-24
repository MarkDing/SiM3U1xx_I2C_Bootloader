/////////////////////////////////////////////////////////////////////////////
// HexFile.cpp
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "HexFile.h"
#include "StringUtils.h"

/////////////////////////////////////////////////////////////////////////////
// Definitions
/////////////////////////////////////////////////////////////////////////////

#define HEX_RECORD_TYPE_DATA            0x00 // 16-bits
#define HEX_RECORD_TYPE_END_OF_FILE     0x01
#define HEX_RECORD_TYPE_EXT_SEG_ADDR    0x02 // 20-bits
#define HEX_RECORD_TYPE_EXT_LIN_ADDR    0x04 // 32-bits
#define HEX_RECORD_TYPE_START_LIN_ADDR  0x05

/////////////////////////////////////////////////////////////////////////////
// Namespaces
/////////////////////////////////////////////////////////////////////////////

using namespace std;

/////////////////////////////////////////////////////////////////////////////
// Static Global Functions
/////////////////////////////////////////////////////////////////////////////

// Convert ASCII text representing hexadecimal byte values
// into a list of bytes
static vector<BYTE> AsciiHexToByteList(const string& text)
{
    vector<BYTE> list;

    for (size_t i = 0; i < text.length(); i += 2)
    {
        list.push_back((BYTE)strtoul(text.substr(i, 2).c_str(), NULL, 16));
    }

    return list;
}

// Return the least significant 8-bits of the sum of all of the
// byte values in the list
static BYTE Checksum(const vector<BYTE>& bytes)
{
    BYTE checksum = 0;

    for (size_t i = 0; i < bytes.size(); i++)
    {
        checksum += bytes[i];
    }

    return checksum;
}

// Convert a list of byte values to a string
// hex record line
static void AppendRecord(string& text, const vector<BYTE>& bytes)
{
    char temp[3];

    text += ":";

    BYTE checksum = 0;

    for (size_t i = 0; i < bytes.size(); i++)
    {
        sprintf_s(temp, sizeof(temp), "%02X", bytes[i]);
        text += temp;

        checksum += bytes[i];
    }

    checksum = 0x100 - checksum;
    sprintf_s(temp, sizeof(temp), "%02X", checksum);
    text += temp;

    text += "\r\n";
}

// Generate an intel hex record
static vector<BYTE> GenerateRecord(WORD address, BYTE recordType, vector<BYTE>::iterator dataBegin, vector<BYTE>::iterator dataEnd)
{
    vector<BYTE> bytes;

    BYTE count = (BYTE)distance(dataBegin, dataEnd);

    bytes.push_back(count);
    bytes.push_back(HIBYTE(address));
    bytes.push_back(LOBYTE(address));
    bytes.push_back(recordType);
    
    bytes.insert(bytes.end(), dataBegin, dataEnd);

    return bytes;
}

static vector<BYTE> GenerateDataRecord(WORD lowerAddress, vector<BYTE>::iterator dataBegin, vector<BYTE>::iterator dataEnd)
{
    return GenerateRecord(lowerAddress, HEX_RECORD_TYPE_DATA, dataBegin, dataEnd);
}

static vector<BYTE> GenerateEndOfFileRecord()
{
    vector<BYTE> data;

    return GenerateRecord(0x0000, HEX_RECORD_TYPE_END_OF_FILE, data.begin(), data.end());
}

static vector<BYTE> GenerateExtendedSegmentAddressRecord(WORD segmentAddress)
{
    vector<BYTE> data;

    data.push_back(HIBYTE(segmentAddress));
    data.push_back(LOBYTE(segmentAddress));

    return GenerateRecord(0x0000, HEX_RECORD_TYPE_EXT_SEG_ADDR, data.begin(), data.end());
}

static vector<BYTE> GenerateExtendedLinearAddressRecord(WORD linearAddress)
{
    vector<BYTE> data;

    data.push_back(HIBYTE(linearAddress));
    data.push_back(LOBYTE(linearAddress));

    return GenerateRecord(0x0000, HEX_RECORD_TYPE_EXT_LIN_ADDR, data.begin(), data.end());
}

/////////////////////////////////////////////////////////////////////////////
// CHexFile Class - Constructor/Destructor
/////////////////////////////////////////////////////////////////////////////

// blockSize - The size of every block in the map
//
// createEmptyBlocks - If TRUE, then return a full block filled
// with fillValue for a block that is not specified in the hex image.
// Otherwise return an empty block
//
// fillValue - Specify the value to be inserted into incomplete blocks
CHexFile::CHexFile(int blockSize, BOOL createEmptyBlocks, BYTE fillValue)
{
    m_blockSize             = blockSize;
    m_createEmptyBlocks     = createEmptyBlocks;
    m_fillValue             = fillValue;
    
    m_minAddress            = MAXDWORD;
    m_maxAddress            = 0;

    m_removeLock            = FALSE;
    m_lockAddress           = 0;
    m_isLockPresent         = FALSE;
    m_lockValue             = 0;

    if (blockSize == 0)
    {
        throw exception("Block size must be nonzero.");
    }

    if (blockSize % 16 != 0)
    {
        throw exception("Block size must be divisible by 16.");
    }
}

CHexFile::~CHexFile()
{
}

/////////////////////////////////////////////////////////////////////////////
// CHexFile Class - Public Reader Methods
/////////////////////////////////////////////////////////////////////////////

HexFileReaderStatus CHexFile::LoadFromString(const std::string& hexFileString, BOOL removeLock, DWORD address)
{
    HexFileReaderStatus     status          = HexOk;
    size_t                  pos             = 0;
    string                  line;
    AddressMode             addressMode     = _16bit;
    WORD                    linearAddress   = 0x0000;
    WORD                    segmentAddress  = 0x0000;

    // Clear the map
    Clear();

    m_removeLock = removeLock;
    m_lockAddress = address;

    if (!hexFileString.empty())
    {
        while (!(line = GetNextLine(hexFileString, pos)).empty())
        {
            // Remove whitespace
            line = TrimLeft(TrimRight(line));

            if (line.length() > 0)
            {
                // Parse the hex record in the line of text
                status = ParseHexRecord(line, addressMode, linearAddress, segmentAddress);

                if (status != HexOk)
                {
                    break;
                }
            }
        }
    }
    else
    {
        status = HexInvalidFile;
    }

    return status;
}

HexFileReaderStatus CHexFile::LoadFromFile(const std::string& hexFilePath, BOOL removeLock, DWORD address)
{
    HexFileReaderStatus     status          = HexOk;
    FILE*                   pFile;

    // Open the hex file
    if (fopen_s(&pFile, hexFilePath.c_str(), "rb") == 0)
    {
        // Copy the string contents of the file
        string hexText = FileReadString(pFile);
        fclose(pFile);

        status = LoadFromString(hexText, removeLock, address);
    }
    else
    {
        status = HexInvalidFile;
    }

    return status;
}

// Return the number of blocks specified in the hex file
int CHexFile::GetNumBlocks()
{
    int numBlocks = 0;
    
    // If map isn't empty
    if (m_blocks.rbegin() != m_blocks.rend())
    {
        // Maps are sorted in ascending order based on the key
        // which is the block ID
        numBlocks = m_blocks.rbegin()->first + 1;
    }

    return numBlocks;
}

// Return the block of data for the specified block ID
BlockData CHexFile::GetBlock(int blockID)
{
    BlockData   data;
    BOOL        blockExists = (m_blocks.count(blockID) == 1);

    // Return the existing block
    if (blockExists)
    {
        size_t bytesToCopy = min((size_t)m_blockSize, m_blocks[blockID].size());

        // Pad the block with the fill value before copying the block data
        data = BlockData(m_blockSize, m_fillValue);
        
        for (size_t i = 0; i < bytesToCopy; i++)
        {
            data[i] = m_blocks[blockID][i];
        }
    }
    // Block doesn't exist in the map, but
    // createEmptyBlocks is true
    else if (!blockExists && m_createEmptyBlocks)
    {
        // Create an empty block using the fill value
        data = BlockData(m_blockSize, m_fillValue);
    }

    return data;
}

void CHexFile::Clear()
{
    m_blocks.clear();

    m_minAddress    = MAXDWORD;
    m_maxAddress    = 0;

    m_removeLock    = FALSE;
    m_lockAddress   = 0;
    m_isLockPresent = FALSE;
    m_lockValue     = 0;
}

DWORD CHexFile::GetLock()
{
    if (m_removeLock && m_isLockPresent)
    {
        return m_lockValue;
    }

    return 0xFFFFFFFF;
}

/////////////////////////////////////////////////////////////////////////////
// CHexFile Class - Public Writer Methods
/////////////////////////////////////////////////////////////////////////////

void CHexFile::SetBlock(int blockID, const BlockData& data)
{
    m_blocks[blockID] = data;
}

void CHexFile::SetMap(const BlockMap& map)
{
    m_blocks = map;
}

void CHexFile::SaveToString(std::string& hexFileString)
{
    hexFileString = GenerateHexString();
}

BOOL CHexFile::SaveToFile(const std::string& hexFilePath)
{
    BOOL success = FALSE;

    string text = GenerateHexString();

    FILE* pFile;
    
    if (fopen_s(&pFile, hexFilePath.c_str(), "wb") == 0)
    {
        success = FileWriteString(pFile, text);

        fclose(pFile);
    }

    return success;
}

/////////////////////////////////////////////////////////////////////////////
// CHexFile Class - Protected Reader Methods
/////////////////////////////////////////////////////////////////////////////

HexFileReaderStatus CHexFile::ParseHexRecord(const string& line, AddressMode& addressMode, WORD& linearAddress, WORD& segmentAddress)
{
    HexFileReaderStatus success = HexInvalidRecord;

    // Hex Record
    // ----------
    // See http://en.wikipedia.org/wiki/Intel_HEX for more details
    //
    // :LLAAAATT[DD...]CC
    // LL   - byte count (number of data bytes)
    // AAAA - address (little-endian)
    // TT   - record type
    // DD   - data
    // CC   - checksum (add LL through DD and subtract from 0x100)

    if (line[0] == ':')
    {
        // Convert the ascii hex values to a list of byte values
        vector<BYTE> bytes = AsciiHexToByteList(line.substr(1, line.length() - 1));

        // Check for minimum hex record size
        if (bytes.size() >= 5)
        {
            int byteCount   = bytes[0];
            int address     = MAKEWORD(bytes[2], bytes[1]);
            int recordType  = bytes[3];

            // Verify number of bytes in the hex record
            if (bytes.size() == 5 + byteCount)
            {
                int checksum = Checksum(bytes);

                // Ignore the checksum, but it should be 0x00

                // Record contains data
                if (recordType == HEX_RECORD_TYPE_DATA)
                {
                    // Get the complete 32-bit address
                    DWORD address32 = address;
                    
                    if (addressMode == _20bit)
                    {
                        address32 = ((((DWORD)segmentAddress) << 4) + address) & 0xFFFFF;
                    }
                    else if (addressMode == _32bit)
                    {
                        address32 = (((DWORD)linearAddress) << 16) | address;
                    }

                    // Add each data byte to the map
                    for (int i = 0; i < byteCount; i++)
                    {
                        AddDataByte(address32 + i, bytes[i+4]);
                    }

                    success = HexOk;
                }
                // Record contains 16-bit segment address that is shifted 4 bits
                // to the left and added to the data address
                else if (recordType == HEX_RECORD_TYPE_EXT_SEG_ADDR)
                {
                    // Must have 2 data bytes specifying the segment address
                    if (byteCount == 2)
                    {
                        segmentAddress = MAKEWORD(bytes[5], bytes[4]);
                        success = HexOk;
                        addressMode = _20bit;
                    }
                    else
                    {
                        success = HexInvalidRecord;
                    }
                }
                // Record contains the upper 2-bytes of the 32-bit address
                else if (recordType == HEX_RECORD_TYPE_EXT_LIN_ADDR)
                {
                    // Must have 2 data bytes specifying the upper address
                    if (byteCount == 2)
                    {
                        linearAddress = MAKEWORD(bytes[5], bytes[4]);
                        success = HexOk;
                        addressMode = _32bit;
                    }
                    else
                    {
                        success = HexInvalidRecord;
                    }
                }
                else
                {
                    // Ignore unknown hex record types
                    success = HexOk;
                }
            }
        }
    }

    return success;
}

// Add the data byte to the block map
void CHexFile::AddDataByte(DWORD address, BYTE value)
{
    // Remove the lock word from the hex file data
    if (m_removeLock)
    {
        // Address is in the lock word range
        if (address >= m_lockAddress && address < m_lockAddress + sizeof(m_lockValue))
        {
            int offset = address - m_lockAddress;
            ((BYTE*)(&m_lockValue))[offset] = value;
            return;
        }
    }

    int blockID     = address / m_blockSize;
    int blockIndex  = address % m_blockSize;

    // Block doesn't exist
    if (m_blocks.count(blockID) == 0)
    {
        // Create a new block initialized with the default fill value
        m_blocks[blockID] = BlockData(m_blockSize, m_fillValue);
    }

    // Add the value to the appropriate block in the map
    m_blocks[blockID][blockIndex] = value;

    m_minAddress = min(address, m_minAddress);
    m_maxAddress = max(address, m_maxAddress);
}

/////////////////////////////////////////////////////////////////////////////
// CHexFile Class - Protected Writer Methods
/////////////////////////////////////////////////////////////////////////////

string CHexFile::GenerateHexString()
{
    string text;

    int numBlocks = GetNumBlocks();

    for (int i = 0; i < numBlocks; i++)
    {
        BlockData block = GetBlock(i);

        if (block.size() > 0)
        {
            DWORD blockAddress = i * m_blockSize;

            AppendRecord(text, GenerateExtendedLinearAddressRecord(blockAddress >> 16));

            for (size_t i = 0; i < block.size(); i += 16)
            {
                DWORD recordAddress = (DWORD)(blockAddress + i);

                AppendRecord(text, GenerateDataRecord(LOWORD(recordAddress), block.begin() + i, block.begin() + i + 16));
            }
        }
    }

    AppendRecord(text, GenerateEndOfFileRecord());

    return text;
}
