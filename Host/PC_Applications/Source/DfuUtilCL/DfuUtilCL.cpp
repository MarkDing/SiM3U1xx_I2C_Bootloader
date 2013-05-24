/////////////////////////////////////////////////////////////////////////////
// DfuUtilCL.cpp : Defines the entry point for the console application.
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\..\Release\DLL\SLAB_DFU.h"
#include "DfuFile.h"
#include "Crc.h"
#include "DFU_Utilities.h"
#include "StringUtils.h"
#include <vector>
#include <algorithm>

/////////////////////////////////////////////////////////////////////////////
// Linker Dependencies
/////////////////////////////////////////////////////////////////////////////

#ifdef _M_X64
#pragma comment (lib, "..\\..\\Release\\DLL\\x64\\SLAB_DFU.lib")
#else
#pragma comment (lib, "..\\..\\Release\\DLL\\Win32\\SLAB_DFU.lib")
#endif

/////////////////////////////////////////////////////////////////////////////
// Namespaces
/////////////////////////////////////////////////////////////////////////////

using namespace std;

/////////////////////////////////////////////////////////////////////////////
// Macros
/////////////////////////////////////////////////////////////////////////////

#define COUNT(a)                                    (sizeof(a) / sizeof(a[0]))

/////////////////////////////////////////////////////////////////////////////
// Definitions
/////////////////////////////////////////////////////////////////////////////

#define SUCCESS                                     0
#define ERROR_INVALID_IMAGE_PATH                    1
#define ERROR_INVALID_ARG                           2
#define ERROR_INVALID_INDEX_PATH_ARG                3
#define ERROR_INVALID_MODE_SELECTION                4
#define ERROR_NO_DEVICE_FOUND                       5
#define ERROR_INVALID_IMAGE                         6

/////////////////////////////////////////////////////////////////////////////
// Static Global Constants
/////////////////////////////////////////////////////////////////////////////

static const GUID DEFAULT_GUID = { 0x04C77E93, 0xB54D, 0x4714, { 0xac, 0x96, 0x15, 0xa6, 0x2b, 0xfa, 0x30, 0x7b } };

/////////////////////////////////////////////////////////////////////////////
// Global Variables (Command Line Options)
/////////////////////////////////////////////////////////////////////////////

// Communication options
static BOOL         Index               = FALSE;
static DWORD        DeviceIndex         = 0;
static BOOL         Path                = FALSE;
static string       DevicePath;
static GUID         Guid                = DEFAULT_GUID;
static DWORD        BaudRate            = 115200;
static DWORD        Timeout             = 1000;

// Modes
static BOOL         ListDevices         = FALSE;
static BOOL         Query               = FALSE;
static BOOL         Reset               = FALSE;
static BOOL         Download            = FALSE;
static BOOL         Upload              = FALSE;

// Download options
static BOOL         CheckImage          = FALSE;
static BOOL         ResetAfter          = FALSE;

// Upload/Download options
static BOOL         Image               = FALSE;
static string       ImagePath;

/////////////////////////////////////////////////////////////////////////////
// Global Variables
/////////////////////////////////////////////////////////////////////////////

static DFU_DEVICE   Device;
static BOOL         UserAbort           = FALSE;

static string Modes[] = {
    "listdevices",
    "query",
    "reset",
    "download",
    "upload"
};

static string Flags[] = {
    "checkimage",
    "resetafter"
};

static string Parameters[] = {
    "index",
    "path",
    "guid",
    "baud",
    "timeout",
    "image"
};

/////////////////////////////////////////////////////////////////////////////
// Static Global Functions
/////////////////////////////////////////////////////////////////////////////

