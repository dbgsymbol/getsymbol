// CWinBinDlg.cpp : implementation file
//
  
#include "stdafx.h"
#include "GetSymbol.h"
#include "CWinBinDlg.h"
#include "afxdialogex.h"

#include "CDetailedInfoDlg.h"

#include <fstream>
#include <string.h>
#include <UserEnv.h>

#include "GZipComp.h"
#include "HttpRoutine.h"

#pragma comment(lib, "userenv.lib")

enum UPD_LIST_COLUMNS {
	COL_NUM,
	COL_OS_VERSION,
	COL_OS_BUILD_NUM,
	COL_UPDATE,
	COL_FILE_VERSION,
	COL_HASH,
};

enum DOWN_LIST_COLUMNS {
	COL_DOWN_NUM,
	COL_DOWN_FILENAME,
	COL_DOWN_BUILD_NUM,
	COL_DOWN_HASH,
	COL_DOWN_STATUS,
};


#define UPD_COL_COUNT		(COL_UPDATE + 1)
// CWinBinDlg dialog

IMPLEMENT_DYNAMIC(CWinBinDlg, CDialogEx)

CWinBinDlg::CWinBinDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_WIN_BIN_DLG, pParent)
	, m_bDownBin(TRUE)
	, m_bDownSym(TRUE)
	, m_strSearch(_T(""))
{
	m_nThreadCount = 0;
	srand(GetTickCount64());
}

CWinBinDlg::~CWinBinDlg()
{
}

void CWinBinDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_List);
	DDX_Control(pDX, IDC_CMB_BINARY_NAME, m_cmbFileName);
	DDX_Control(pDX, IDC_LIST_DOWNLOAD, m_listDownload);
	DDX_Check(pDX, IDC_CHECK_BIN, m_bDownBin);
	DDX_Check(pDX, IDC_CHECK_PDB, m_bDownSym);
	DDX_Control(pDX, IDC_CMB_OS_VERSION, m_cmbVersionFilter);
	DDX_Text(pDX, IDC_EDIT_SEARACH, m_strSearch);
	DDV_MaxChars(pDX, m_strSearch, 200);
}


BEGIN_MESSAGE_MAP(CWinBinDlg, CDialogEx)
ON_CBN_CLOSEUP(IDC_CMB_BINARY_NAME, &CWinBinDlg::OnCloseupCmbBinaryName)
ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST, &CWinBinDlg::OnColumnclickList)
ON_CBN_DROPDOWN(IDC_CMB_BINARY_NAME, &CWinBinDlg::OnDropdownCmbBinaryName)
ON_BN_CLICKED(IDC_CHECK_PDB, &CWinBinDlg::OnBnClickedCheckPdb)
ON_BN_CLICKED(IDC_CHECK_BIN, &CWinBinDlg::OnBnClickedCheckBin)
ON_CBN_CLOSEUP(IDC_CMB_OS_VERSION, &CWinBinDlg::OnCloseupCmbOsVerion)
ON_BN_CLICKED(IDC_BTN_DOWN, &CWinBinDlg::OnBnClickedBtnDown)
ON_EN_CHANGE(IDC_EDIT_SEARACH, &CWinBinDlg::OnChangeEditSearach)
ON_BN_CLICKED(IDC_BTN_EXIT, &CWinBinDlg::OnBnClickedBtnExit)
ON_WM_RBUTTONDOWN()
ON_NOTIFY(NM_RCLICK, IDC_LIST, &CWinBinDlg::OnRclickList)
ON_COMMAND(ID_OPERATION_VIEWINFO, &CWinBinDlg::OnOperationViewinfo)
ON_NOTIFY(NM_DBLCLK, IDC_LIST, &CWinBinDlg::OnDblclkList)
ON_COMMAND(ID_OPERATION_ADDTO, &CWinBinDlg::OnOperationAddto)
ON_COMMAND(ID_DOWNLOAD_BINARY, &CWinBinDlg::OnOperationDownloadbinary)
ON_COMMAND(ID_DOWNLOAD_BINARYSYMBOL, &CWinBinDlg::OnDownloadBinarysymbol)
ON_NOTIFY(NM_RCLICK, IDC_LIST_DOWNLOAD, &CWinBinDlg::OnRclickListDownload)
ON_COMMAND(ID_DL_REMOVE, &CWinBinDlg::OnDlRemove)
ON_COMMAND(ID_DL_CLEARALL, &CWinBinDlg::OnDlClearall)
ON_COMMAND(ID_DL_OPENFILELOCATION, &CWinBinDlg::OnDlOpenfilelocation)
END_MESSAGE_MAP()

// CWinBinDlg message handlers
std::vector<Json::String> GetFileNames(TCHAR* szFileNameJsonURL) {

	ULONG status;
	TCHAR* lpStringBuffer = NULL;
	ULONG dwReaded = 0;

	status = BeginDownload(szFileNameJsonURL, NULL, &lpStringBuffer, &dwReaded);

	if (status != ERROR_SUCCESS || !dwReaded) {
		pWinBinDlg->m_StatusBar.SetText(_T("Internet connection error"), 3, 0);
		return {};
	}

	Json::Reader reader;
	Json::Value root;

	std::string str((char*)lpStringBuffer);

	if (!reader.parse(str, root)) {
		LocalFree(lpStringBuffer);
		return {};
	}

	std::vector<Json::String> items;
	for (unsigned int i = 0; i < root.size(); i++) {
		items.push_back(root[i].asString());
	}
	
	LocalFree(lpStringBuffer);

	return items;
}

void adjust_file_version(Json::String *file_version) {
	char* ptr = (char*)strchr(file_version->c_str(), ' ');
	if (ptr) {
		*ptr = 0;
	}
}

