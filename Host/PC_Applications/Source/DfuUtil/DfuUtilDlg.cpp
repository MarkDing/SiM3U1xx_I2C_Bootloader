/////////////////////////////////////////////////////////////////////////////
// DfuUtilDlg.cpp : implementation file
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DfuUtil.h"
#include "DfuUtilDlg.h"
#include "afxdialogex.h"
#include "SettingsDlg.h"
#include "GuidUtils.h"
#include "DFU_Utilities.h"
#include "StdioFilePlus.h"
#include "DfuFile.h"
#include "TransferHexDlg.h"
#include "Crc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// Linker Dependencies
/////////////////////////////////////////////////////////////////////////////

#ifdef _M_X64
#pragma comment (lib, "..\\..\\Release\\DLL\\x64\\SLAB_DFU.lib")
#else
#pragma comment (lib, "..\\..\\Release\\DLL\\Win32\\SLAB_DFU.lib")
#endif

/////////////////////////////////////////////////////////////////////////////
// Static Functions
/////////////////////////////////////////////////////////////////////////////

static CString FormatHex(BYTE buffer[], DWORD length)
{
    CString text;
    CString temp;

    for (DWORD i = 0; i < length; i++)
    {
        temp.Format(_T("%02X"), buffer[i]);

        if (i < length - 1)
        {
            temp += _T(" ");
        }

        text += temp;
    }

    return text;
}

static CString NumberToString(DWORD number)
{
    CString text;

    text.Format(_T("%u"), number);

    for (int i = text.GetLength() - 3; i >= 1; i -= 3)
    {
        text.Insert(i, _T(","));
    }

    return text;
}

static CString GetFnameExt(CString filename)
{
    TCHAR path[MAX_PATH];
    TCHAR drive[_MAX_DRIVE];
    TCHAR dir[_MAX_DIR];
    TCHAR fname[_MAX_FNAME];
    TCHAR ext[_MAX_EXT];

    _tsplitpath_s(filename, drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, ext, _MAX_EXT);
    _tmakepath_s(path, MAX_PATH, NULL, NULL, fname, ext);

    return path;
}

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About
/////////////////////////////////////////////////////////////////////////////

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
    virtual BOOL OnInitDialog();
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BOOL CAboutDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	BYTE		major;
	BYTE		minor;
	BOOL		release;
	DFU_CODE    code;
	CString		text;

	code = DFU_GetLibraryVersion(&major, &minor, &release);

	if (code == DFU_CODE_SUCCESS)
	{
		text.Format(_T("%d.%d%s"), major, minor, (release) ? _T("") : _T(" (Debug)"));
		SetDlgItemText(IDC_STATIC_LIB_VERSION, text);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDfuUtilDlg dialog
/////////////////////////////////////////////////////////////////////////////

CDfuUtilDlg::CDfuUtilDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDfuUtilDlg::IDD, pParent)
    , m_baud(115200)
    , m_timeout(1000)
    , m_precheck(TRUE)
    , m_reset(FALSE)
    , m_hNotifyDevNode(NULL)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDfuUtilDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

    DDX_Control(pDX, IDC_COMBO_DEVICE_LIST, m_comboDevList);
}

BEGIN_MESSAGE_MAP(CDfuUtilDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDOK, &CDfuUtilDlg::OnBnClickedOk)
    ON_BN_CLICKED(IDCANCEL, &CDfuUtilDlg::OnBnClickedCancel)
    ON_WM_CLOSE()
    ON_COMMAND(ID_FILE_EXIT, &CDfuUtilDlg::OnFileExit)
    ON_COMMAND(ID_TOOLS_OPTIONS, &CDfuUtilDlg::OnToolsOptions)
    ON_COMMAND(ID_HELP_ABOUT, &CDfuUtilDlg::OnHelpAbout)
    ON_WM_DEVICECHANGE()
    ON_WM_DESTROY()
    ON_CBN_DROPDOWN(IDC_COMBO_DEVICE_LIST, &CDfuUtilDlg::OnCbnDropdownComboDeviceList)
    ON_CBN_CLOSEUP(IDC_COMBO_DEVICE_LIST, &CDfuUtilDlg::OnCbnCloseupComboDeviceList)
    ON_BN_CLICKED(IDC_BUTTON_QUERY, &CDfuUtilDlg::OnBnClickedButtonQuery)
    ON_BN_CLICKED(IDC_BUTTON_DOWNLOAD_IMAGE, &CDfuUtilDlg::OnBnClickedButtonDownloadImage)
    ON_BN_CLICKED(IDC_BUTTON_UPLOAD_IMAGE, &CDfuUtilDlg::OnBnClickedButtonUploadImage)
    ON_BN_CLICKED(IDC_BUTTON_CLEAR_LOG, &CDfuUtilDlg::OnBnClickedButtonClearLog)
    ON_BN_CLICKED(IDC_BUTTON_BROWSE_DOWNLOAD_IMAGE, &CDfuUtilDlg::OnBnClickedButtonBrowseDownloadImage)
    ON_BN_CLICKED(IDC_BUTTON_BROWSE_UPLOAD_IMAGE, &CDfuUtilDlg::OnBnClickedButtonBrowseUploadImage)
    ON_BN_CLICKED(IDC_BUTTON_RESET, &CDfuUtilDlg::OnBnClickedButtonReset)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDfuUtilDlg message handlers