// Print a string for the error code
// Lookup command line error codes first
// Lookup DFU_CODE error codes second
static void PrintError(int code)
{
    string text;

    // Decode command line error code
    if (code == SUCCESS ||
        code == ERROR_INVALID_IMAGE_PATH ||
        code == ERROR_INVALID_ARG ||
        code == ERROR_INVALID_INDEX_PATH_ARG || 
        code == ERROR_INVALID_MODE_SELECTION ||
        code == ERROR_NO_DEVICE_FOUND ||
        code == ERROR_INVALID_IMAGE)
    {
        switch (code)
        {
        case SUCCESS:
            text = "Success.\n";
            break;
        case ERROR_INVALID_IMAGE_PATH:
            text = "Invalid image path.\n";
            break;
        case ERROR_INVALID_ARG:
            text = "Invalid arguments.\n";
            break;
        case ERROR_INVALID_INDEX_PATH_ARG:
            text = "Can't specify both -index and -path.\n";
            break;
        case ERROR_INVALID_MODE_SELECTION:
            text = "Must specify -listdevices, -query, -reset, -upload, or -download.\n";
            break;
        case ERROR_NO_DEVICE_FOUND:
            text = "No device found.\n";
            break;
        case ERROR_INVALID_IMAGE:
            text = "Invalid image.\n";
            break;
        }
    }
    // Decode DFU error code
    else
    {
        text = DecodeDfuCode(code) + ".\n";
    }

    printf("%s", text.c_str());
}

// Display the command line argument help text to the console
static void PrintUsage()
{
    BYTE major;
    BYTE minor;
    BOOL release;
    string sLibVersion;

    DFU_GetLibraryVersion(&major, &minor, &release);

    sLibVersion = FormatString("%u.%u%s", major, minor, release ? "" : " (Debug)");

    printf("Silicon Laboratories DFU Utility 1.0\n");
    printf("SLAB_DFU.dll %s\n", sLibVersion.c_str());
    printf("====================================\n");
    printf("\n");

    printf("Description:\n");
    printf("Communicates with a device running the modular DFU bootloader\n");
    printf("to query, reset, upload, or download a DFU image.\n");
    printf("\n");

    printf("Usages:\n");
    printf("DfuUtilCL -listdevices\n");
    printf("DfuUtilCL -query\n");
    printf("DfuUtilCL -reset\n");
    printf("DfuUtilCL -upload    -image(imagepath)\n");
    printf("DfuUtilCL -download  -image(imagepath) [-checkimage] [-resetafter]]\n");
    printf("\n");

    printf("Communication Parameters:\n");
    printf("[-index(deviceindex) | -path(devicepath)]\n");
    printf("[-guid(guidstring)]\n");
    printf("[-baud(baudrate)]\n");
    printf("[-timeout(timeoutms)]\n");
    printf("\n");

    printf("Parameter Descriptions:\n");
    printf("-listdevices        Shows a list of devices, including device index and path\n");
    printf("-query              Indicates that the utility will return information about the\n");
    printf("                    current image on the DFU device\n");
    printf("-reset              Indicates that the utility will reset the bootloader\n");
    printf("-upload             Indicates that the utility will upload the DFU image from\n");
    printf("                    the DFU device to the specified DFU file\n");
    printf("-download           Indicates that the utility will download the specified DFU\n");
    printf("                    image to the DFU device\n");
    printf("-checkimage         Indicates that the utility will check to see if\n");
    printf("                    the application firmware is compatible with the DFU image\n");
    printf("                    before downloading\n");
    printf("-resetafter         Reset the device after successfully downloading a DFU image\n");
    printf("                    to the device\n");
    printf("-image              Indicates that a DFU image will be specified\n");
    printf("imagepath           Specifies the DFU image file name for -upload or -download\n");
    printf("-index              Indicates that a device index will be specified\n");
    printf("deviceindex         Specifies which DFU device to connect to by device index\n");
    printf("-path               Indicates that a device path will be specified\n");
    printf("devicepath          Specifies which DFU device to connect to by device path\n");
    printf("-guid               Indicates that a GUID for USB DFU device filtering will be\n");
    printf("                    specified\n");
    printf("guidstring          Specifies the GUID for DFU device filtering\n");
    printf("-baud               Indicates that a UART baud rate will be specified\n");
    printf("baudrate            Specifies the UART baud rate in bps\n");
    printf("-timeout            Indicates that a transfer timeout will be specified\n");
    printf("timeoutms           Specifies the transfer timeout in milliseconds\n");
    printf("\n");

    printf("Default Parameter Values:\n");
    printf("deviceindex         - 0\n");
    printf("guidstring          - 04C77E93-B54D-4714-AC96-15A62BFA307B\n");
    printf("baudrate            - 115200\n");
    printf("timeoutms           - 1000\n");
    printf("\n");

    printf("Remarks:\n");
    printf("1. Either the -index or the -path switch may be used, but not both.\n");
    printf("2. Exactly one of the -listdevices, -query, -reset, -upload, or -download\n");
    printf("   switches must be used.\n");
    printf("3. By default, this utility will automatically connect to the first DFU\n");
    printf("   device by device index (0). Use DfuUtilCL -listdevices to get a list\n");
    printf("   of devices. Next, use the -index or -path argument to specify which device\n");
    printf("   to connect to.\n");
    printf("\n");

    printf("Examples:\n");
    printf("DfuUtilCL -listdevices\n");
    printf("DfuUtilCL -query\n");
    printf("DfuUtilCL -reset\n");
    printf("DfuUtilCL -upload -image(upload.dfu) -path(\\\\.\\COM4) -baud(230400) -timeout(900)\n");
    printf("DfuUtilCL -download -image(download.dfu) -index(1)\n");    
    printf("\n");
}

