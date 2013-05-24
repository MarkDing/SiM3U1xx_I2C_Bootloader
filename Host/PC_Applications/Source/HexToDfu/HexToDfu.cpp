// HexToDfu.cpp : Defines the entry point for the console application.
//

/////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <stdio.h>
#include <vector>
#include <string>
#include <algorithm>
#include "DfuFile.h"
#include "StringUtils.h"
#include "DeviceInfo.h"

/////////////////////////////////////////////////////////////////////////////
// Definitions
/////////////////////////////////////////////////////////////////////////////

#define SUCCESS                         0
#define ERROR_INVALID_INPUT_PATH        1
#define ERROR_INVALID_ARG               2
#define ERROR_INVALID_HEX_PATH          3
#define ERROR_INVALID_HEX_RECORD        4
#define ERROR_HEX_CONVERSION_FAILED     5
#define ERROR_INVALID_DFU_PATH          6
#define ERROR_DFU_GENERATION_FAILED     7

/////////////////////////////////////////////////////////////////////////////
// Namespaces
/////////////////////////////////////////////////////////////////////////////

using namespace std;

/////////////////////////////////////////////////////////////////////////////
// Static Global Variables
/////////////////////////////////////////////////////////////////////////////

// Argument values
static string Input;
static string Output;
static string PartNumber = "SiM3U16x";
static string Family = "SiM3U1xx";
static string AppName = "";
static BYTE VersionMajor = 0x01;
static BYTE VersionMinor = 0x00;
static BOOL Locked = FALSE;

/////////////////////////////////////////////////////////////////////////////
// Static Global Functions
/////////////////////////////////////////////////////////////////////////////

static void PrintError(int code)
{
    switch (code)
    {
    case SUCCESS:
        printf("Success.\n");
        break;
    case ERROR_INVALID_INPUT_PATH:
        printf("Invalid input path.\n");
        break;
    case ERROR_INVALID_ARG:
        printf("Invalid arguments.\n");
        break;
    case ERROR_INVALID_HEX_PATH:
        printf("Invalid hex path.\n");
        break;
    case ERROR_INVALID_HEX_RECORD:
        printf("Invalid hex record.\n");
        break;
    case ERROR_HEX_CONVERSION_FAILED:
        printf("Hex conversion failed.\n");
        break;
    case ERROR_INVALID_DFU_PATH:
        printf("Invalid DFU path.\n");
        break;
    case ERROR_DFU_GENERATION_FAILED:
        printf("DFU generation failed.\n");
        break;
    default:
        printf("Error code (%d).\n", code);
    }
}

// Display the command line argument help text to the console
static void PrintUsage()
{
    printf("Silicon Laboratories Hex to DFU File Converter 1.0\n");
    printf("==================================================\n");
    printf("\n");
    printf("Converts an Intel hex file to a Silicon Labs Device\n");
    printf("Firmware Upgrade (DFU) file.\n");
    printf("\n");
    printf("HexToDfu -listparts\n");
    printf("\n");
    printf("-listparts \tDisplays a list of all supported part numbers\n");
    printf("\n");
    printf("HexToDfu [drive:][path]input [[drive:][path]output]\n");
    printf("[-part(partnumber)] [-family(target)] [-appname(name)]\n");
    printf("[-appversion(version)] [-locked]\n");
    printf("\n");
    printf("input       Specifies the Intel hex file to convert (*.hex).\n");
    printf("output      Specifies the DFU file to create (*.dfu).\n");
    printf("-part       Indicates that a part number will be specified.\n");
    printf("partnumber  Specifies the part number that the created\n");
    printf("            DFU image will be used for (see -listparts).\n");
    printf("-family     Indicates that a target device family will be\n");
    printf("            specified.\n");
    printf("target      Specifies the family name that the DFU image is\n");
    printf("            compatible with.\n");
    printf("-appname    Indicates that a firmware application name will be\n");
    printf("            specified.\n");
    printf("name        Specifies the firmware application name (15-characters).\n");
    printf("-appversion Indicates that a firmware application version will be\n");
    printf("            specified.\n");
    printf("version     Specifies the firmware application version\n");
    printf("            (BCD major.minor).\n");
    printf("-locked     Indicates that the device will be locked after the image\n");
    printf("            is downloaded.\n");
    printf("\n");
    printf("Defaults:\n");
    printf("output      - input file name with the .dfu extension\n");
    printf("part        - SiM3U16x\n");
    printf("family      - SiM3U1xx\n");
    printf("name        - \n");
    printf("version     - 1.00\n");
    printf("-locked     - not set\n");
    printf("\n");
    printf("Examples:\n");
    printf("HexToDfu -listparts\n");
    printf("HexToDfu blinky.hex blinky.dfu -part(SiM3U16x) -family(SiM3U1xx)\n");
    printf("    -appname(Blinky) -appversion(1.00) -locked\n");
    printf("\n");
}

