#pragma once
#include "json/json.h"


typedef struct _UPD_FILE_INFO {
	CString file_name;
	Json::String hash;
	std::vector<Json::String> os_versions;
	Json::String file_version;
	std::vector<Json::String> update_kbs;
	std::vector<Json::String> os_build_nums;
	std::vector<Json::String> release_dates;
	int timestamp;
	int virtual_size;
	bool operator==(const _UPD_FILE_INFO& rh) {
		return hash == rh.hash;
	}

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
	void OnOperationDownload(BOOL bOnlyBin);
public:

	CListCtrl m_List;
	virtual BOOL OnInitDialog();
	CComboBox m_cmbFileName;

	TCHAR m_wszFileName[0x100];

	BOOL m_cmbDropDown = FALSE;
	Json::Value m_infoRoot;
	std::vector<UPD_FILE_INFO> m_upd_file_info_list;
	
	CStatusBarCtrl	m_StatusBar;

	DWORD m_nThreadCount;

	afx_msg void OnCloseupCmbBinaryName();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnColumnclickList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDropdownCmbBinaryName();
	CListCtrl m_listDownload;

	std::vector<UPD_FILE_INFO> m_upd_download_list;

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

	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRclickList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnOperationViewinfo();
	afx_msg void OnDblclkList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnOperationAddto();

	afx_msg void OnOperationDownloadbinary();
	afx_msg void OnDownloadBinarysymbol();
	afx_msg void OnRclickListDownload(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDlRemove();
	afx_msg void OnDlClearall();
	afx_msg void OnDlOpenfilelocation();
};