static void PrintDeviceList()
{
    DFU_CODE    code;
    DWORD       numDevices;    
    
    printf("DFU Device List\n");
    printf("===============\n");

    code = DFU_GetNumDevices(&numDevices);

    if (code == DFU_CODE_SUCCESS)
    {
        char            path[MAX_PATH];
        DFU_INTERFACE   type = 0;

        for (DWORD i = 0; i < numDevices; i++)
        {
            code = DFU_GetDevicePath(i, path);

            DFU_GetInterfaceByIndex(i, &type);

            if (code == DFU_CODE_SUCCESS)
            {
                printf("%u. %s (%s)\n", i, path, type == DFU_INTERFACE_UART ? "UART" : "USB");
            }
        }

        if (numDevices == 0)
        {
            printf("No devices found.\n");
        }
    }

    printf("\n");
}

// Display the conversion options
static void PrintOptions()
{
    string sGuid;

    sGuid = FormatString(_T("%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x"),
		Guid.Data1,
		Guid.Data2,
		Guid.Data3,
		Guid.Data4[0],
		Guid.Data4[1],
		Guid.Data4[2],
		Guid.Data4[3],
		Guid.Data4[4],
		Guid.Data4[5],
		Guid.Data4[6],
		Guid.Data4[7]);

    printf("DFU Utility Options\n");
    printf("===================\n");

    if (Query)
        printf("Mode:                       Query\n");
    if (Reset)
        printf("Mode:                       Reset\n");
    if (Upload)
        printf("Mode:                       Upload\n");
    if (Download)
        printf("Mode:                       Download\n");

    if (Upload)
        printf("Output DFU File Path:       %s\n", ImagePath.c_str());
    if (Download)
        printf("Input DFU File Path:        %s\n", ImagePath.c_str());
    if (Index)
        printf("Device Index:               %u\n", DeviceIndex);
    if (Path)
        printf("Device Path:                %s\n", DevicePath.c_str());

    printf("GUID:                       %s\n", sGuid.c_str());
    printf("Baud Rate:                  %u\n", BaudRate);
    printf("Timeout (ms):               %u\n", Timeout);

    if (Download)
    {
        printf("Check Image:                %s\n", CheckImage ? "true" : "false");
        printf("Reset After:                %s\n", ResetAfter ? "true" : "false");
    }
    printf("\n");
}

