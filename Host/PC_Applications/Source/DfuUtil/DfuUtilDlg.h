/////////////////////////////////////////////////////////////////////////////
// DfuUtilDlg.h : header file
/////////////////////////////////////////////////////////////////////////////

#pragma once

/////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////

#include "..\..\Release\DLL\SLAB_DFU.h"
#include <dbt.h>

/////////////////////////////////////////////////////////////////////////////
// CDfuUtilDlg dialog
/////////////////////////////////////////////////////////////////////////////

class CDfuUtilDlg : public CDialogEx
{
// Construction
public:
	CDfuUtilDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_DFUUTIL_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

    CComboBox   m_comboDevList;
    CFont       m_font;

    GUID        m_guid;
    DWORD       m_baud;
    DWORD       m_timeout;
    BOOL        m_precheck;
    BOOL        m_reset;

// Protected Methods
protected:
    void InitializeDialog();
    void InitLog();
	void UpdateDeviceList();
	CString GetSelectedDevicePath();
    DFU_CODE EnterIdleState();
    DFU_CODE GetInfoPage(BYTE* buffer, WORD size);
    DFU_CODE ValidateImage(CString filename, DFU_GET_INFO info);
    BOOL Connect();
    void Disconnect();
    BOOL Query();
    BOOL Download();
    BOOL Reset();
    BOOL Upload();

    void AppendLogText(CString text);
    void ClearLogText();

    void SaveSettings();
    void LoadSettings();
    void GetExeDirectory();

// Surprise Removal Protected Methods
protected:
    void RegisterDeviceChange(GUID guid);
	void UnregisterDeviceChange();

// Implementation
protected:
	HICON       m_hIcon;
    DFU_DEVICE  m_dev;
    HDEVNOTIFY	m_hNotifyDevNode;
    CString     m_exePath;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedCancel();
    afx_msg void OnClose();
    afx_msg void OnFileExit();
    afx_msg void OnToolsOptions();
    afx_msg void OnHelpAbout();
    afx_msg BOOL OnDeviceChange(UINT nEventType, DWORD_PTR dwData);
    afx_msg void OnCbnDropdownComboDeviceList();
    afx_msg void OnCbnCloseupComboDeviceList();
    afx_msg void OnDestroy();
    afx_msg void OnBnClickedButtonQuery();
    afx_msg void OnBnClickedButtonDownloadImage();
    afx_msg void OnBnClickedButtonUploadImage();
    afx_msg void OnBnClickedButtonClearLog();
    afx_msg void OnBnClickedButtonBrowseDownloadImage();
    afx_msg void OnBnClickedButtonBrowseUploadImage();
    afx_msg void OnBnClickedButtonReset();
};
