
// GetSymbolDlg.h : header file
//

#pragma once
#include "afxcmn.h"

#include "HttpRoutine.h"
// CGetSymbolDlg dialog
class CGetSymbolDlg : public CDialogEx
{
public:
	BOOL	m_bBusy;
	BOOL	m_bPaused;

	DWORD	m_SuccCount;
	DWORD	m_FailCount;
// Construction
public:
	CGetSymbolDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_GETSYMBOL_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnExit();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	HANDLE m_hMainThread;
	UINT m_ThreadCount;
	DWORD m_nThreadRunningCount;
	CString m_SrcPath;
	CString m_DestPath;
	afx_msg void OnBnClickedBtnSrc();
	afx_msg void OnBnClickedBtnDest();
	CListCtrl m_List;
	CStatusBarCtrl	m_StatusBar;
	CMenu m_Menu;
	afx_msg void OnBnClickedBtnStart();
	afx_msg void OnColumnclickList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedRadioMs();
	afx_msg void OnBnClickedRadioGoogle();
	afx_msg void OnBnClickedRadioMozilla();
	afx_msg void OnBnClickedRadioCitrix();
};