static string BufferToString(BYTE* buffer, DWORD size)
{
    string msg;

    for (DWORD i = 0; i < size; i++)
    {
        msg += FormatString("%02X", buffer[i]);

        if (i < size - 1)
            msg += " ";
    }

    return msg;
}

static string NumberToString(DWORD number)
{
    string  text;
    char    temp[100];

    sprintf(temp, "%u", number);
    text = temp;

    for (int i = (int)text.length() - 3; i >= 1; i -= 3)
    {
        text.insert(i, ",");
    }

    return text;
}

static void PrintInfo(DFU_GET_INFO info)
{
    printf("Device Information\n");
    printf("==================\n");
    printf("\n");
    printf("Length:         %u bytes\n", info.length);
    printf("BL Revision:    %X.%02X\n", info.bBootloaderRevision_Major, info.bBootloaderRevision_Minor);
    printf("bmAttributes:   0x%08X\n", info.bmAttributes);
    printf("wBlockSize:     0x%08X (%s bytes)\n", info.wBlockSize, NumberToString(info.wBlockSize).c_str());
    printf("maxAppSize:     0x%08X (%s bytes)\n", info.maxAppSize, NumberToString(info.maxAppSize).c_str());
    printf("bDeviceID:      %s\n", BufferToString(info.bDeviceID, sizeof(info.bDeviceID)).c_str());
    printf("bUUID:          %s\n", BufferToString(info.bUUID, sizeof(info.bUUID)).c_str());
    printf("\n");
}

static void PrintInfoPage(BYTE* infoPage, DWORD size)
{
    DFU_File_Type* info = (DFU_File_Type*)infoPage;

    if (size < sizeof(DFU_File_Type))
        return;

    printf("Firmware Image Information\n");
    printf("==========================\n");
    printf("\n");

    if (info->flash.wSignature != DFU_FILE_SIGNATURE)
    {
        printf("Invalid signature; the following information may be invalid!\n\n");
    }

    // Make sure that the strings are null-terminated
    info->flash.sAppName[sizeof(info->flash.sAppName) - 1] = 0;
    info->flash.sTargetFamily[sizeof(info->flash.sTargetFamily) - 1] = 0;

    printf("DFU File Revision:      %X.%02X\n", info->flash.bDfuFileRevision_Major, info->flash.bDfuFileRevision_Minor);
    printf("App Revision:           %X.%02X\n", info->flash.bAppRevision_Major, info->flash.bAppRevision_Minor);
    printf("sAppName:               %s\n", info->flash.sAppName);
    printf("sTargetFamily:          %s\n", info->flash.sTargetFamily);
    printf("wAppSize:               0x%08X (%s bytes)\n", info->flash.wAppSize, NumberToString(info->flash.wAppSize).c_str());
    printf("wCrc:                   0x%08X\n", info->flash.wCrc);
    printf("wSignature:             0x%08X\n", info->flash.wSignature);
    printf("wLock:                  0x%08X\n", info->flash.wLock);
    printf("\n");
    printf("wAppStartAddress:       0x%08X\n", info->wAppStartAddress);
    printf("wBlockSize:             0x%08X (%s bytes)\n", info->wBlockSize, NumberToString(info->wBlockSize).c_str());
    printf("wFlashKey_A:            0x%08X\n", info->wFlashKey_A);
    printf("wFlashKey_B:            0x%08X\n", info->wFlashKey_B);
    printf("\n");
}