/////////////////////////////////////////////////////////////////////////////

BOOL CDfuUtilDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	InitializeDialog();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CDfuUtilDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CDfuUtilDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CDfuUtilDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CDfuUtilDlg::OnBnClickedOk()
{
}

void CDfuUtilDlg::OnBnClickedCancel()
{
}

void CDfuUtilDlg::OnClose()
{
    CDialogEx::OnOK();
}

void CDfuUtilDlg::OnDestroy()
{
    SaveSettings();

    CDialogEx::OnDestroy();

	// Stop receiving WM_DEVICECHANGE messages
	UnregisterDeviceChange();
}

void CDfuUtilDlg::OnFileExit()
{
    OnClose();
}

void CDfuUtilDlg::OnToolsOptions()
{
    CSettingsDlg dlg;

    dlg.m_guid      = GuidToString(m_guid);
    dlg.m_baud      = m_baud;
    dlg.m_timeout   = m_timeout;
    dlg.m_precheck  = m_precheck;
    dlg.m_reset     = m_reset;

    if (dlg.DoModal() == IDOK)
    {
        m_guid      = StringToGuid(dlg.m_guid);
        m_baud      = dlg.m_baud;
        m_timeout   = dlg.m_timeout;
        m_precheck  = dlg.m_precheck;
        m_reset     = dlg.m_reset;

        // GUID changed, re-register for device change
        // notification
        if (DFU_SetGuid(&m_guid) == DFU_CODE_SUCCESS)
        {
            UnregisterDeviceChange();
            RegisterDeviceChange(m_guid);
        }

        UpdateDeviceList();
    }
}

void CDfuUtilDlg::OnHelpAbout()
{
    CAboutDlg dlg;
    dlg.DoModal();
}

void CDfuUtilDlg::OnBnClickedButtonReset()
{
    AppendLogText(_T("==============================================Reset================================================\r\n"));
    AppendLogText(_T("Resetting device...\r\n"));

    if (Connect())
    {
        Reset();
    }

    Disconnect();

    AppendLogText(_T("===================================================================================================\r\n"));
    AppendLogText(_T("\r\n"));
}

void CDfuUtilDlg::OnBnClickedButtonQuery()
{
    CString path = GetSelectedDevicePath();

    if (path.IsEmpty())
        return;

    AppendLogText(_T("==============================================Query================================================\r\n"));
    AppendLogText(_T("Querying device for device and firmware image information...\r\n"));

    if (Connect())
    {
        Query();
    }

    Disconnect();

    AppendLogText(_T("===================================================================================================\r\n"));
    AppendLogText(_T("\r\n"));
}

void CDfuUtilDlg::OnBnClickedButtonBrowseDownloadImage()
{
    CString filter = _T("DFU Binary Files (*.dfu)|*.dfu|All Files (*.*)|*.*||");
    CFileDialog dlg(TRUE, 0, 0, OFN_HIDEREADONLY, filter);

    if (dlg.DoModal() == IDOK)
    {
        SetDlgItemText(IDC_EDIT_DOWNLOAD_IMAGE_PATH, dlg.GetPathName());
    }
}

