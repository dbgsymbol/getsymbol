#pragma once

#include "CGetSymbolTab.h"
// CMainDlg dialog

class CMainDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CMainDlg)

public:
	CMainDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CMainDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MAIN_DLG };
#endif

protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	
	DECLARE_MESSAGE_MAP()

public:
	BOOL	m_bExitPending;
	CGetSymbolTab m_Tab;

	void Close();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
};