// Make sure that all arguments are valid switches.
// Return FALSE if there is an invalid switch.
static BOOL AreSwitchesValid(const vector<string>& arguments)
{
    // Ignore the first argument, which is the exe path
    for (size_t i = 1; i < arguments.size(); i++)
    {
        string argument = arguments[i];

        // Argument starts with a dash (-), meaning
        // that it's a switch
        if (argument.substr(0, 1) == "-")
        {
            BOOL match = FALSE;

            // Mode flags must match the form: -mode
            for (int j = 0; j < COUNT(Modes); j++)
            {
                string mode = Modes[j];

                if (argument.substr(1, mode.length()) == mode &&
                    argument.length() == mode.length() + 1)
                {
                    match = TRUE;
                    break;
                }
            }

            // Flags with parameters must match the form: -flag(parameter)
            for (int j = 0; j < COUNT(Parameters); j++)
            {
                string parameter = Parameters[j];

                if (argument.substr(1, parameter.length() + 1) == (parameter + "(") &&
                    argument[argument.length() - 1] == ')')
                {
                    match = TRUE;
                    break;
                }
            }
            
            // Flags must match the form: -flag
            for (int j = 0; j < COUNT(Flags); j++)
            {
                string flag = Flags[j];

                if (argument.substr(1, flag.length()) == flag &&
                    argument.length() == flag.length() + 1)
                {
                    match = TRUE;
                    break;
                }
            }

            // Flag is invalid
            if (!match)
                return FALSE;
        }
        else
        {
            return FALSE;
        }
    }

    return TRUE;
}

// Initialize the argument variables based on the switches
// specified
static BOOL ParseArguments(const vector<string>& arguments)
{
    // Ignore the first argument, which is the exe path
    for (size_t i = 1; i < arguments.size(); i++)
    {
        string argument = arguments[i];

        // Remove the leading dash (-)
        argument = argument.substr(1, argument.length() - 1);

        //////////////////////////////
        // Flags without parameters //
        //////////////////////////////
        if (argument == "listdevices")
        {
            ListDevices = TRUE;
        }
        else if (argument == "query")
        {
            Query = TRUE;
        }
        else if (argument == "reset")
        {
            Reset = TRUE;
        }
        else if (argument == "download")
        {
            Download = TRUE;
        }
        else if (argument == "upload")
        {
            Upload = TRUE;
        }
        else if (argument == "checkimage")
        {
            CheckImage = TRUE;
        }
        else if (argument == "resetafter")
        {
            ResetAfter = TRUE;
        }
        ///////////////////////////
        // Flags with parameters //
        ///////////////////////////
        else
        {
            size_t lparen = argument.find('(');
            size_t rparen = argument.find(')');

            if (lparen == string::npos ||
                rparen == string::npos ||
                lparen > rparen)
            {
                return FALSE;
            }

            // Extract the parameter between the parenthesis
            string parameter = argument.substr(lparen + 1, rparen - lparen - 1);
            
            // Remove the parenthesis and parameter from the argument string
            argument = argument.substr(0, lparen);

            if (argument == "index")
            {
                Index = TRUE;
                DeviceIndex = (DWORD)strtoul(parameter.c_str(), NULL, 10);
            }
            else if (argument == "path")
            {
                Path = TRUE;
                DevicePath = parameter;
            }
            else if (argument == "guid")
            {
                if (parameter.length() != 36)
                    return FALSE;

                Guid.Data1      = (DWORD)strtoul(parameter.c_str(), NULL, 16);
                Guid.Data2      = (WORD)strtoul(parameter.c_str(), NULL, 16);
                Guid.Data3      = (WORD)strtoul(parameter.c_str(), NULL, 16);
                Guid.Data4[0]   = (BYTE)strtoul(parameter.c_str(), NULL, 16);
                Guid.Data4[1]   = (BYTE)strtoul(parameter.c_str(), NULL, 16);
                Guid.Data4[2]   = (BYTE)strtoul(parameter.c_str(), NULL, 16);
                Guid.Data4[3]   = (BYTE)strtoul(parameter.c_str(), NULL, 16);
                Guid.Data4[4]   = (BYTE)strtoul(parameter.c_str(), NULL, 16);
                Guid.Data4[5]   = (BYTE)strtoul(parameter.c_str(), NULL, 16);
                Guid.Data4[6]   = (BYTE)strtoul(parameter.c_str(), NULL, 16);
                Guid.Data4[7]   = (BYTE)strtoul(parameter.c_str(), NULL, 16);
            }
            else if (argument == "baud")
            {
                BaudRate = (DWORD)strtoul(parameter.c_str(), NULL, 10);
            }
            else if (argument == "timeout")
            {
                Timeout = (DWORD)strtoul(parameter.c_str(), NULL, 10);
            }
            else if (argument == "image")
            {
                Image = TRUE;
                ImagePath = parameter;
            }
            else
            {
                return FALSE;
            }
        }
    }

    return TRUE;
}