void CDfuUtilDlg::OnBnClickedButtonDownloadImage()
{
    CString path;
    GetDlgItemText(IDC_EDIT_DOWNLOAD_IMAGE_PATH, path);

    if (path.IsEmpty())
    {
        MessageBox(_T("Select a valid image file"), (LPCTSTR)0, MB_ICONWARNING);
        return;
    }

    AppendLogText(_T("============================================Download===============================================\r\n"));
    AppendLogText(_T("Downloading ") + GetFnameExt(path) + _T(" to device...\r\n"));

    if (Connect())
    {
        if (Download())
        {
            if (m_reset)
            {
                Reset();
            }
        }
    }

    Disconnect();

    AppendLogText(_T("===================================================================================================\r\n"));
    AppendLogText(_T("\r\n"));
}

void CDfuUtilDlg::OnBnClickedButtonBrowseUploadImage()
{
    CString filter = _T("DFU Binary Files (*.dfu)|*.dfu|All Files (*.*)|*.*||");
    CFileDialog dlg(FALSE, _T("dfu"), _T("image"), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, filter);

    if (dlg.DoModal() == IDOK)
    {
        SetDlgItemText(IDC_EDIT_UPLOAD_IMAGE_PATH, dlg.GetPathName());
    }
}

void CDfuUtilDlg::OnBnClickedButtonUploadImage()
{
    CString path;
    GetDlgItemText(IDC_EDIT_UPLOAD_IMAGE_PATH, path);

    if (path.IsEmpty())
    {
        MessageBox(_T("Select a valid image file"), (LPCTSTR)0, MB_ICONWARNING);
        return;
    }

    AppendLogText(_T("=============================================Upload================================================\r\n"));
    AppendLogText(_T("Uploading ") + GetFnameExt(path) + _T(" from device...\r\n"));

    if (Connect())
    {
        Upload();
    }

    Disconnect();

    AppendLogText(_T("===================================================================================================\r\n"));
    AppendLogText(_T("\r\n"));
}

void CDfuUtilDlg::OnBnClickedButtonClearLog()
{
    ClearLogText();
}

/////////////////////////////////////////////////////////////////////////////
// CDfuUtilDlg Class - Protected Methods
/////////////////////////////////////////////////////////////////////////////

void CDfuUtilDlg::InitializeDialog()
{
    InitLog();

    DFU_GetGuid(&m_guid);

    GetExeDirectory();

    LoadSettings();

    DFU_SetGuid(&m_guid);

    // Register to receive WM_DEVICECHANGE messages
	RegisterDeviceChange(m_guid);

	UpdateDeviceList();
}

void CDfuUtilDlg::InitLog()
{
    CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_LOG);

    // Set the default log limit to a much larger value
    pEdit->SetLimitText(1000000);

    CFont*      pFont = GetFont();
    LOGFONT     lf;

	// Get the logfont for the current CEdit font
	if (pFont->GetLogFont(&lf))
	{
		// Change the face name to use the new face name
		_tcscpy_s(lf.lfFaceName, 32, _T("Courier New"));

        // Change the font size
        lf.lfHeight = -12;

		// Create a new font using the old log font and
		// new face name
		if (m_font.CreateFontIndirect(&lf))
		{
			// Use the new face name
			pEdit->SetFont(&m_font, TRUE);
		}
	}
}

void CDfuUtilDlg::UpdateDeviceList()
{
	int			sel			= m_comboDevList.GetCurSel();
	CString		selText;
	DWORD		numDevices	= 0;

	if (sel != CB_ERR)
	{
		m_comboDevList.GetLBText(sel, selText);
	}

	m_comboDevList.ResetContent();

	if (DFU_GetNumDevices(&numDevices) == DFU_CODE_SUCCESS)
	{
		for (DWORD i = 0; i < numDevices; i++)
		{
			DFU_CODE		code;
			DFU_DEVICE_STR	path;
			CString			sPath;

            code = DFU_GetDevicePath(i, path);

			if (code == DFU_CODE_SUCCESS)
			{
				sPath = path;
			}
			else
			{
				sPath = DecodeDfuCode(code);
			}

			m_comboDevList.AddString(sPath);
		}
	}

	sel = 0;

	for (int i = 0; i < m_comboDevList.GetCount(); i++)
	{
		CString item;
		
		m_comboDevList.GetLBText(i, item);

		if (item == selText)
		{
			sel = i;
			break;
		}
	}

	m_comboDevList.SetCurSel(sel);
}

