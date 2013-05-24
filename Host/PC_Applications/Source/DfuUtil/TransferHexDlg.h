#pragma once

/////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////

#include "resource.h"
#include "DFU_Utilities.h"

/////////////////////////////////////////////////////////////////////////////
// Structures
/////////////////////////////////////////////////////////////////////////////

struct TransferHexParam
{
	// Inputs
    BOOL            download;   // download or upload
	DFU_DEVICE	    device;
    WORD            blockSize;
	char		    imageFileName[MAX_PATH];

	// Outputs
	DFU_CODE        code;
	CString		    desc;
    CString         msg;
    DFU_GET_STATUS  status;
};

/////////////////////////////////////////////////////////////////////////////
// CTransferHexDlg Class
/////////////////////////////////////////////////////////////////////////////

class CTransferHexDlg : public CDialog
{
	DECLARE_DYNAMIC(CTransferHexDlg)

public:
	CTransferHexDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CTransferHexDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_TRANSFER_HEX_FILE };

	CProgressCtrl m_progress;

public:
	TransferHexParam	m_param;
	HANDLE				m_hThread;

protected:
	void StartThread();
	void StopThread();
	void StartTimer();
	void StopTimer();

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedOk();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedCancel();
};