static BOOL CALLBACK Callback(int percent, BOOL workInProgress, int blockID, void* userData)
{
    printf("%d %%\r", percent);

    if (!workInProgress && percent == 100)
    {
        printf("\n");
    }

    return !UserAbort;
}

static int Connect()
{
    int             code;
    DWORD           numDevices  = 0;
    DFU_INTERFACE   type;
    char            path[MAX_PATH];

    DFU_SetGuid(&Guid);
    DFU_GetNumDevices(&numDevices);

    if (numDevices > 0)
    {
        if (Path)
        {
            code = DFU_OpenByDevicePath(&Device, DevicePath.c_str());
        }
        else
        {
            code = DFU_OpenByIndex(&Device, DeviceIndex);
        }

        if (DFU_GetOpenedInterface(Device, &type) == DFU_CODE_SUCCESS && type == DFU_INTERFACE_UART)
        {
            DFU_SetUartConfig(Device, BaudRate, DFU_EIGHT_DATA_BITS, DFU_NO_PARITY, DFU_ONE_STOP_BIT, DFU_NO_FLOW_CONTROL);
        }

        DFU_SetTransferTimeout(Device, Timeout);
    }
    else
    {
        code = ERROR_NO_DEVICE_FOUND;
    }

    if (code == DFU_CODE_SUCCESS)
    {
        if (DFU_GetOpenedDevicePath(Device, path) == DFU_CODE_SUCCESS)
        {
            printf("Connected to %s...\n", path);
        }
    }

    return code;
}

static int MakeIdleState()
{
    int         code;
    DFU_STATE   state;

    code = DFU_DfuGetState(Device, &state);

    if (code == DFU_CODE_SUCCESS)
    {
        if (state == DFU_STATE_DFU_ERROR)
        {
            code = DFU_DfuClearStatus(Device);
        }
        else if (state != DFU_STATE_DFU_IDLE)
        {
            code = DFU_DfuAbort(Device);
        }
    }

    return code;
}

static int GetInfoPage(BYTE* buffer, WORD size)
{
    int     code;
    WORD    transferred;

    code = DFU_DfuUpload(Device, buffer, size, 0, &transferred);

    if (code == DFU_CODE_SUCCESS)
    {
        code = DFU_DfuAbort(Device);
    }

    return code;
}

static int UploadImage()
{
    int code;

    code = Connect();

    if (code == DFU_CODE_SUCCESS)
    {
        code = MakeIdleState();

        if (code == DFU_CODE_SUCCESS)
        {
            DFU_GET_INFO info;

            printf("Getting block size...\n");
            code = DFU_SlabGetInfo(Device, &info);

            if (code == DFU_CODE_SUCCESS)
            {
                printf("Uploading...\n");
                code = DFU_UploadImage(Device, (WORD)info.wBlockSize, ImagePath.c_str(), Callback, NULL);
            }
        }

        DFU_Close(Device);
    }

    return code;
}

