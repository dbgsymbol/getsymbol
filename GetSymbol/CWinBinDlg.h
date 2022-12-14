#pragma once
#include "json/json.h"

typedef struct _UPD_FILE_INFO {
	Json::String hash;
	std::vector<Json::String> os_versions;
	Json::String file_version;
	std::vector<Json::String> update_kbs;
	std::vector<Json::String> os_build_nums;
	std::vector<Json::String> release_dates;
	int timestamp;
	int virtual_size;
}UPD_FILE_INFO, * PUPD_FILE_INFO;

// CWinBinDlg dialog

class CWinBinDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CWinBinDlg)

public:
	CWinBinDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CWinBinDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_WIN_BIN_DLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	void ShowSelectedFileInfo(int itemSelected);
	void ShowUpdFileInfoList();
public:

	CListCtrl m_List;
	virtual BOOL OnInitDialog();
	CComboBox m_cmbFileName;

	TCHAR m_wszFileName[0x100];

	BOOL m_cmbDropDown = FALSE;
	std::vector<UPD_FILE_INFO> m_upd_file_info_list;
	
	CStatusBarCtrl	m_StatusBar;
	DWORD m_nThreadCount;

	afx_msg void OnCloseupCmbBinaryName();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnColumnclickList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDropdownCmbBinaryName();
	CListCtrl m_listProp;
	afx_msg void OnClickList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKeydownList(NMHDR* pNMHDR, LRESULT* pResult);

	BOOL m_bDownBin;
	BOOL m_bDownSym;
	afx_msg void OnBnClickedCheckPdb();
	afx_msg void OnBnClickedCheckBin();
	CComboBox m_cmbVersionFilter;
	afx_msg void OnCloseupCmbOsVerion();
	afx_msg void OnBnClickedBtnDown();
	afx_msg void OnChangeEditSearach();
	CString m_strSearch;
	afx_msg void OnBnClickedBtnExit();
};