std::vector<Json::String> get_os_build_nums(Json::String heading) {
	size_t os_builds_start = heading.find('(');
	size_t os_builds_end = heading.find(')');

	if (os_builds_start == -1 || os_builds_end == -1) {
		return {};
	}

	Json::String builds = heading.substr(os_builds_start, os_builds_end - os_builds_start + 1);
	
	std::vector<Json::String> os_builds;

	Json::String build;
	size_t index_dot = 0;
	size_t start = 0;
	size_t end = 0;

	bool _break = false;
	while (1) {
		index_dot = builds.find('.', end);
		start = builds.rfind(' ', index_dot);
		end = builds.find(',', index_dot);
		if (end == -1) {
			end = builds.find(')', index_dot);
			if (end != -1) {
				_break = true;
			}
		}
		if (end == -1) {
			break;
		}

		build = builds.substr(start + 1, end - start - 1);
		if (!strstr(build.c_str(), "and")) {
			build = Json::String("10.0.") + build;
			os_builds.push_back(build);
		}
		else {
			Json::String build_2 = build;
			end = build_2.find(' ');
			build = build_2.substr(0, end);
			build = Json::String("10.0.") + build;
			os_builds.push_back(build);

			build = build_2.substr(end + 4, build_2.find('\x00') - (end + 4));
			build = Json::String("10.0.") + build;
			os_builds.push_back(build);
		}

		if (_break) {
			break;
		}
	}
	
	return os_builds;
	
}

void GetUpdFileInfo(Json::Value data, UPD_FILE_INFO* upd_file_info) {
	Json::String item;
	for each (Json::String version in data["windowsVersions"].getMemberNames()) {
		if (!strchr(version.c_str(), '-')) {
			item = Json::String("Windows 10-") + version;
		}
		else {
			item = Json::String("Windows ") + version;
		}

		upd_file_info->os_versions.push_back(item);
	}

	for each (Json::String win_ver in  data["windowsVersions"].getMemberNames()) {

		Json::Value updates = data["windowsVersions"][win_ver];
		std::vector<Json::String> update_names = updates.getMemberNames();
		for each (Json::String name in updates.getMemberNames()) {
			

			if (!strcmp(name.c_str(), "BASE")) {
				upd_file_info->update_kbs.push_back(name + " - " + win_ver);
				upd_file_info->release_dates.push_back(updates[name]["windowsVersionInfo"]["releaseDate"].asString());
				upd_file_info->os_build_nums.push_back(upd_file_info->file_version);
				continue;
			}

			upd_file_info->update_kbs.push_back(name);

			Json::Value update_info = updates[name]["updateInfo"];

			upd_file_info->release_dates.push_back(update_info["releaseDate"].asString());

			std::vector<Json::String> build_nums = get_os_build_nums(update_info["heading"].asString());
			for each (Json::String build_num in build_nums) {
				if (std::find(upd_file_info->os_build_nums.begin(), upd_file_info->os_build_nums.end(), build_num) == upd_file_info->os_build_nums.end()) {
					upd_file_info->os_build_nums.push_back(build_num);
				}
			}

			Json::Value otherWindowsVersions = update_info["otherWindowsVersions"];
			Json::ValueType type = otherWindowsVersions.type();
			if (type == Json::ValueType::arrayValue) {
				for (unsigned int i = 0; i < otherWindowsVersions.size(); i++) {
					Json::String other_version = otherWindowsVersions[i].asString();
					if (!strchr(other_version.c_str(), '-')) {
						item = Json::String("Windows 10-") + other_version;
					}
					else {
						item = Json::String("Windows ") + other_version;
					}

					upd_file_info->os_versions.push_back(item);
					if (std::find(upd_file_info->os_versions.begin(), upd_file_info->os_versions.end(), item) == upd_file_info->os_versions.end()) {
						upd_file_info->os_versions.push_back(item);
					}
				}
			}
		}
	}
}

BOOL MakeSymbolServerUrl(TCHAR* file_name, int timestamp, int virtual_size, TCHAR *wszUrl) {

	if (!timestamp || !virtual_size) {
		return FALSE;
	}

	wsprintf(wszUrl, L"https://msdl.microsoft.com/download/symbols/%s/%08X%x/%s", file_name, timestamp, virtual_size, file_name);
	return TRUE;
}

std::vector<UPD_FILE_INFO> GetUpdateFileInfoList(TCHAR* wszIndexFileUrl) {
	
	ULONG status;
	TCHAR* lpStringBuffer = NULL;
	ULONG dwReaded = 0;
	
	status = BeginDownload(wszIndexFileUrl, NULL, &lpStringBuffer, &dwReaded);

	if (status != ERROR_SUCCESS || !dwReaded) {
		pWinBinDlg->m_StatusBar.SetText(_T("Internet connection error"), 3, 0);
		return {};
	}

	std::vector<UPD_FILE_INFO> m_upd_file_info_list;

	unsigned int out_size;
	GZipDecompress((unsigned char *)lpStringBuffer, dwReaded, NULL, out_size);

	unsigned char* szOutBuf = (unsigned char*)LocalAlloc(LPTR, out_size);
	GZipDecompress((unsigned char*)lpStringBuffer, dwReaded, szOutBuf, out_size);

	Json::Reader reader;
	Json::Value root;

	std::string str((char*)szOutBuf);
		
	if (!reader.parse(str, root)) {
		LocalFree(lpStringBuffer);
		return m_upd_file_info_list;
	}

	pWinBinDlg->m_infoRoot = root;

	Json::Value::Members upd_file_hashes = root.getMemberNames();
	for each(Json::String file_hash in upd_file_hashes) {
			
		Json::Value fileInfo = root[file_hash]["fileInfo"];
		Json::String file_version = fileInfo["version"].asString();
		adjust_file_version(&file_version);

			
		UPD_FILE_INFO upd_file_info;
		upd_file_info.file_name = pWinBinDlg->m_wszFileName;
		upd_file_info.hash = file_hash;
		upd_file_info.file_version = file_version;

		int type = fileInfo["timestamp"].type();
		if (type == Json::ValueType::intValue) {
			upd_file_info.timestamp = fileInfo["timestamp"].asInt();
		}
		else if (type == Json::ValueType::uintValue) {
			upd_file_info.timestamp = fileInfo["timestamp"].asUInt();
		}
		else{
			upd_file_info.timestamp = 0;
		}

		type = fileInfo["virtualSize"].type();
		if (type == Json::ValueType::intValue) {
			upd_file_info.virtual_size = fileInfo["virtualSize"].asInt();
		}
		else if (type == Json::ValueType::uintValue) {
			upd_file_info.virtual_size = fileInfo["virtualSize"].asUInt();
		}
		else {
			upd_file_info.virtual_size = 0;
		}

		GetUpdFileInfo(root[file_hash], &upd_file_info);

		m_upd_file_info_list.push_back(upd_file_info);
	}

	LocalFree(lpStringBuffer);
	LocalFree(szOutBuf);

	return m_upd_file_info_list;
}

