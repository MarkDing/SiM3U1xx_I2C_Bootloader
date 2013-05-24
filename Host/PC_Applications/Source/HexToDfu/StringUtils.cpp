/////////////////////////////////////////////////////////////////////////////
// StringUtils.cpp
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "StringUtils.h"

/////////////////////////////////////////////////////////////////////////////
// Namespaces
/////////////////////////////////////////////////////////////////////////////

using namespace std;

/////////////////////////////////////////////////////////////////////////////
// Global Functions
/////////////////////////////////////////////////////////////////////////////

// Return the string contents of an opened file
string FileReadString(FILE* pFile)
{
    string text;

    long size;
    size_t read;
    char* buffer = NULL;

    // Move to the end of the file
    fseek(pFile, 0, SEEK_END);

    // Get the file size in bytes
    size = ftell(pFile);

    // Move to the beginning of the file
    rewind(pFile);

    buffer = new char[size];

    // Read the entire file into a buffer
    read = fread(buffer, sizeof(char), size, pFile);

    // Read the whole file
    if (read == size)
    {
        text = string(buffer, size);
    }

    delete [] buffer;

    return text;
}

// Write the contents of a string to an opened file
BOOL FileWriteString(FILE* pFile, const string& text)
{
    size_t written = 0;

    written = fwrite(text.c_str(), sizeof(char), text.length(), pFile);

    return written == text.length();
}

// Return the next line in the string (including
// the newline (\n))
//
// Call this function the first time with pos = 0.
// pos will be updated each time to search for the
// next line.
//
// Returns an empty string ("") when there are no
// more lines.
string GetNextLine(const string& text, size_t& pos)
{
    size_t linePos = 0;

    if ((linePos = text.find('\n', pos)) != string::npos)
    {
        // Return the line with the newline (\n) included
        string line = text.substr(pos, linePos - pos + 1);

        pos = linePos + 1;

        return line;
    }

    return "";
}

// Remove leading whitespace from the beginning of the string
// and return the results in a new string
string TrimLeft(const string& text)
{
    string clean;

    // Number of whitespace characters to remove
    size_t numChars = 0;
    
    // Count the number of whitespace characters at the beginning
    // of the string
    for (size_t i = 0; i < text.length(); i++)
    {
        if (text[i] == '\r' ||
            text[i] == '\n' ||
            text[i] == '\t' ||
            text[i] == ' ')
        {
            numChars++;
        }
        else
        {
            break;
        }
    }

    clean = text.substr(numChars, text.length() - numChars);

    return clean;
}

// Remove trailing whitespace from the end of the string
// and return the results in a new string
string TrimRight(const string& text)
{
    string clean;

    // Number of whitespace characters to remove
    size_t numChars = 0;
    
    // Count the number of whitespace characters at the end
    // of the string
    for (int i = (int)text.length() - 1; i >= 0; i--)
    {
        if (text[i] == '\r' ||
            text[i] == '\n' ||
            text[i] == '\t' ||
            text[i] == ' ')
        {
            numChars++;
        }
        else
        {
            break;
        }
    }

    clean = text.substr(0, text.length() - numChars);

    return clean;
}

std::string FormatString(const char * format, ...)
{
    std::string text;
    char buffer[MAX_PATH];

    va_list args;
    va_start(args, format);
    vsprintf(buffer, format, args);
    text = buffer;
    va_end(args);

    return text;
}