static void PrintPartNumbers()
{
    vector<string> names = GetDeviceNames();

    printf("Part Number List\n");
    printf("================\n");

    for (size_t i = 0; i < names.size(); i++)
    {
        printf("%s\n", names[i].c_str());
    }
    printf("\n");
}

// Display the conversion options
static void PrintOptions()
{
    string resolvedPartNumber = ResolvePartNumber(PartNumber);

    printf("File Conversion Options\n");
    printf("=======================\n");
    printf("Input Hex File Path: \t%s\n", Input.c_str());
    printf("Output DFU File Path: \t%s\n", Output.c_str());
    printf("Part Number: \t\t%s\n", PartNumber.c_str());
    printf("Resolved Part Number: \t%s\n", resolvedPartNumber.c_str());
    printf("Target Family: \t\t%s\n", Family.c_str());
    printf("Application Name: \t%s\n", AppName.c_str());
    printf("Application Version: \t%X.%02X\n", VersionMajor, VersionMinor);
    printf("Locked: \t\t%s\n", Locked ? "Y" : "N");
    printf("\n");
}

// Return true if text contains the pattern string at index 0
static bool StartsWith(const string& text, const string& pattern)
{
    if (text.substr(0, pattern.length()) == pattern)
    {
        return true;
    }

    return false;
}

// Parse the arguments for an input path and check to see if the
// path is valid.
//
// Returns true if path argument is valid.
static bool ParseInputPath(const vector<string>& arguments)
{
    bool success = false;

    if (arguments.size() < 2)
    {
        return false;
    }

    Input = arguments[1];

    FILE* pFile = fopen(Input.c_str(), "rb");

    if (pFile)
    {
        success = true;
        fclose(pFile);
    }

    return success;
}

// Set the default output path to match the input path
// and replace the extension with .dfu
static void InitializeOutputPath(const string& inputPath)
{
    char path[_MAX_PATH];
    char drive[_MAX_DRIVE];
    char dir[_MAX_DIR];
    char fname[_MAX_FNAME];
    char ext[_MAX_EXT];

    _splitpath(inputPath.c_str(), drive, dir, fname, ext);
    _makepath(path, drive, dir, fname, ".dfu");

    Output = path;
}

// Make sure that all arguments are valid switches.
// Return false if there is an invalid switch.
static bool AreSwitchesValid(const vector<string>& arguments)
{
    // Ignore the first argument, which is the exe path
    for (size_t i = 1; i < arguments.size(); i++)
    {
        // Argument starts with a dash (-), meaning
        // that it's a switch
        if (StartsWith(arguments[i], "-"))
        {
            // List of valid switches
            if (!StartsWith(arguments[i], "-part") &&
                !StartsWith(arguments[i], "-family") &&
                !StartsWith(arguments[i], "-appname") &&
                !StartsWith(arguments[i], "-appversion") &&
                !StartsWith(arguments[i], "-locked"))
            {
                return false;
            }
        }
    }

    return true;
}

