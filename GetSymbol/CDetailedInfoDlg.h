#pragma once


// CDetailedInfoDlg dialog

class CDetailedInfoDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CDetailedInfoDlg)

public:
	CDetailedInfoDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CDetailedInfoDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DETAILED_INFO_DLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString m_strDetailedInfo;
};