// Read the entire contents of a binary DFU file into a vector
BOOL LoadFile(const char* dfuFileName, BYTE** buffer, DWORD* size)
{
    BOOL success = FALSE;

    FILE* pFile;
    
    // Open the DFU file (read + binary, must exist)
    if (fopen_s(&pFile, dfuFileName, "rb") == 0)
    {
        long    fileSize    = 0;
        size_t  bytesRead   = 0;

        // Get the size of the file
        fseek(pFile, 0, SEEK_END);
        fileSize = ftell(pFile);
        rewind(pFile);

        *buffer = new BYTE[fileSize];

        // Read the entire contents of the binary file
        size_t read = fread(*buffer, sizeof(BYTE), fileSize, pFile);

        // Make sure that the entire image is read
        if (read == fileSize)
        {
            *size = fileSize;

            success = TRUE;
        }
        else
        {
            delete [] *buffer;
        }

        fclose(pFile);
    }

    return success;
}

static DWORD CrcImage(BYTE* image, DWORD size)
{
    DWORD crc = 0xFFFFFFFF;

    DFU_File_Type* file = (DFU_File_Type*)image;

    // Start a block 1
    // Iterate through app size number of bytes
    for (DWORD i = file->wBlockSize; i < min(size, file->wBlockSize + file->flash.wAppSize); i++)
    {
        _crc(crc, image[i]);
    }

    crc ^= 0xFFFFFFFF;

    return crc;
}

static int ValidateImage(DFU_GET_INFO info)
{
    int code;

    if (!CheckImage)
    {
        return DFU_CODE_SUCCESS;
    }

    printf("Validating Image...\n");

    BYTE* device = new BYTE[info.wBlockSize];

    code = GetInfoPage(device, (WORD)info.wBlockSize);

    if (code == DFU_CODE_SUCCESS)
    {
        BYTE*   image     = NULL;
        DWORD   imageSize = 0;

        if (LoadFile(ImagePath.c_str(), &image, &imageSize))
        {
            DFU_File_Type* deviceInfoPage   = (DFU_File_Type*)device;
            DFU_File_Type* imageInfoPage    = (DFU_File_Type*)image;

            DWORD expectedImageNumBlocks     = (imageInfoPage->wBlockSize + imageInfoPage->flash.wAppSize + imageInfoPage->wBlockSize - 1) / imageInfoPage->wBlockSize;
            DWORD expectedImageSize          = expectedImageNumBlocks * imageInfoPage->wBlockSize;

            if (imageSize != expectedImageSize)
            {
                printf("Invalid Image File Size!\n");
                code = ERROR_INVALID_IMAGE;
            }
            if (imageInfoPage->flash.wAppSize > info.maxAppSize)
            {
                printf("Invalid Image Size!\n");
                code = ERROR_INVALID_IMAGE;
            }
            else if ((imageInfoPage->flash.wSignature & DFU_FILE_SIGNATURE_MASK) != (DFU_FILE_SIGNATURE & DFU_FILE_SIGNATURE_MASK))
            {
                printf("Invalid Image Signature!\n");
                code = ERROR_INVALID_IMAGE;
            }
            else if (imageInfoPage->wAppStartAddress != deviceInfoPage->wAppStartAddress)
            {
                printf("Invalid App Start Address!\n");
                code = ERROR_INVALID_IMAGE;
            }
            else if (imageInfoPage->wBlockSize != info.wBlockSize ||
                     imageInfoPage->wBlockSize != deviceInfoPage->wBlockSize)
            {
                printf("Invalid Block Size!\n");
                code = ERROR_INVALID_IMAGE;
            }
            else
            {
                DWORD imageCrc = CrcImage(image, imageSize);

                if (imageCrc != imageInfoPage->flash.wCrc)
                {
                    printf("Invalid Image CRC!\n");
                    code = ERROR_INVALID_IMAGE;
                }
            }

            delete [] image;
        }
        else
        {
            code = ERROR_INVALID_IMAGE;
        }
    }

    delete [] device;

    return code;
}