// Initialize the argument variables based on the switches
// specified
static bool ParseArguments(const vector<string>& arguments)
{
    for (size_t i = 2; i < arguments.size(); i++)
    {
        string argument = arguments[i];

        // Found the optional output file path
        if (i == 2 && !StartsWith(argument, "-"))
        {
            Output = argument;
        }
        // Parse the part string
        else if (StartsWith(argument, "-part("))
        {
            PartNumber = argument.substr(6, argument.length() - 7);
        }
        // Parse the target family string
        else if (StartsWith(argument, "-family("))
        {
            Family = argument.substr(8, argument.length() - 9);
        }
        // Parse the app name string
        else if (StartsWith(argument, "-appname("))
        {
            AppName = argument.substr(9, argument.length() - 10);
        }
        // Parse the app version
        else if (StartsWith(argument, "-appversion("))
        {
            string text = argument.substr(12, argument.length() - 13);
            size_t pos = text.find(".");

            // No decimal point
            if (pos == string::npos)
            {
                VersionMajor = (BYTE)strtoul(text.c_str(), NULL, 16);
                VersionMinor = 0x00;
            }
            else
            {
                string sMajor = text.substr(0, pos).c_str();
                string sMinor = text.substr(pos + 1).c_str();

                if (sMinor.length() == 1)
                    sMinor += "0";

                VersionMajor = (BYTE)strtoul(sMajor.c_str(), NULL, 16);
                VersionMinor = (BYTE)strtoul(sMinor.c_str(), NULL, 16);
            }
        }
        // Parse the locked flag
        else if (StartsWith(argument, "-locked"))
        {
            Locked = TRUE;
        }
        else
        {
            return false;
        }
    }

    return true;
}

static int Convert()
{
    int code = SUCCESS;

    FILE* pFile = fopen(Input.c_str(), "rb");

    if (pFile)
    {
        string  hexFileString   = FileReadString(pFile);
        BYTE*   dfuFile         = NULL;
        DWORD   dfuFileSize;

        fclose(pFile);

        // Convert the hex file string to a binary DFU file array
        DfuCode dfuCode = ConvertHexToDfu(hexFileString, &dfuFile, &dfuFileSize, PartNumber, Family, AppName, VersionMajor, VersionMinor, Locked);

        if (dfuCode == DfuOk)
        {
            // Open the .dfu file (binary write + create)
            pFile = fopen(Output.c_str(), "wb");

            if (pFile)
            {
                // Write the .dfu binary file
                size_t written = fwrite(dfuFile, sizeof(BYTE), dfuFileSize, pFile);

                fclose(pFile);

                if (written != dfuFileSize)
                {
                    code = ERROR_DFU_GENERATION_FAILED;
                }
            }
            else
            {
                code = ERROR_INVALID_DFU_PATH;
            }

            delete [] dfuFile;
        }
        else if (dfuCode == DfuInvalidPart)
        {
            code = ERROR_INVALID_ARG;
        }
        else if (dfuCode == DfuInvalidHexRecord)
        {
            code = ERROR_INVALID_HEX_RECORD;
        }
        else
        {
            code = ERROR_HEX_CONVERSION_FAILED;
        }
    }
    else
    {
        code = ERROR_INVALID_HEX_PATH;
    }

    return code;
}

/////////////////////////////////////////////////////////////////////////////
// Main
/////////////////////////////////////////////////////////////////////////////

int _tmain(int argc, _TCHAR* argv[])
{
    int code = SUCCESS;

    // Convert the arguments to a list of strings
    vector<string> arguments(argv, argv + argc);

    // The first argument is the path to the executable
    if (arguments.size() == 0)
    {
        PrintError(ERROR_INVALID_ARG);
        return ERROR_INVALID_ARG;
    }

    // If there are no arguments or the /? switch is found
    if (arguments.size() == 1 ||
        find(arguments.begin(), arguments.end(), "/?") != arguments.end())
    {
        PrintUsage();
        return SUCCESS;
    }

    if (find(arguments.begin(), arguments.end(), "-listparts") != arguments.end())
    {
        PrintPartNumbers();
        return SUCCESS;
    }

    // Parse the command line arguments for the input path
    if (!ParseInputPath(arguments))
    {
        PrintError(ERROR_INVALID_INPUT_PATH);
        return ERROR_INVALID_INPUT_PATH;
    }

    // Generate default output path
    InitializeOutputPath(Input);

    // Check for invalid switches
    if (!AreSwitchesValid(arguments))
    {
        PrintError(ERROR_INVALID_ARG);
        return ERROR_INVALID_ARG;
    }

    // Parse the command line arguments
    if (!ParseArguments(arguments))
    {
        PrintError(ERROR_INVALID_ARG);
        return ERROR_INVALID_ARG;
    }

    PrintOptions();

    printf("Converting...\n");

    code = Convert();

    PrintError(code);

	return code;
}
