#pragma once


// CUpdateDlg dialog

class CUpdateDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CUpdateDlg)

public:
	CUpdateDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CUpdateDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_UPDATE_DLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString m_strNewVersion;
};