DWORD WINAPI LoadFileNames(LPVOID lp) {
	TCHAR wszFileNameUrl[MAX_PATH];
	wsprintf(wszFileNameUrl, L"%s/filenames.json", WINBINDEX_DATA_URL);

	std::vector<Json::String> file_names = GetFileNames(wszFileNameUrl);

	wchar_t wszFileName[MAX_PATH];
	int ret;
	for each (Json::String file_name in file_names) {

		char* ptr = (char*)(file_name.c_str()) + strlen(file_name.c_str()) - 4;
		if (!strstr(ptr, ".exe") && !strstr(ptr, ".dll") && !strstr(ptr, ".sys")) {
			continue;
		}

		size_t out_size;
		size_t size = strlen(file_name.c_str()) + 1;
		mbstowcs_s(&out_size, wszFileName, size, file_name.c_str(), MAX_PATH);
		ret = pWinBinDlg->m_cmbFileName.AddString(wszFileName);
	}

	if (file_names.size() > 0) {
		pWinBinDlg->m_StatusBar.SetText(_T("Ready"), 3, 0);
	}

	return 0;
}

BOOL CWinBinDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_strSearch = L"";

	m_cmbFileName.AddString(L"..Select a file..");
	m_cmbFileName.SelectString(0, L"..Select a file..");

	// TODO:  Add extra initialization here
	m_List.SetExtendedStyle(m_List.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

	m_List.InsertColumn(COL_NUM, _T("No"), LVCFMT_LEFT, 30);
	m_List.InsertColumn(COL_OS_VERSION, _T("OS"), LVCFMT_LEFT, 110);
	m_List.InsertColumn(COL_OS_BUILD_NUM, _T("Build Version"), LVCFMT_LEFT, 150);
	m_List.InsertColumn(COL_FILE_VERSION, _T("File Version"), LVCFMT_LEFT, 110);
	m_List.InsertColumn(COL_UPDATE, _T("Update"), LVCFMT_LEFT, 100); 
	m_List.InsertColumn(COL_HASH, _T("SHA256"), LVCFMT_LEFT, 200);

	m_listDownload.SetExtendedStyle(m_List.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

	m_listDownload.InsertColumn(COL_DOWN_NUM, _T("No"), LVCFMT_LEFT, 30);
	m_listDownload.InsertColumn(COL_DOWN_FILENAME, _T("Filename"), LVCFMT_LEFT, 80);
	m_listDownload.InsertColumn(COL_DOWN_BUILD_NUM, _T("Build Version"), LVCFMT_LEFT, 110);
	m_listDownload.InsertColumn(COL_DOWN_HASH, _T("SHA256"), LVCFMT_LEFT, 150);
	m_listDownload.InsertColumn(COL_DOWN_STATUS, _T("Download Status"), LVCFMT_LEFT, 300);

	int widths[4];
	m_StatusBar.Create(WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), this, IDC_WIN_BIN_STATUSBAR);

	widths[0] = 100;
	widths[1] = 150;
	widths[2] = 250;
	widths[3] = 700;
	m_StatusBar.SetParts(4, widths);

	m_StatusBar.SetText(_T("Threads "), 0, 0);
	m_StatusBar.SetText(_T("0"), 1, 0);

	m_StatusBar.SetText(_T("Status "), 2, 0);
	m_StatusBar.SetText(_T("Ready"), 3, 0);

	CreateThread(NULL, 0, LoadFileNames, NULL, 0, NULL);
	m_cmbDropDown = FALSE;

	UpdateData(FALSE);
	return TRUE; 
}

void CWinBinDlg::ShowUpdFileInfoList() {
	m_List.DeleteAllItems();
	size_t out_size;
	size_t size;
	TCHAR szText[0x2000];

	CString strFilterVersion;
	GetDlgItemText(IDC_CMB_OS_VERSION, strFilterVersion);

	TCHAR wszFilerVersion[0x2000];
	_tcscpy_s(wszFilerVersion, strFilterVersion.GetBuffer(strFilterVersion.GetLength()));

	char filter_version[0x2000];
	size = _tcslen(wszFilerVersion) + 1;
	wcstombs_s(&out_size, filter_version, size, wszFilerVersion, 0x2000);

	UpdateData();

	TCHAR wszSearch[0x2000];
	_tcscpy_s(wszSearch, m_strSearch.GetBuffer(m_strSearch.GetLength()));

	char search_str[0x2000];
	size = _tcslen(wszSearch) + 1;
	wcstombs_s(&out_size, search_str, size, wszSearch, 0x2000);

	for each (UPD_FILE_INFO upd_file_info in m_upd_file_info_list) {

		if (strFilterVersion != L"All versions") {
			std::vector<Json::String> versions = upd_file_info.os_versions;
			if (std::find(versions.begin(), versions.end(), Json::String(filter_version) ) == versions.end()) {
				continue;
			}
		}

		bool filtered = true;
		if (m_strSearch != L"") {
			filtered = false;
			if (strstr(upd_file_info.file_version.c_str(), search_str)) {
				filtered = true;
			}

			if (strstr(upd_file_info.hash.c_str(), search_str)) {
				filtered = true;
			}

			if (!filtered) {
				for each (Json::String str in upd_file_info.os_build_nums) {
					if (strstr(str.c_str(), search_str)) {
						filtered = true;
						continue;
					}
				}
			}

			if (!filtered) {
				for each (Json::String str in upd_file_info.update_kbs) {
					if (strstr(str.c_str(), search_str)) {
						filtered = true;
						continue;
					}
				}
			}

			if (!filtered) {
				for each (Json::String str in upd_file_info.release_dates) {
					if (strstr(str.c_str(), search_str)) {
						filtered = true;
						continue;
					}
				}
			}
		}

		if (!filtered) {
			continue;
		}

		LV_ITEM lvitem;
		lvitem.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE;
		lvitem.iSubItem = 0;
		lvitem.stateMask = LVIS_STATEIMAGEMASK;
		lvitem.state = INDEXTOSTATEIMAGEMASK(1);
		lvitem.iImage = 2;
		lvitem.iItem = m_List.GetItemCount();
		lvitem.pszText = szText;
		int nIndex = m_List.InsertItem(&lvitem);

		_stprintf_s(szText, _T("%d"), nIndex + 1);
		m_List.SetItemText(nIndex, COL_NUM, szText);
		m_List.SetItemData(nIndex, nIndex);

		char* hash = (char*)(upd_file_info.hash.c_str());
		size = strlen(hash) + 1;
		mbstowcs_s(&out_size, szText, size, hash, 0x2000);

		m_List.SetItemText(nIndex, COL_HASH, szText);

		Json::String os_version = upd_file_info.os_versions[0];
		for (int i = 1; i < upd_file_info.os_versions.size(); i++) {
			os_version += Json::String(", ") + upd_file_info.os_versions[i];
		}

		size = strlen(os_version.c_str()) + 1;
		mbstowcs_s(&out_size, szText, size, os_version.c_str(), 0x2000);
		m_List.SetItemText(nIndex, COL_OS_VERSION, szText);

		Json::String file_version = upd_file_info.file_version;
		
		size = strlen(file_version.c_str()) + 1;
		mbstowcs_s(&out_size, szText, size, file_version.c_str(), 0x2000);

		m_List.SetItemText(nIndex, COL_FILE_VERSION, szText);

		Json::String update_kb = upd_file_info.update_kbs[0];
		for (int i = 1; i < upd_file_info.update_kbs.size(); i++) {
			update_kb += Json::String(", ") + upd_file_info.update_kbs[i];
		}
		size = strlen(update_kb.c_str()) + 1;
		mbstowcs_s(&out_size, szText, size, update_kb.c_str(), 0x2000);
		m_List.SetItemText(nIndex, COL_UPDATE, szText);

		if (upd_file_info.os_build_nums.size() == 0) {
			continue;
		}

		Json::String build_num = upd_file_info.os_build_nums[0];
		for (int i = 1; i < upd_file_info.os_build_nums.size(); i++) {
			build_num += Json::String(", ") + upd_file_info.os_build_nums[i];
		}
		size = strlen(build_num.c_str()) + 1;
		mbstowcs_s(&out_size, szText, size, build_num.c_str(), 0x2000);
		m_List.SetItemText(nIndex, COL_OS_BUILD_NUM, szText);
	}

	GetDlgItem(IDC_EDIT_SEARACH)->SetFocus();
}