CString CDfuUtilDlg::GetSelectedDevicePath()
{
	CString		path;
	CString		selText;
	int			sel			= m_comboDevList.GetCurSel();
	int			pos			= 0;

	if (sel != CB_ERR)
	{
		m_comboDevList.GetLBText(sel, selText);

		path = selText;
	}

	return path;
}

DFU_CODE CDfuUtilDlg::EnterIdleState()
{
    DFU_CODE    code;
    DFU_STATE   state;

    code = DFU_DfuGetState(m_dev, &state);

    if (code == DFU_CODE_SUCCESS)
    {
        if (state == DFU_STATE_DFU_ERROR)
        {
            code = DFU_DfuClearStatus(m_dev);
        }
        else if (state != DFU_STATE_DFU_IDLE)
        {
            code = DFU_DfuAbort(m_dev);
        }
    }

    return code;
}

DFU_CODE CDfuUtilDlg::GetInfoPage(BYTE* buffer, WORD size)
{
    DFU_CODE    code;
    WORD        transferred;

    code = DFU_DfuUpload(m_dev, buffer, size, 0, &transferred);

    if (code == DFU_CODE_SUCCESS)
    {
        code = DFU_DfuAbort(m_dev);
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

DWORD CrcImage(BYTE* image, DWORD size)
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

DFU_CODE CDfuUtilDlg::ValidateImage(CString filename, DFU_GET_INFO info)
{
    DFU_CODE code;

    if (!m_precheck)
    {
        return DFU_CODE_SUCCESS;
    }

    BYTE* device = new BYTE[info.wBlockSize];

    code = GetInfoPage(device, (WORD)info.wBlockSize);

    if (code == DFU_CODE_SUCCESS)
    {
        BYTE*   image     = NULL;
        DWORD   imageSize = 0;

        if (LoadFile(CStringA(filename), &image, &imageSize))
        {
            DFU_File_Type* deviceInfoPage   = (DFU_File_Type*)device;
            DFU_File_Type* imageInfoPage    = (DFU_File_Type*)image;

            DWORD expectedImageNumBlocks     = (imageInfoPage->wBlockSize + imageInfoPage->flash.wAppSize + imageInfoPage->wBlockSize - 1) / imageInfoPage->wBlockSize;
            DWORD expectedImageSize          = expectedImageNumBlocks * imageInfoPage->wBlockSize;

            if (imageSize != expectedImageSize)
            {
                AppendLogText(_T("*** Invalid Image File Size!\r\n"));
                code = DFU_CODE_API_INVALID_IMAGE_FILE;
            }
            else if (imageInfoPage->flash.wAppSize > info.maxAppSize)
            {
                AppendLogText(_T("*** Invalid Image Size!\r\n"));
                code = DFU_CODE_API_INVALID_IMAGE_FILE;
            }
            else if ((imageInfoPage->flash.wSignature & DFU_FILE_SIGNATURE_MASK) != (DFU_FILE_SIGNATURE & DFU_FILE_SIGNATURE_MASK))
            {
                AppendLogText(_T("*** Invalid Image Signature!\r\n"));
                code = DFU_CODE_API_INVALID_IMAGE_FILE;
            }
            else if (imageInfoPage->wAppStartAddress != deviceInfoPage->wAppStartAddress)
            {
                AppendLogText(_T("*** Invalid App Start Address!\r\n"));
                code = DFU_CODE_API_INVALID_IMAGE_FILE;
            }
            else if (imageInfoPage->wBlockSize != info.wBlockSize ||
                     imageInfoPage->wBlockSize != deviceInfoPage->wBlockSize)
            {
                AppendLogText(_T("*** Invalid Block Size!\r\n"));
                code = DFU_CODE_API_INVALID_IMAGE_FILE;
            }
            else
            {
                DWORD imageCrc = CrcImage(image, imageSize);

                if (imageCrc != imageInfoPage->flash.wCrc)
                {
                    AppendLogText(_T("*** Invalid Image CRC!\r\n"));
                    code = DFU_CODE_API_INVALID_IMAGE_FILE;
                }
                else
                {
                    AppendLogText(_T("- Image is valid\r\n"));
                }
            }

            delete [] image;
        }
        else
        {
            code = DFU_CODE_API_INVALID_IMAGE_FILE;
        }
    }
    else
    {
        AppendLogText(_T("*** Failed to query device: ") + DecodeDfuCode(code) + _T("\r\n"));
    }

    delete [] device;

    return code;
}

BOOL CDfuUtilDlg::Connect()
{
    BOOL success = FALSE;

    CString path = GetSelectedDevicePath();
    DWORD numDevices;

    DFU_GetNumDevices(&numDevices);

    if (!path.IsEmpty())
    {
        DFU_CODE code = DFU_OpenByDevicePath(&m_dev, CStringA(path));

        if (code != DFU_CODE_SUCCESS)
        {
            AppendLogText(_T("*** Failed to connect to ") + path + _T(": ") + DecodeDfuCode(code) + _T("\r\n"));
        }
        else
        {
            DFU_INTERFACE type;

            AppendLogText(_T("- Connected to ") + path + _T("\r\n"));

            DFU_GetOpenedInterface(m_dev, &type);

            if (type == DFU_INTERFACE_UART)
            {
                code = DFU_SetUartConfig(m_dev, m_baud, DFU_EIGHT_DATA_BITS, DFU_NO_PARITY, DFU_ONE_STOP_BIT, DFU_NO_FLOW_CONTROL);

                if (code == DFU_CODE_SUCCESS)
                {
                    CString msg;
                    msg.Format(_T("- Set UART Config to %u 8-N-1\r\n"), m_baud);
                    AppendLogText(msg);
                }
                else
                {
                    AppendLogText(_T("*** Failed to set UART config: ") + DecodeDfuCode(code) + _T("\r\n"));
                }
            }

            if (code == DFU_CODE_SUCCESS)
            {
                code = DFU_SetTransferTimeout(m_dev, m_timeout);

                if (code == DFU_CODE_SUCCESS)
                {
                    CString msg;
                    msg.Format(_T("- Set timeouts to %u ms\r\n"), m_timeout);
                    AppendLogText(msg);
                }
                else
                {
                    AppendLogText(_T("*** Failed to set timeouts: ") + DecodeDfuCode(code) + _T("\r\n"));
                }
            }

            if (code == DFU_CODE_SUCCESS)
            {
                code = EnterIdleState();

                if (code == DFU_CODE_SUCCESS)
                {
                    AppendLogText(_T("- Entered DFU idle state\r\n"));
                }
                else
                {
                    AppendLogText(_T("*** Failed to enter DFU idle state: ") + DecodeDfuCode(code) + _T("\r\n"));
                }
            }

            if (code == DFU_CODE_SUCCESS)
            {
                success = TRUE;
            }
        }
    }

    return success;
}

CString InfoToString(DFU_GET_INFO info)
{
    CString format;
    CString msg;
        
    format += _T("\r\n");
    format += _T("- Device Information:\r\n");
    format += _T("    Length:                       %u bytes\r\n");
    format += _T("    BL Revision:                  %X.%02X\r\n");
    format += _T("    bmAttributes:                 0x%08X\r\n");
    format += _T("    wBlockSize:                   0x%08X (%s bytes)\r\n");
    format += _T("    maxAppSize:                   0x%08X (%s bytes)\r\n");
    format += _T("    bDeviceID:                    %s\r\n");
    format += _T("    bUUID:                        %s\r\n");
    format += _T("\r\n");
        
    msg.Format(format,
        info.length,
        info.bBootloaderRevision_Major,
        info.bBootloaderRevision_Minor,
        info.bmAttributes,
        info.wBlockSize, NumberToString(info.wBlockSize),
        info.maxAppSize, NumberToString(info.maxAppSize),
        FormatHex(info.bDeviceID, sizeof(info.bDeviceID)),
        FormatHex(info.bUUID, sizeof(info.bUUID))
        );

    return msg;
}

CString InfoBlockToString(DFU_File_Type dfuFile)
{
    CString format;
    CString msg;

    format = _T("- Firmware Image Information:\r\n");

    if (dfuFile.flash.wSignature != DFU_FILE_SIGNATURE)
    {
        format += _T("*** Invalid signature; the following information may be invalid!\r\n");
    }

    format += _T("    DFU File Revision:            %X.%02X\r\n");
    format += _T("    App Revision:                 %X.%02X\r\n");
    format += _T("    sAppName:                     %s\r\n");
    format += _T("    sTargetFamily:                %s\r\n");
    format += _T("    wAppSize:                     0x%08X (%s bytes)\r\n");
    format += _T("    wCrc:                         0x%08X\r\n");
    format += _T("    wSignature:                   0x%08X\r\n");
    format += _T("    wLock:                        0x%08X\r\n");
    format += _T("    wAppStartAddress:             0x%08X\r\n");
    format += _T("    wBlockSize:                   0x%08X (%s bytes)\r\n");
    format += _T("    wFlashKey_A:                  0x%08X\r\n");
    format += _T("    wFlashKey_B:                  0x%08X\r\n");
    format += _T("\r\n");

    // Make sure that the strings are null-terminated
    dfuFile.flash.sAppName[sizeof(dfuFile.flash.sAppName) - 1] = 0;
    dfuFile.flash.sTargetFamily[sizeof(dfuFile.flash.sTargetFamily) - 1] = 0;

    msg.Format(format,
        dfuFile.flash.bDfuFileRevision_Major, dfuFile.flash.bDfuFileRevision_Minor,
        dfuFile.flash.bAppRevision_Major, dfuFile.flash.bAppRevision_Minor,
        CString(dfuFile.flash.sAppName),
        CString(dfuFile.flash.sTargetFamily),
        dfuFile.flash.wAppSize, NumberToString(dfuFile.flash.wAppSize),
        dfuFile.flash.wCrc,
        dfuFile.flash.wSignature,
        dfuFile.flash.wLock,
        dfuFile.wAppStartAddress,
        dfuFile.wBlockSize, NumberToString(dfuFile.wBlockSize),
        dfuFile.wFlashKey_A,
        dfuFile.wFlashKey_B
        );

    return msg;
}

BOOL CDfuUtilDlg::Query()
{
    BOOL success = FALSE;
    DFU_GET_INFO info;
    DFU_CODE code = DFU_SlabGetInfo(m_dev, &info);

    if (code == DFU_CODE_SUCCESS)
    {
        AppendLogText(InfoToString(info));

        BYTE* infoBlock = new BYTE[info.wBlockSize];

        code = GetInfoPage(infoBlock, (WORD)info.wBlockSize);

        if (code == DFU_CODE_SUCCESS)
        {
            AppendLogText(InfoBlockToString(*((DFU_File_Type*)infoBlock)));
        }
        else
        {
            AppendLogText(_T("*** Failed to upload block 0: ") + DecodeDfuCode(code) + _T("\r\n"));
        }

        delete [] infoBlock;

        success = TRUE;
    }
    else
    {
        AppendLogText(_T("*** Failed to query device: ") + DecodeDfuCode(code) + _T("\r\n"));
    }

    return success;
}

BOOL CDfuUtilDlg::Download()
{
    CString path;
    GetDlgItemText(IDC_EDIT_DOWNLOAD_IMAGE_PATH, path);

    BOOL            success = FALSE;
    DFU_GET_INFO    info;
    DFU_CODE        code    = DFU_SlabGetInfo(m_dev, &info);

    if (code == DFU_CODE_SUCCESS)
    {
        CString msg;
        msg.Format(_T("- Queried block size: %u\r\n"), info.wBlockSize);
        AppendLogText(msg);

        code = ValidateImage(path, info);

        if (code == DFU_CODE_SUCCESS)
        {
            CTransferHexDlg dlg;

            dlg.m_param.download    = TRUE;
            dlg.m_param.device      = m_dev;
            dlg.m_param.blockSize   = (WORD)info.wBlockSize;
            strcpy_s(dlg.m_param.imageFileName, MAX_PATH, CStringA(path));

            dlg.DoModal();

            if (dlg.m_param.code == DFU_CODE_SUCCESS)
            {
                AppendLogText(_T("- Download successful\r\n"));

                // Retrieve new CRC
                BYTE* infoBlock = new BYTE[info.wBlockSize];
                code = GetInfoPage(infoBlock, (WORD)info.wBlockSize);

                if (code == DFU_CODE_SUCCESS)
                {
                    DFU_File_Type* dfuFile = (DFU_File_Type*)infoBlock;

                    if (dfuFile->flash.wSignature != DFU_FILE_SIGNATURE)
                    {
                        msg.Format(_T("*** Invalid file signature: 0x%08\r\n"), dfuFile->flash.wSignature);
                        AppendLogText(msg);
                    }
                    else
                    {
                        msg.Format(_T("- CRC: 0x%08X\r\n"), dfuFile->flash.wCrc);
                        AppendLogText(msg);
                    }
                }

                delete [] infoBlock;

                success = TRUE;
            }
            else
            {
                AppendLogText(_T("*** Failed to download image: ") + DecodeDfuCode(dlg.m_param.code) + _T("\r\n"));
            
                if (!dlg.m_param.msg.IsEmpty())
                {
                    AppendLogText(dlg.m_param.msg + _T("\r\n"));
                }
            }
        }
        else
        {
            AppendLogText(_T("*** Failed to validate image!\r\n"));
        }
    }
    else
    {
        AppendLogText(_T("*** Failed to query block size: ") + DecodeDfuCode(code) + _T("\r\n"));
    }

    return success;
}

BOOL CDfuUtilDlg::Reset()
{
    BOOL success = FALSE;

    DFU_CODE code = DFU_SlabReset(m_dev);

    if (code == DFU_CODE_SUCCESS)
    {
        AppendLogText(_T("- Reset successful\r\n"));
    }
    else
    {
        AppendLogText(_T("*** Failed to reset the device: ") + DecodeDfuCode(code) + _T("\r\n"));
    }

    return success;
}

BOOL CDfuUtilDlg::Upload()
{
    CString path;
    GetDlgItemText(IDC_EDIT_UPLOAD_IMAGE_PATH, path);

    BOOL            success = FALSE;
    DFU_GET_INFO    info;
    DFU_CODE        code    = DFU_SlabGetInfo(m_dev, &info);

    if (code == DFU_CODE_SUCCESS)
    {
        CString msg;
        msg.Format(_T("- Queried block size: %u\r\n"), info.wBlockSize);
        AppendLogText(msg);

        CTransferHexDlg dlg;

        dlg.m_param.download    = FALSE;
        dlg.m_param.device      = m_dev;
        dlg.m_param.blockSize   = (WORD)info.wBlockSize;
        strcpy_s(dlg.m_param.imageFileName, MAX_PATH, CStringA(path));

        dlg.DoModal();

        if (dlg.m_param.code == DFU_CODE_SUCCESS)
        {
            AppendLogText(_T("- Upload successful\r\n"));

            success = TRUE;
        }
        else
        {
            AppendLogText(_T("*** Failed to upload image: ") + DecodeDfuCode(dlg.m_param.code) + _T("\r\n"));
            
            if (!dlg.m_param.msg.IsEmpty())
            {
                AppendLogText(dlg.m_param.msg + _T("\r\n"));
            }
        }
    }
    else
    {
        AppendLogText(_T("*** Failed to query block size: ") + DecodeDfuCode(code) + _T("\r\n"));
    }

    return success;
}

void CDfuUtilDlg::Disconnect()
{
    DFU_DEVICE_STR path;

    if (DFU_GetOpenedDevicePath(m_dev, path) == DFU_CODE_SUCCESS)
    {
        DFU_Close(m_dev);
        AppendLogText(_T("- Disconnected from ") + CString(path) + _T("\r\n"));
    }
}

void CDfuUtilDlg::AppendLogText(CString text)
{
    CEdit* pEdit = ((CEdit*)GetDlgItem(IDC_EDIT_LOG));

    int len = pEdit->GetWindowTextLength();
    pEdit->SetSel(len, len);
    pEdit->ReplaceSel(text);
}

void CDfuUtilDlg::ClearLogText()
{
    SetDlgItemText(IDC_EDIT_LOG, _T(""));
}

void CDfuUtilDlg::SaveSettings()
{
    CString filename = m_exePath + _T("options.txt");

    CStdioFilePlus file;

    if (file.Open(filename, CFile::modeWrite | CFile::modeCreate))
    {
        CString text;
        GetDlgItemText(IDC_EDIT_DOWNLOAD_IMAGE_PATH, text);
        file.WriteCString(text);
        GetDlgItemText(IDC_EDIT_UPLOAD_IMAGE_PATH, text);
        file.WriteCString(text);

        file.WriteCString(GuidToString(m_guid));
        file.WriteDword(m_baud);
        file.WriteDword(m_timeout);
        file.WriteBool(m_precheck);
        file.WriteBool(m_reset);

        file.Close();
    }
}

void CDfuUtilDlg::LoadSettings()
{
    CString filename = m_exePath + _T("options.txt");

    CStdioFilePlus file;

    if (file.Open(filename, CFile::modeRead))
    {
        CString text;
        file.ReadCString(text);
        SetDlgItemText(IDC_EDIT_DOWNLOAD_IMAGE_PATH, text);
        file.ReadCString(text);
        SetDlgItemText(IDC_EDIT_UPLOAD_IMAGE_PATH, text);

        CString sGuid;
        file.ReadCString(sGuid);
        m_guid = StringToGuid(sGuid);

        file.ReadDword(m_baud);
        file.ReadDword(m_timeout);
        file.ReadBool(m_precheck);
        file.ReadBool(m_reset);

        file.Close();
    }
}

void CDfuUtilDlg::GetExeDirectory()
{
	TCHAR path[_MAX_PATH];
	TCHAR drive[_MAX_DRIVE];
	TCHAR dir[_MAX_DIR];
	TCHAR fname[_MAX_FNAME];
	TCHAR ext[_MAX_EXT];

	// Get the path to the exe
	GetModuleFileName(NULL, path, _MAX_PATH);

	// Split the path into component strings
	_tsplitpath_s(path, drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, ext, _MAX_EXT);

	// Create a new path to the exe directory
	_tmakepath_s(path, _MAX_PATH, drive, dir, NULL, NULL);

	m_exePath = path;
}

/////////////////////////////////////////////////////////////////////////////
// CDfuUtilDlg - Surprise Removal Protected Methods
/////////////////////////////////////////////////////////////////////////////

// Register for device change notification for USB HID devices
// OnDeviceChange() will handle device arrival and removal
void CDfuUtilDlg::RegisterDeviceChange(GUID guid)
{
	DEV_BROADCAST_DEVICEINTERFACE devIF = {0};

	devIF.dbcc_size			= sizeof(devIF);    
	devIF.dbcc_devicetype	= DBT_DEVTYP_DEVICEINTERFACE;
    devIF.dbcc_classguid    = guid;
	
	m_hNotifyDevNode = RegisterDeviceNotification(GetSafeHwnd(), &devIF, DEVICE_NOTIFY_WINDOW_HANDLE);
}

// Unregister for device change notification for USB HID devices
void CDfuUtilDlg::UnregisterDeviceChange()
{
	if (m_hNotifyDevNode)
	{
		UnregisterDeviceNotification(m_hNotifyDevNode);
		m_hNotifyDevNode = NULL;
	}
}

// Handle device change messages (ie a device is added or removed)
// - If a device is connected, then add the device to the device list
// - If the device we were connected to was removed, then disconnect from the device
BOOL CDfuUtilDlg::OnDeviceChange(UINT nEventType, DWORD_PTR dwData)
{
	// Device has been removed
	if (nEventType == DBT_DEVICEREMOVECOMPLETE ||
		nEventType == DBT_DEVICEARRIVAL)
	{
		if (dwData)
		{
			PDEV_BROADCAST_HDR pHdr = (PDEV_BROADCAST_HDR)dwData;

			if (pHdr->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE)
			{
				PDEV_BROADCAST_DEVICEINTERFACE pDevInf = (PDEV_BROADCAST_DEVICEINTERFACE)pHdr;
				BOOL opened			= FALSE;
				BOOL disconnected	= FALSE;

				// "\\?\hid#vid_10c4&pid_81ba..."
				CString deviceStr = pDevInf->dbcc_name;

				// Check to see if the device is opened
                opened = DFU_IsOpened(m_dev);

				if (nEventType == DBT_DEVICEREMOVECOMPLETE)
				{
					DFU_DEVICE_STR devPath;

					// If a device is currently open
					if (opened)
					{
						// Get our device's path
						if (DFU_GetOpenedDevicePath(m_dev, devPath) == DFU_CODE_SUCCESS)
						{
							// Check to see if our device was removed by comparing device paths
							if (deviceStr.CompareNoCase(CString(devPath)) == 0)
							{
								// Disconnect from it
                                DFU_Close(m_dev);

								disconnected = TRUE;
							}
						}
					}
				}

				// Only update the device list if we aren't connected to a device
				if (disconnected || !opened)
				{
					UpdateDeviceList();
				}
			}
		}
	}

	return TRUE;
}

void CDfuUtilDlg::OnCbnDropdownComboDeviceList()
{
    UpdateDeviceList();
}

void CDfuUtilDlg::OnCbnCloseupComboDeviceList()
{
    UpdateDeviceList();
}