static int DownloadImage()
{
    int code;

    code = Connect();

    if (code == DFU_CODE_SUCCESS)
    {
        code = MakeIdleState();

        if (code == DFU_CODE_SUCCESS)
        {
            DFU_GET_INFO info;

            printf("Getting block size...\n");
            code = DFU_SlabGetInfo(Device, &info);

            if (code == DFU_CODE_SUCCESS)
            {
                code = ValidateImage(info);

                if (code == DFU_CODE_SUCCESS)
                {
                    DFU_GET_STATUS status;

                    printf("Downloading...\n");
                    code = DFU_DownloadImage(Device, (WORD)info.wBlockSize, ImagePath.c_str(), Callback, NULL, &status);

                    if (ResetAfter)
                    {
                        if (code == DFU_CODE_SUCCESS)
                        {
                            printf("Resetting...\n");
                            code = DFU_SlabReset(Device);
                        }
                    }
                }
            }
        }

        DFU_Close(Device);
    }

    return code;
}

// Return information about the image on the device
static int QueryImage()
{
    int code;

    code = Connect();

    if (code == DFU_CODE_SUCCESS)
    {
        code = MakeIdleState();

        if (code == DFU_CODE_SUCCESS)
        {
            DFU_GET_INFO info;

            printf("Getting Device Information...\n\n");
            code = DFU_SlabGetInfo(Device, &info);

            if (code == DFU_CODE_SUCCESS)
            {
                PrintInfo(info);

                BYTE* buffer = new BYTE[info.wBlockSize];

                printf("Getting Firmware Image Information...\n\n");
                code = GetInfoPage(buffer, (WORD)info.wBlockSize);
                
                if (code == DFU_CODE_SUCCESS)
                {
                    PrintInfoPage(buffer, info.wBlockSize);
                }

                delete [] buffer;
            }
        }

        DFU_Close(Device);
    }

    return code;
}

// Resets the device
static int ResetDevice()
{
    int code;

    code = Connect();

    if (code == DFU_CODE_SUCCESS)
    {
        code = DFU_SlabReset(Device);
        DFU_Close(Device);
    }

    return code;
}

/////////////////////////////////////////////////////////////////////////////
// Global Functions
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Main
/////////////////////////////////////////////////////////////////////////////

BOOL CtrlHandler(DWORD fdwCtrlType)
{
    // Abort a long download/upload process by aborting in the callback
    if (fdwCtrlType == CTRL_C_EVENT)
    {
        UserAbort = TRUE;
        return TRUE;
    }

    return FALSE;
}

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
    int code = SUCCESS;

    SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, TRUE);

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

    // Can't specify both -index and -path
    if (Index && Path)
    {
        PrintError(ERROR_INVALID_INDEX_PATH_ARG);
        return ERROR_INVALID_INDEX_PATH_ARG;
    }

    // Must specify -listdevices, -query, -reset, -download, or -upload
    if ((ListDevices + Query + Reset + Download + Upload) != 1)
    {
        PrintError(ERROR_INVALID_MODE_SELECTION);
        return ERROR_INVALID_MODE_SELECTION;
    }

    PrintOptions();

    if (ListDevices)
    {
        PrintDeviceList();
        return SUCCESS;
    }

    if (Query)
    {
        code = QueryImage();
    }
    else if (Reset)
    {
        code = ResetDevice();
    }
    else if (Upload)
    {
        if (Image)
        {
            code = UploadImage();
        }
        else
        {
            PrintError(ERROR_INVALID_IMAGE_PATH);
            return ERROR_INVALID_IMAGE_PATH;
        }
    }
    else if (Download)
    {
        if (Image)
        {
            code = DownloadImage();
        }
        else
        {
            PrintError(ERROR_INVALID_IMAGE_PATH);
            return ERROR_INVALID_IMAGE_PATH;
        }
    }

    PrintError(code);
    return code;
}