void CWinBinDlg::OnCloseupCmbBinaryName()
{
	m_cmbDropDown = FALSE;
	CString strText;
	GetDlgItemText(IDC_CMB_BINARY_NAME, strText);
	if (strText == L"..Select a file..") {
		m_cmbFileName.SelectString(0, m_wszFileName);
		return;
	}

	int count = m_cmbVersionFilter.GetCount();
	for (int i = 0; i < count; i++) {
		m_cmbVersionFilter.DeleteString(0);
	}

	m_cmbVersionFilter.SetFocus();
	m_cmbFileName.SetFocus();

	UpdateData(FALSE);
	m_List.DeleteAllItems();

	_tcscpy_s(m_wszFileName, strText.GetBuffer(strText.GetLength()));

	TCHAR wszIndexFileUrl[MAX_PATH];
	wsprintf(wszIndexFileUrl, L"%s/by_filename_compressed/%s.json.gz",WINBINDEX_DATA_URL ,m_wszFileName);

	m_upd_file_info_list = GetUpdateFileInfoList(wszIndexFileUrl);

	if (m_upd_file_info_list.size() == 0) {
		return;
	}
	
	m_cmbVersionFilter.AddString(L"All versions");
	std::vector<Json::String> all_os_versions;

	size_t out_size;
	size_t size;
	TCHAR szText[0x2000];

	for each (UPD_FILE_INFO upd_file_info in m_upd_file_info_list) {
		for each (Json::String ver in upd_file_info.os_versions) {
			if (std::find(all_os_versions.begin(), all_os_versions.end(), ver) == all_os_versions.end()) {
				all_os_versions.push_back(ver);
				size = strlen(ver.c_str()) + 1;
				mbstowcs_s(&out_size, szText, size, ver.c_str(), MAX_PATH);
				m_cmbVersionFilter.AddString(szText);
			}
		}
	}

	m_cmbVersionFilter.SelectString(0, L"All versions");
	
	ShowUpdFileInfoList();
}

void CWinBinDlg::OnCloseupCmbOsVerion()
{
	m_List.DeleteAllItems();
	ShowUpdFileInfoList();
}

BOOL CWinBinDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_ESCAPE) {
			return TRUE;
		}
		if (pMsg->wParam == VK_RETURN)
		{
			if (GetFocus()->GetDlgCtrlID() != IDC_CMB_BINARY_NAME) {
				return TRUE;
			}

			if (!m_cmbDropDown) {
				return TRUE;
			}
		}
		
		if (GetFocus()->GetDlgCtrlID() == IDC_CMB_BINARY_NAME) {
			if (!m_cmbDropDown){
				if (pMsg->wParam == VK_UP || pMsg->wParam == VK_DOWN) {
					BOOL bRet = CDialogEx::PreTranslateMessage(pMsg);
					OnCloseupCmbBinaryName();
					return bRet;
				}
				return TRUE;
			}
		}
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}

typedef struct _SortData {
	CString	szText;
}SortData, * PSortData;

static int CALLBACK GenSort(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	SortData* d1 = (SortData*)lParam1;
	SortData* d2 = (SortData*)lParam2;
	int result;

	if (_tcslen(d1->szText) > _tcslen(d2->szText))
		result = 1;
	else if (_tcslen(d1->szText) < _tcslen(d2->szText))
		result = -1;
	else
		result = _tcscmp(d1->szText, d2->szText);
	return result;
}

void CWinBinDlg::OnColumnclickList(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	int count = m_List.GetItemCount();
	SortData* arStr = new SortData[count];
	for (int i = 0; i < count; i++)
	{
		arStr[i].szText = m_List.GetItemText(i, pNMListView->iSubItem);
		m_List.SetItemData(i, (LPARAM)(arStr + i));
	}
	m_List.SortItems(GenSort, 0);
	delete[] arStr;
	*pResult = 0;
}


