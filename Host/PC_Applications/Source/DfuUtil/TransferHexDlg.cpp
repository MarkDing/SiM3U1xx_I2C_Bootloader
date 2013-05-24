/////////////////////////////////////////////////////////////////////////////
// TransferHexDlg.cpp : implementation file
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TransferHexDlg.h"

/////////////////////////////////////////////////////////////////////////////
// Global Variables (Threading)
/////////////////////////////////////////////////////////////////////////////

struct GlobalThreadData
{
	BOOL	bContinue;
	int		percent;
};

static GlobalThreadData GlobalData;

/////////////////////////////////////////////////////////////////////////////
// Prototypes
/////////////////////////////////////////////////////////////////////////////

static DWORD WINAPI TransferThreadProc(LPVOID param);

/////////////////////////////////////////////////////////////////////////////
// CTransferHexDlg dialog
/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CTransferHexDlg, CDialog)

CTransferHexDlg::CTransferHexDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTransferHexDlg::IDD, pParent)
	, m_hThread(NULL)
{

}

CTransferHexDlg::~CTransferHexDlg()
{
}

void CTransferHexDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS1, m_progress);
}


BEGIN_MESSAGE_MAP(CTransferHexDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CTransferHexDlg::OnBnClickedOk)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDCANCEL, &CTransferHexDlg::OnBnClickedCancel)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTransferHexDlg message handlers
/////////////////////////////////////////////////////////////////////////////

BOOL CTransferHexDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

    if (m_param.download)
    {
        SetWindowText(_T("Downloading Image File..."));
    }
    else
    {
        SetWindowText(_T("Uploading Image File..."));
    }

	m_progress.SetRange(0, 1000);

	StartThread();
	StartTimer();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CTransferHexDlg::OnDestroy()
{
	CDialog::OnDestroy();
}

void CTransferHexDlg::OnBnClickedOk()
{
}

void CTransferHexDlg::OnBnClickedCancel()
{
	StopTimer();
	StopThread();

	CDialog::OnCancel();
}

void CTransferHexDlg::StartThread()
{
	DWORD threadID = 0;

	GlobalData.bContinue	= TRUE;
	GlobalData.percent		= 0;

	m_hThread = CreateThread(
		NULL,					// Default security
		0,						// Default stack size
		TransferThreadProc,		// Thread function
		&m_param,				// Thread function parameter
		0,						// Immediately run the thread
		&threadID);				// Thread ID
}

void CTransferHexDlg::StopThread()
{
	// Thread creation succeeded
	if (m_hThread != NULL)
	{
		DWORD timeout		= 15000;
		DWORD start			= GetTickCount();
		DWORD waitStatus;

#ifdef _DEBUG
		timeout = INFINITE;
#endif

		// Cancel the transfer process
		GlobalData.bContinue = FALSE;

		waitStatus = WaitForSingleObject(m_hThread, timeout);

		// Thread is busy or an error occurred
		if (waitStatus != WAIT_OBJECT_0)
		{
			TerminateThread(m_hThread, 0xDEAD);
            m_param.code = DFU_CODE_API_FATAL_ERROR;
		}

		CloseHandle(m_hThread);
		m_hThread = NULL;
	}
}

void CTransferHexDlg::StartTimer()
{
	SetTimer(0, 15, NULL);
}

void CTransferHexDlg::StopTimer()
{
	KillTimer(0);
}

void CTransferHexDlg::OnTimer(UINT_PTR nIDEvent)
{
	int			percent;
	BOOL		bContinue;
	CString		text;

	percent		= GlobalData.percent;
	bContinue	= GlobalData.bContinue;

	// Update the progress bar
	// (This is a cheesy workaround to get the stupid
	// Win7 progress ctrl to update faster)
	if (percent == 100)
	{
		m_progress.SetPos(1000);
		m_progress.SetPos(999);
		m_progress.SetPos(1000);
	}
	else
	{
		m_progress.SetPos(percent*10 + 1);
		m_progress.SetPos(percent*10);
	}

	// Update the progress bar caption
	text.Format(_T("%u%%"), percent);

	SetDlgItemText(IDC_STATIC_PERCENT, text);

	// The thread has finished on its own
	if (!bContinue)
	{
		// Close this dialog
		OnBnClickedCancel();
	}

	CDialog::OnTimer(nIDEvent);
}

BOOL CALLBACK ProgressCallback(int percent, BOOL workInProgress, int blockID, void* userData)
{
	// Check if the user wishes to abort the transfer
	BOOL bContinue;
	
	bContinue = GlobalData.bContinue;

	// Update the transfer percent
	GlobalData.percent = percent;

	return bContinue;
}

DWORD WINAPI TransferThreadProc(LPVOID param)
{
	if (param)
	{
		TransferHexParam* pParam = (TransferHexParam*)param;

        if (pParam->download)
        {
    		pParam->code	= DFU_DownloadImage(pParam->device, pParam->blockSize, pParam->imageFileName, ProgressCallback, pParam, &pParam->status);
		    pParam->desc	= _T("DFU_DownloadImage()");

            if (pParam->code == DFU_CODE_API_DNLOAD_PHASE_FAILED || 
                pParam->code == DFU_CODE_API_MANIFEST_PHASE_FAILED)
            {
                CString format;
                CString msg;

                format += _T("DFU_GETSTATUS\r\n");
                format += _T("bStatus: \t\t%s\r\n");
                format += _T("bwPollTimeout: \t%u\r\n");
                format += _T("bState: \t\t%s\r\n");
                format += _T("iString: \t\t%02X\r\n");
        
                msg.Format(format,
                    DecodeDfuStatus(pParam->status.bStatus),
                    pParam->status.bwPollTimeout,
                    DecodeDfuState(pParam->status.bState),
                    pParam->status.iString
                    );

                pParam->msg = msg;
            }
        }
        else
        {
            pParam->code	= DFU_UploadImage(pParam->device, pParam->blockSize, pParam->imageFileName, ProgressCallback, pParam);
		    pParam->desc	= _T("DFU_UploadHexImage()");
        }

		// Notify the GUI that the thread is finished
		GlobalData.bContinue = FALSE;

		return 0xCEED;
	}
	else
	{
		return 0xBAD;
	}
}
