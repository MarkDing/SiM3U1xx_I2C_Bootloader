/////////////////////////////////////////////////////////////////////////////
// SettingsDlg.cpp : implementation file
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DfuUtil.h"
#include "SettingsDlg.h"
#include "afxdialogex.h"
#include "GuidUtils.h"

/////////////////////////////////////////////////////////////////////////////
// Static Global Constants
/////////////////////////////////////////////////////////////////////////////

static const GUID DEFAULT_GUID = { 0x04C77E93, 0xB54D, 0x4714, { 0xac, 0x96, 0x15, 0xa6, 0x2b, 0xfa, 0x30, 0x7b } };

/////////////////////////////////////////////////////////////////////////////
// CSettingsDlg dialog
/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CSettingsDlg, CDialogEx)

CSettingsDlg::CSettingsDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CSettingsDlg::IDD, pParent)
    , m_guid(_T(""))
    , m_baud(0)
    , m_timeout(0)
    , m_precheck(FALSE)
    , m_reset(FALSE)
{

}

CSettingsDlg::~CSettingsDlg()
{
}

void CSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT_GUID, m_guid);
    DDX_Text(pDX, IDC_EDIT_BAUD, m_baud);
    DDX_Text(pDX, IDC_EDIT_TRANSFER_TIMEOUT, m_timeout);
    DDX_Check(pDX, IDC_CHECK_VALIDATE_IMAGE, m_precheck);
    DDX_Check(pDX, IDC_CHECK_RESET, m_reset);
}

BEGIN_MESSAGE_MAP(CSettingsDlg, CDialogEx)
    ON_BN_CLICKED(IDOK, &CSettingsDlg::OnBnClickedOk)
    ON_BN_CLICKED(IDCANCEL, &CSettingsDlg::OnBnClickedCancel)
    ON_BN_CLICKED(IDC_BUTTON_DEFAULTS, &CSettingsDlg::OnBnClickedButtonDefaults)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSettingsDlg message handlers
/////////////////////////////////////////////////////////////////////////////

void CSettingsDlg::OnBnClickedOk()
{
    if (UpdateData(TRUE))
    {
        if (!IsValidGuid(m_guid))
        {
            MessageBox(_T("Invalid GUID"), (LPCTSTR)0, MB_ICONWARNING);
            return;
        }

        CDialogEx::OnOK();
    }
}

void CSettingsDlg::OnBnClickedCancel()
{
    // TODO: Add your control notification handler code here
    CDialogEx::OnCancel();
}

void CSettingsDlg::OnBnClickedButtonDefaults()
{
    m_guid = GuidToString(DEFAULT_GUID);
    m_baud = 115200;
    m_timeout = 1000;
    m_precheck = TRUE;
    m_reset = FALSE;
    UpdateData(FALSE);
}