void CWinBinDlg::OnDropdownCmbBinaryName()
{
	m_cmbDropDown = TRUE;
	if (m_cmbFileName.GetCount() == 1) {
		LoadFileNames(NULL);
	}
}

void CWinBinDlg::ShowSelectedFileInfo(int itemSelected) {
	if (itemSelected == -1) {
		return;
	}
	TCHAR szText[0x2000];
	_tcscpy_s(szText, m_List.GetItemText(itemSelected, COL_HASH));

	char file_hash[0x2000];
	size_t out_size;
	size_t size = _tcslen(szText) + 1;
	wcstombs_s(&out_size, file_hash, size, szText, 0x2000);

	Json::Value	file_info = m_infoRoot[file_hash];
	UPD_FILE_INFO upd_file_info;
	upd_file_info.timestamp = 0;
	upd_file_info.virtual_size = 0;
	bool bfound = false;
	for (int i = 0; i < m_upd_file_info_list.size(); i++) {
		if (m_upd_file_info_list[i].hash == file_hash) {
			upd_file_info = m_upd_file_info_list[i];
			bfound = true;
			break;
		}
	}

	TCHAR wszDownloadURL[0x2000] = L"";
	CString strFileName = upd_file_info.file_name;
	MakeSymbolServerUrl(strFileName.GetBuffer(strFileName.GetLength()), upd_file_info.timestamp, upd_file_info.virtual_size, wszDownloadURL);

	char download_url[0x2000];
	size = _tcslen(wszDownloadURL) + 1;
	wcstombs_s(&out_size, download_url, size, wszDownloadURL, 0x2000);

	file_info["download_url"] = download_url;
	Json::String detailed_info = file_info.toStyledString();
	
	int index = 0;
	while (true) {
		index = detailed_info.find("\n", index);
		if (index == std::string::npos) {
			break;
		}
		detailed_info.replace(index, 1, "\r\n");
		index += 2;
	}
	size = strlen(detailed_info.c_str()) + 1;
	mbstowcs_s(&out_size, szText, size, detailed_info.c_str(), 0x2000);

	CDetailedInfoDlg infoDlg;
	infoDlg.m_strDetailedInfo = szText;
	infoDlg.DoModal();
}

void CWinBinDlg::OnBnClickedCheckPdb()
{
	UpdateData();
	if (m_bDownSym) {
		m_bDownBin = TRUE;
		UpdateData(FALSE);
	}

	if (!m_bDownSym && !m_bDownBin) {
		::EnableWindow(::GetDlgItem(m_hWnd, IDC_BTN_DOWN), FALSE);
	}
	else {
		::EnableWindow(::GetDlgItem(m_hWnd, IDC_BTN_DOWN), TRUE);
	}
}

void CWinBinDlg::OnBnClickedCheckBin()
{
	UpdateData();
	if (!m_bDownBin) {
		m_bDownSym = FALSE;
		UpdateData(FALSE);
	}

	if (!m_bDownSym && !m_bDownBin) {
		::EnableWindow(::GetDlgItem(m_hWnd, IDC_BTN_DOWN), FALSE);
	}
	else {
		::EnableWindow(::GetDlgItem(m_hWnd, IDC_BTN_DOWN), TRUE);
	}
}

DWORD WINAPI DownloadThread(LPVOID lp)
{
	int nItem = (int)lp;
	CListCtrl* pListDownload = &pWinBinDlg->m_listDownload;

	TCHAR wszText[0x2000];
	_tcscpy_s(wszText, pListDownload->GetItemText(nItem, COL_DOWN_HASH));

	char file_hash[0x2000];
	size_t out_size;
	size_t size = _tcslen(wszText) + 1;
	wcstombs_s(&out_size, file_hash, size, wszText, 0x2000);

	UPD_FILE_INFO upd_file_info;
	upd_file_info.timestamp = 0;
	upd_file_info.virtual_size = 0;
	bool bfound = false;
	for (int i = 0; i < pWinBinDlg->m_upd_download_list.size(); i++) {
		if (pWinBinDlg->m_upd_download_list[i].hash == file_hash) {
			upd_file_info = pWinBinDlg->m_upd_download_list[i];
			bfound = true;
			break;
		}
	}

	TCHAR wszDownloadURL[0x2000] = L"";
	CString strFileName = upd_file_info.file_name;
	MakeSymbolServerUrl(strFileName.GetBuffer(strFileName.GetLength()), upd_file_info.timestamp, upd_file_info.virtual_size, wszDownloadURL);

	CString strDownloadURL = wszDownloadURL;
	if (strDownloadURL == L"") {
		InterlockedDecrement((LONG volatile*)&pWinBinDlg->m_nThreadCount);
		pListDownload->SetItemText(nItem, COL_DOWN_STATUS, _T("Can't determine download URL"));
		return 0;
	}
	
	TCHAR wszDirPath[MAX_PATH] = L"";
	GetCurrentDirectory(MAX_PATH, wszDirPath);

	wsprintf(wszDirPath, L"%s\\%s", wszDirPath, L"Downloads");
	CreateDirectory(wszDirPath, NULL);

	_tcscpy_s(wszText, strFileName.GetBuffer(strFileName.GetLength()));
	wsprintf(wszDirPath, L"%s\\%s", wszDirPath, wszText);
	CreateDirectory(wszDirPath, NULL);

	TCHAR wszFullFilePath[MAX_PATH];
	TCHAR wszBinName[MAX_PATH];
	TCHAR wszExtension[5];

	_tcscpy_s(wszBinName, wszText);
	_tcscpy_s(wszExtension, wszText + wcslen(wszBinName) - 4 );
	wszBinName[wcslen(wszBinName) - 4] = 0;

	_tcscpy_s(wszText, pListDownload->GetItemText(nItem, COL_DOWN_BUILD_NUM));
	TCHAR* ptr = wcschr(wszText, L',');
	if (ptr) *ptr = L'\x00';

	TCHAR wszDownFileName[MAX_PATH];
	wsprintf(wszDownFileName, L"%s_%s%s", wszBinName, wszText, wszExtension);
	wsprintf(wszFullFilePath, L"%s\\%s", wszDirPath, wszDownFileName);

	HANDLE hFile = CreateFile(wszFullFilePath, GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		InterlockedDecrement((LONG volatile*)&pWinBinDlg->m_nThreadCount);
		pListDownload->SetItemText(nItem, COL_DOWN_STATUS, _T("Creating file failed"));
		return 0;
	}

	wsprintf(wszText, L"%d", pWinBinDlg->m_nThreadCount);
	pWinBinDlg->m_StatusBar.SetText(wszText, 1, 0);
	
	wsprintf(wszText, L"Downloading PE ...");
	pListDownload->SetItemText(nItem, COL_DOWN_STATUS, wszText);

	ULONG status;
	ULONG dwReaded = 0;

	status = BeginDownload(wszDownloadURL, hFile, NULL, &dwReaded);
	CloseHandle(hFile);

	if (status != ERROR_SUCCESS || !dwReaded) {
		InterlockedDecrement((LONG volatile*)&pWinBinDlg->m_nThreadCount);
		wsprintf(wszText, L"%d", pWinBinDlg->m_nThreadCount);
		pWinBinDlg->m_StatusBar.SetText(wszText, 1, 0);

		wsprintf(wszText, L"Downloading PE failed");
		pListDownload->SetItemText(nItem, COL_DOWN_STATUS, wszText);
		DeleteFile(wszFullFilePath);
		return 0;
	}

	wsprintf(wszText, L"Downloading PE success");
	pListDownload->SetItemText(nItem, COL_DOWN_STATUS, wszText);

	if (!pWinBinDlg->m_bDownSym) {
		InterlockedDecrement((LONG volatile*)&pWinBinDlg->m_nThreadCount);
		wsprintf(wszText, L"%d", pWinBinDlg->m_nThreadCount);
		pWinBinDlg->m_StatusBar.SetText(wszText, 1, 0);
		return 0;
	}

	wsprintf(wszText, L"Downloading PDB ...");
	pListDownload->SetItemText(nItem, COL_DOWN_STATUS, wszText);

	CString strError = _T("Error occured.");
	BOOL bSuccess = FALSE;

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	TCHAR szRun[1024] = { 0 };
	TCHAR cmdline[1024] = { 0 };
	TCHAR szTmpFilePath[1024];
	TCHAR szFileTitle[260];

	GetFileTitle(wszFullFilePath, szFileTitle, 250);

	GetTempPath(1024, szTmpFilePath);
	_stprintf_s(szTmpFilePath, _T("%s%s%d%s"), szTmpFilePath, szFileTitle, pWinBinDlg->m_nThreadCount, _T(".log"));

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	_stprintf_s(szRun, _T("symchk.exe /r /if %s /s SRV*%s*%s"), (LPCTSTR)wszFullFilePath, wszDirPath, MS_SYMBOL_SERVER);
	_stprintf_s(cmdline, _T("cmd.exe /c \"%s > %s\""), szRun, szTmpFilePath);

	CreateProcess(NULL, cmdline, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
	WaitForSingleObject(pi.hProcess, INFINITE);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	hFile = CreateFile(szTmpFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		DWORD dwRead;
		DWORD dwSize = GetFileSize(hFile, NULL);
		char* szBuf = (char*)LocalAlloc(LPTR, dwSize + 0x1000);
		ReadFile(hFile, szBuf, dwSize, &dwRead, NULL);
		CloseHandle(hFile);

		char szLine[300] = { 0 };
		if (strline(szBuf, szLine) != NULL)
		{
			if (strlen(szLine) >= 3)
			{
				TCHAR wszLine[300];
				mbstowcs(wszLine, szLine, 300);
				strError = wszLine;
			}
			else
			{
				bSuccess = TRUE;
				wsprintf(wszText, L"Downloading PDB success");
				strError = wszText;
			}
		}
		LocalFree(szBuf);
	}

	DeleteFile(szTmpFilePath);
	pListDownload->SetItemText(nItem, COL_DOWN_STATUS, strError);

	InterlockedDecrement((LONG volatile*)&pWinBinDlg->m_nThreadCount);
	wsprintf(wszText, L"%d", pWinBinDlg->m_nThreadCount);
	pWinBinDlg->m_StatusBar.SetText(wszText, 1, 0);

	return bSuccess;
}

DWORD WINAPI DownloadListedFiles(LPVOID lp) {

	CListCtrl* pListDownload = &pWinBinDlg->m_listDownload;
	int nItemCount = pListDownload->GetItemCount();

	pWinBinDlg->m_StatusBar.SetText(L"Downloading files ...", 3, 0);

	CString strStatus;
	for (int i = 0; i < nItemCount; i++) {
		strStatus = pListDownload->GetItemText(i, COL_DOWN_STATUS);
		if (_tcsstr(strStatus.GetBuffer(strStatus.GetLength()), L"...") 
			|| _tcsstr(strStatus.GetBuffer(strStatus.GetLength()), L"success")) continue;

		while (pWinBinDlg->m_nThreadCount >= 10) {
			Sleep(20);
		}
		if (pMainDlg->m_bExitPending) break;
		InterlockedIncrement((LONG volatile*)&pWinBinDlg->m_nThreadCount);
		CreateThread(NULL, 0, DownloadThread, (LPVOID)i, 0, NULL);
	}
	
	while (TRUE)
	{
		if (pWinBinDlg->m_nThreadCount == 0)
			break;
		Sleep(300);
	}

	pWinBinDlg->m_StatusBar.SetText(L"Downloading finished.", 3, 0);
	return 0;
}

void CWinBinDlg::OnBnClickedBtnDown()
{
	if (!m_bDownBin && !m_bDownSym) {
		return;
	}

	if (m_listDownload.GetItemCount() == 0) {
		return;
	}

	CreateThread(NULL, 0, DownloadListedFiles, NULL, 0, NULL);
}


void CWinBinDlg::OnChangeEditSearach()
{
	ShowUpdFileInfoList();
}

DWORD WINAPI ExitThread(LPVOID lp) {
	pMainDlg->Close();
	return 0;
}

void CWinBinDlg::OnBnClickedBtnExit()
{
	if (pMainDlg->m_bExitPending) {
		return;
	}

	if (AfxMessageBox(L"Really want to exit?", MB_YESNO) == IDYES) {
		pMainDlg->m_bExitPending = TRUE;
		CreateThread(NULL, 0, ExitThread, NULL, 0, NULL);
	}
}


void CWinBinDlg::OnRButtonDown(UINT nFlags, CPoint point)
{
	RECT listRect;
	m_List.GetWindowRect(&listRect);
	CDialogEx::OnRButtonDown(nFlags, point);
}


void CWinBinDlg::OnRclickList(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	*pResult = 0;
	if (m_List.GetSelectedCount() == 0) {
		return;
	}
	
	CPoint pt1, pt2;
	CPoint pt;
	GetCursorPos(&pt1);
	pt2 = pt1;

	m_List.ScreenToClient(&pt1);

	CMenu menu;
	if (menu.LoadMenu(IDR_MENU_OPERATION))
	{
		CMenu* pMenum_Tree = menu.GetSubMenu(0);
		
		if (m_List.GetSelectedCount() > 1) {
			pMenum_Tree->EnableMenuItem(0, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
		}
		
		pMenum_Tree->TrackPopupMenu(TPM_LEFTALIGN, pt2.x, pt2.y, this);
	}

}

void CWinBinDlg::OnOperationViewinfo()
{
	int itemSelected = (int)m_List.GetFirstSelectedItemPosition() - 1;
	ShowSelectedFileInfo(itemSelected);

}

void CWinBinDlg::OnDblclkList(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	ShowSelectedFileInfo(pNMItemActivate->iItem);
	*pResult = 0;
}


void CWinBinDlg::OnOperationAddto()
{
	int nItemCnt = m_List.GetItemCount();

	LV_ITEM lvitem;
	TCHAR wszText[0x2000];
	char file_hash[0x2000];
	size_t out_size;

	for (int i = 0; i < nItemCnt; i++) {
		if (m_List.GetItemState(i, LVIS_SELECTED)) {
			CString strHash = m_List.GetItemText(i, COL_HASH);
			bool already_queued = false;
			for (int index = 0; index < m_listDownload.GetItemCount(); index++) {
				if (m_listDownload.GetItemText(index, COL_DOWN_HASH) == strHash) {
					already_queued = true;
					break;
				}
			}

			if (already_queued) continue;

			_tcscpy_s(wszText, strHash.GetBuffer(strHash.GetLength()));
			size_t size = _tcslen(wszText) + 1;
			wcstombs_s(&out_size, file_hash, size, wszText, 0x2000);

			UPD_FILE_INFO upd_file_info;
			upd_file_info.timestamp = 0;
			upd_file_info.virtual_size = 0;
			bool bfound = false;
			for (int i = 0; i < pWinBinDlg->m_upd_file_info_list.size(); i++) {
				if (pWinBinDlg->m_upd_file_info_list[i].hash == file_hash) {
					upd_file_info = pWinBinDlg->m_upd_file_info_list[i];
					bfound = true;
					break;
				}
			}

			if (!bfound) continue;

			m_upd_download_list.push_back(upd_file_info);

			lvitem.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE;
			lvitem.iSubItem = 0;
			lvitem.stateMask = LVIS_STATEIMAGEMASK;
			lvitem.state = INDEXTOSTATEIMAGEMASK(1);
			lvitem.iImage = 2;
			lvitem.iItem = m_listDownload.GetItemCount();
			lvitem.pszText = wszText;
			int nIndex = m_listDownload.InsertItem(&lvitem);

			_stprintf_s(wszText, _T("%d"), nIndex + 1);
			m_listDownload.SetItemText(nIndex, COL_DOWN_NUM, wszText);
			m_listDownload.SetItemText(nIndex, COL_DOWN_FILENAME, upd_file_info.file_name);
			m_listDownload.SetItemText(nIndex, COL_DOWN_BUILD_NUM, m_List.GetItemText(i, COL_OS_BUILD_NUM));
			m_listDownload.SetItemText(nIndex, COL_DOWN_HASH, strHash);

		}
	}
}

DWORD WINAPI DownloadSelectedFiles(LPVOID lp) {

	int nInsertedCnt = (int)lp;
	CListCtrl* pListDownload = &pWinBinDlg->m_listDownload;
	
	int nItemCnt = pListDownload->GetItemCount();
	pWinBinDlg->m_StatusBar.SetText(L"Downloading files ...", 3, 0);

	CString strStatus;
	for (int i = nItemCnt - nInsertedCnt; i < nItemCnt; i++) {
		strStatus = pListDownload->GetItemText(i, COL_DOWN_STATUS);
		if (_tcsstr(strStatus.GetBuffer(strStatus.GetLength()), L"...")
			|| _tcsstr(strStatus.GetBuffer(strStatus.GetLength()), L"success")) continue;

		while (pWinBinDlg->m_nThreadCount >= 10) {
			Sleep(20);
		}
		if (pMainDlg->m_bExitPending) break;

		InterlockedIncrement((LONG volatile*)&pWinBinDlg->m_nThreadCount);
		CreateThread(NULL, 0, DownloadThread, (LPVOID)i, 0, NULL);
	}

	while (TRUE)
	{
		if (pWinBinDlg->m_nThreadCount == 0)
			break;
		Sleep(300);
	}
	
	pWinBinDlg->m_StatusBar.SetText(L"Downloading finished.", 3, 0);

	return 0;
}

void CWinBinDlg::OnOperationDownload(BOOL bOnlyBin) {
	int nItemCnt = m_List.GetItemCount();
	int nInsertedCnt = 0;
	LV_ITEM lvitem;
	TCHAR wszText[0x2000];

	char file_hash[0x2000];
	size_t out_size;

	for (int i = 0; i < nItemCnt; i++) {
		if (m_List.GetItemState(i, LVIS_SELECTED)) {
			CString strHash = m_List.GetItemText(i, COL_HASH);
			bool already_queued = false;
			for (int index = 0; index < m_listDownload.GetItemCount(); index++) {
				if (m_listDownload.GetItemText(index, COL_DOWN_HASH) == strHash) {
					already_queued = true;
					break;
				}
			}

			if (already_queued) continue;

			_tcscpy_s(wszText, strHash.GetBuffer(strHash.GetLength()));
			size_t size = _tcslen(wszText) + 1;
			wcstombs_s(&out_size, file_hash, size, wszText, 0x2000);

			UPD_FILE_INFO upd_file_info;
			upd_file_info.timestamp = 0;
			upd_file_info.virtual_size = 0;
			bool bfound = false;
			for (int i = 0; i < pWinBinDlg->m_upd_file_info_list.size(); i++) {
				if (pWinBinDlg->m_upd_file_info_list[i].hash == file_hash) {
					upd_file_info = pWinBinDlg->m_upd_file_info_list[i];
					bfound = true;
					break;
				}
			}

			if (!bfound) continue;

			m_upd_download_list.push_back(upd_file_info);

			lvitem.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE;
			lvitem.iSubItem = 0;
			lvitem.stateMask = LVIS_STATEIMAGEMASK;
			lvitem.state = INDEXTOSTATEIMAGEMASK(1);
			lvitem.iImage = 2;
			lvitem.iItem = m_listDownload.GetItemCount();
			lvitem.pszText = wszText;
			int nIndex = m_listDownload.InsertItem(&lvitem);

			_stprintf_s(wszText, _T("%d"), nIndex + 1);
			m_listDownload.SetItemText(nIndex, COL_DOWN_NUM, wszText);
			m_listDownload.SetItemText(nIndex, COL_DOWN_FILENAME, upd_file_info.file_name);
			m_listDownload.SetItemText(nIndex, COL_DOWN_BUILD_NUM, m_List.GetItemText(i, COL_OS_BUILD_NUM));
			m_listDownload.SetItemText(nIndex, COL_DOWN_HASH, strHash);

			nInsertedCnt++;
		}
	}

	if (bOnlyBin) {
		m_bDownBin = TRUE;
		m_bDownSym = FALSE;
	}
	else {
		m_bDownBin = TRUE;
		m_bDownSym = TRUE;
	}

	UpdateData(FALSE);
	CreateThread(NULL, 0, DownloadSelectedFiles, (LPVOID)nInsertedCnt, 0, NULL);
}
void CWinBinDlg::OnOperationDownloadbinary()
{
	OnOperationDownload(TRUE);
}


void CWinBinDlg::OnDownloadBinarysymbol()
{
	OnOperationDownload(FALSE);
}


void CWinBinDlg::OnRclickListDownload(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	*pResult = 0;

	CPoint pt1, pt2;
	CPoint pt;
	GetCursorPos(&pt1);
	pt2 = pt1;

	m_listDownload.ScreenToClient(&pt1);

	CMenu menu;
	if (menu.LoadMenu(IDR_MENU_DOWN_LIST))
	{
		CMenu* pMenum_Tree = menu.GetSubMenu(0);
		if (m_listDownload.GetSelectedCount() == 0 ) {
			pMenum_Tree->EnableMenuItem(0, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
		}

		if (m_listDownload.GetItemCount() == 0 ) {
			pMenum_Tree->EnableMenuItem(1, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
		}

		TCHAR wszText[0x2000];
		CString strStatus = m_listDownload.GetItemText((int)m_listDownload.GetFirstSelectedItemPosition() - 1, COL_DOWN_STATUS);

		_tcscpy_s(wszText, strStatus.GetBuffer(strStatus.GetLength()));
		if (m_listDownload.GetSelectedCount() != 1 || !_tcsstr(wszText, L"success")) {
			pMenum_Tree->EnableMenuItem(3, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
		}

		pMenum_Tree->TrackPopupMenu(TPM_LEFTALIGN, pt2.x, pt2.y, this);
	}
}


void CWinBinDlg::OnDlRemove()
{
	if (m_nThreadCount > 0) {
		AfxMessageBox(L"Please try again after busy threads terminated.");
		return;
	}

	LV_ITEM lvitem;
	TCHAR wszText[0x2000];

	char file_hash[0x2000];
	size_t out_size;
	for (int i = 0; i < m_listDownload.GetItemCount(); i++) {
		if (m_listDownload.GetItemState(i, LVIS_SELECTED)) {
			CString strHash = m_listDownload.GetItemText(i, COL_DOWN_HASH);

			_tcscpy_s(wszText, strHash.GetBuffer(strHash.GetLength()));
			size_t size = _tcslen(wszText) + 1;
			wcstombs_s(&out_size, file_hash, size, wszText, 0x2000);

			UPD_FILE_INFO upd_file_info;
			upd_file_info.timestamp = 0;
			upd_file_info.virtual_size = 0;
			bool bfound = false;
			for (int i = 0; i < pWinBinDlg->m_upd_download_list.size(); i++) {
				if (pWinBinDlg->m_upd_download_list[i].hash == file_hash) {
					upd_file_info = pWinBinDlg->m_upd_download_list[i];
					bfound = true;
					break;
				}
			}

			if (!bfound) continue;
			
			pWinBinDlg->m_upd_download_list.erase(std::find(pWinBinDlg->m_upd_download_list.begin(), pWinBinDlg->m_upd_download_list.end(), upd_file_info));
			m_listDownload.DeleteItem(i);
			i--;
		}
	}

	for (int i = 0; i < m_listDownload.GetItemCount(); i ++) {
		_stprintf_s(wszText, _T("%d"), i + 1);
		m_listDownload.SetItemText(i, COL_DOWN_NUM, wszText);
	}


}


void CWinBinDlg::OnDlClearall()
{
	if (m_nThreadCount > 0) {
		AfxMessageBox(L"Please try again after busy threads terminated.");
		return;
	}
	m_upd_download_list.clear();
	m_listDownload.DeleteAllItems();
}


void CWinBinDlg::OnDlOpenfilelocation()
{
	TCHAR wszDirPath[MAX_PATH] = L"";
	GetCurrentDirectory(MAX_PATH, wszDirPath);

	wsprintf(wszDirPath, L"%s\\%s", wszDirPath, L"Downloads");
	CString strFileName = m_listDownload.GetItemText((int)m_listDownload.GetFirstSelectedItemPosition() - 1, COL_DOWN_FILENAME);
	wsprintf(wszDirPath, L"%s\\%s", wszDirPath, strFileName.GetBuffer(strFileName.GetLength()));

	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	TCHAR cmdline[1024] = { 0 };
	wsprintf(cmdline, _T("explorer.exe %s"), wszDirPath);

	CreateProcess(NULL, cmdline, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
}
