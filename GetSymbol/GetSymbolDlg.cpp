
// GetSymbolDlg.cpp : implementation file
//

#include "stdafx.h"
#include "GetSymbol.h"
#include "GetSymbolDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

enum LIST_COLUMNS {
	COLUMN_NO,
	COLUMN_SRCFILE,
	COLUMN_STAT,
};

#define COLUMN_COUNT		(COLUMN_STAT + 1)

#define MS_SYMBOL_SERVER			_T("http://msdl.microsoft.com/download/symbols")
#define MOZILLA_SYMBOL_SERVER		_T("http://symbols.mozilla.org/firefox")
#define GOOGLE_SYMBOL_SERVER		_T("https://chromium-browser-symsrv.commondatastorage.googleapis.com/tmpq4t67i74")
#define CITRIX_SYMBOL_SERVER		_T("http://ctxsym.citrix.com/symbols")

TCHAR szSymUrl[MAX_PATH];

DWORD WINAPI StartProc(LPVOID lp);
void FileSearch(CString strPath);
void GetPDB(CString szFilePath);
BOOL isPE(CString szFilePath);
DWORD WINAPI SymCheckThread(LPVOID lp);
char* strline(char* szStr, char* szLine);

DWORD g_dwThreadRunningCount;

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CGetSymbolDlg dialog



CGetSymbolDlg::CGetSymbolDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CGetSymbolDlg::IDD, pParent)
	, m_ThreadCount(0)
	, m_SrcPath(_T("c:\\windows\\system32"))
	, m_DestPath(_T("c:\\symbols"))
{
	m_bBusy = FALSE;
	m_ThreadCount = 10;
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CGetSymbolDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_THREAD_COUNT, m_ThreadCount);
	DDV_MinMaxUInt(pDX, m_ThreadCount, 1, 30);
	DDX_Text(pDX, IDC_EDIT_SRC_PATH, m_SrcPath);
	DDX_Text(pDX, IDC_EDIT_DEST_PATH, m_DestPath);
	DDX_Control(pDX, IDC_LIST, m_List);
}

BEGIN_MESSAGE_MAP(CGetSymbolDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(ID_BTN_EXIT, &CGetSymbolDlg::OnBnClickedBtnExit)
	ON_BN_CLICKED(IDC_BTN_SRC, &CGetSymbolDlg::OnBnClickedBtnSrc)
	ON_BN_CLICKED(IDC_BTN_DEST, &CGetSymbolDlg::OnBnClickedBtnDest)
	ON_BN_CLICKED(ID_BTN_START, &CGetSymbolDlg::OnBnClickedBtnStart)
//	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST, &CGetSymbolDlg::OnColumnclickList)
ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST, &CGetSymbolDlg::OnColumnclickList)
ON_BN_CLICKED(IDC_RADIO_MS, &CGetSymbolDlg::OnBnClickedRadioMs)
ON_BN_CLICKED(IDC_RADIO_GOOGLE, &CGetSymbolDlg::OnBnClickedRadioGoogle)
ON_BN_CLICKED(IDC_RADIO_MOZILLA, &CGetSymbolDlg::OnBnClickedRadioMozilla)
ON_BN_CLICKED(IDC_RADIO_CITRIX, &CGetSymbolDlg::OnBnClickedRadioCitrix)
END_MESSAGE_MAP()


// CGetSymbolDlg message handlers

void SplitString(TCHAR* string, TCHAR spliter, TCHAR* first_part, TCHAR* second_part) {
	
	wcscpy(first_part, string);
	TCHAR* ptr = wcschr(first_part, spliter);
	*ptr = L'\0x0';

	ptr = wcschr(string, spliter);
	wcscpy(second_part, ptr + 1);
}

DWORD WINAPI UpdateCheckThread(LPVOID lp) {

	ULONG status;

	TCHAR * lpStringBuffer = NULL;

	status = BeginDownload(UPDATE_CHECK_URL, NULL, &lpStringBuffer);
	
	if (status != ERROR_SUCCESS) {
		return status;
	}

	TCHAR lpszVersionInfo[MAX_PATH];
	TCHAR lpszUpdateURL[MAX_PATH];

	RtlZeroMemory(lpszVersionInfo, sizeof(lpszVersionInfo));
	RtlZeroMemory(lpszUpdateURL, sizeof(lpszUpdateURL));

	SplitString(lpStringBuffer, L'|', lpszVersionInfo, lpszUpdateURL);

	TCHAR lpszAppNameVersion[MAX_PATH];
	wsprintf(lpszAppNameVersion, L"%s %s", APP_NAME, APP_VERSION);

	if (wcscmp(lpszVersionInfo, lpszAppNameVersion) > 0) {
		TCHAR msg[MAX_PATH] = L"Update available. Download and install now?\r\n\r\n";
		wcscat(msg, lpszVersionInfo);
		
		if (MessageBoxW(0, msg, APP_NAME, MB_YESNO | MB_ICONINFORMATION) == IDYES) {
			TCHAR szModuleFileName[MAX_PATH];
			TCHAR szNewFileName[MAX_PATH];
			GetModuleFileName(NULL, szModuleFileName, MAX_PATH);
			wsprintf(szNewFileName, L"%s%s", szModuleFileName, L".tmp");

			DeleteFile(szNewFileName);
			MoveFile(szModuleFileName, szNewFileName);

			HANDLE hFile =  CreateFile(szModuleFileName, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, NULL);

			if (hFile != INVALID_HANDLE_VALUE)
			{
				BeginDownload(lpszUpdateURL, hFile, NULL);
				CloseHandle(hFile);

				STARTUPINFO si;
				PROCESS_INFORMATION pi;

				ZeroMemory(&si, sizeof(si));
				si.cb = sizeof(si);
				ZeroMemory(&pi, sizeof(pi));
				CreateProcess(NULL, szModuleFileName, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);

				ExitProcess(0);
			}
		}
	}

	LocalFree(lpStringBuffer);
	return 0;
} 

BOOL CGetSymbolDlg::OnInitDialog()
{ 
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);
	 
	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	TCHAR lpszAppNameVersion[MAX_PATH];
	wsprintf(lpszAppNameVersion, L"%s %s", APP_NAME, APP_VERSION);

	SetWindowText(lpszAppNameVersion);

	m_List.SetExtendedStyle(m_List.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
	m_List.InsertColumn(COLUMN_NO, _T("No"), LVCFMT_LEFT, 60);
	m_List.InsertColumn(COLUMN_SRCFILE, _T("Source File Path"), LVCFMT_LEFT, 340);
	m_List.InsertColumn(COLUMN_STAT, _T("Download Status"), LVCFMT_LEFT, 290);

	CheckRadioButton(IDC_RADIO_MS, IDC_RADIO_CITRIX, IDC_RADIO_MS);
	_tcscpy_s(szSymUrl, MS_SYMBOL_SERVER);

	
	CreateThread(0, 0, UpdateCheckThread, 0, 0, 0);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CGetSymbolDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else if (nID == SC_CLOSE)
	{
		OnBnClickedBtnExit();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CGetSymbolDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CGetSymbolDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CGetSymbolDlg::OnBnClickedBtnExit()
{
	if (m_bBusy)
		return;

	DeleteFile(_T("dbghelp.dll"));
	DeleteFile(_T("symbolcheck.dll"));
	DeleteFile(_T("symchk.exe"));
	DeleteFile(_T("dbgeng.dll"));
	DeleteFile(_T("DbgModel.dll"));
	DeleteFile(_T("msvcrt.dll"));
	DeleteFile(_T("symsrv.dll"));
	DeleteFile(_T("symsrv.yes"));
	CDialogEx::OnCancel();
}


BOOL CGetSymbolDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)
			return TRUE;
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}


void CGetSymbolDlg::OnBnClickedBtnSrc()
{
	ITEMIDLIST		*pidlBrowse;
	TCHAR			pszPathname[MAX_PATH];

	BROWSEINFO BrInfo;
	BrInfo.hwndOwner = GetSafeHwnd();
	BrInfo.pidlRoot = NULL;

	memset(&BrInfo, 0, sizeof(BrInfo));
	BrInfo.pszDisplayName = pszPathname;
	BrInfo.lpszTitle = L"Select Source Directory.";
	BrInfo.ulFlags = BIF_RETURNONLYFSDIRS;

	pidlBrowse = SHBrowseForFolder(&BrInfo);

	if (pidlBrowse != NULL)
	{

		::SHGetPathFromIDList(pidlBrowse, pszPathname);
		m_SrcPath = pszPathname;
		m_DestPath = pszPathname;
		if (m_DestPath[m_DestPath.GetLength() - 1] == _T('\\'))
			m_DestPath += "Symbols";
		else
			m_DestPath += "\\Symbols";

		UpdateData(FALSE);
	}
}


void CGetSymbolDlg::OnBnClickedBtnDest()
{
	ITEMIDLIST		*pidlBrowse;
	TCHAR			pszPathname[MAX_PATH];

	BROWSEINFO BrInfo;
	BrInfo.hwndOwner = GetSafeHwnd();
	BrInfo.pidlRoot = NULL;

	memset(&BrInfo, 0, sizeof(BrInfo));
	BrInfo.pszDisplayName = pszPathname;
	BrInfo.lpszTitle = L"Select Dest Directory.";
	BrInfo.ulFlags = BIF_RETURNONLYFSDIRS;

	pidlBrowse = SHBrowseForFolder(&BrInfo);

	if (pidlBrowse != NULL)
	{

		::SHGetPathFromIDList(pidlBrowse, pszPathname);
		m_DestPath = pszPathname;
		UpdateData(FALSE);
	}
}


void CGetSymbolDlg::OnBnClickedBtnStart()
{
	if (m_bBusy)
		return;

	UpdateData(TRUE);
	m_List.DeleteAllItems();

	HANDLE hThread = CreateThread(NULL, 0, StartProc, NULL, 0, NULL);
	CloseHandle(hThread);	
}

DWORD WINAPI StartProc(LPVOID lp)
{
	pDlg->m_bBusy = TRUE;
	pDlg->GetDlgItem(ID_BTN_START)->EnableWindow(FALSE);
	pDlg->GetDlgItem(ID_BTN_EXIT)->EnableWindow(FALSE);

	pDlg->m_SuccCount = pDlg->m_FailCount = g_dwThreadRunningCount = 0;

	FileSearch(pDlg->m_SrcPath);

	while (TRUE)
	{
		if (g_dwThreadRunningCount == 0)
			break;
		Sleep(300);
	}

	DWORD dwTotal = pDlg->m_SuccCount + pDlg->m_FailCount;
	CString str;
	str.Format(_T("Symbol Download Finished! \r\n\r\n        Success  :  [%d / %d] \n        Failed     :  [%d / %d]"), pDlg->m_SuccCount, dwTotal, pDlg->m_FailCount, dwTotal);
	AfxMessageBox(str);

	pDlg->m_bBusy = FALSE;
	pDlg->GetDlgItem(ID_BTN_START)->EnableWindow(TRUE);
	pDlg->GetDlgItem(ID_BTN_EXIT)->EnableWindow(TRUE);
	return 0;
}

void FileSearch(CString strPath)
{
	TCHAR szFolderPath[1024];
	TCHAR szTmpPath[1024];
	HANDLE hFind = NULL;
	WIN32_FIND_DATA fileinfo;

	if (strPath[strPath.GetLength() - 1] == _T('\\'))
		strPath.SetAt(strPath.GetLength() - 1, _T('\0'));

	_tcscpy_s(szFolderPath, strPath);
	_tcscat_s(szFolderPath, _T("\\*.*"));

	hFind = FindFirstFile(szFolderPath, &fileinfo);

	if (hFind == INVALID_HANDLE_VALUE)
		return;

	do
	{
		if (!_tcscmp(fileinfo.cFileName, _T(".")) || !_tcscmp(fileinfo.cFileName, _T("..")))
			continue;

		_tcscpy_s(szTmpPath, strPath);         
		_tcscat_s(szTmpPath, _T("\\"));
		_tcscat_s(szTmpPath, fileinfo.cFileName);

		if (fileinfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			FileSearch(szTmpPath);
		}
		else
		{
			if (isPE(szTmpPath))
				GetPDB(szTmpPath);
		}
	} while (FindNextFile(hFind, &fileinfo));

	FindClose(hFind);

}

void GetPDB(CString szFilePath)
{
	TCHAR szText[200];
	LV_ITEM lvitem;
	lvitem.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE;
	lvitem.iSubItem = 0;
	lvitem.stateMask = LVIS_STATEIMAGEMASK;
	lvitem.state = INDEXTOSTATEIMAGEMASK(1);
	lvitem.iImage = 2;
	lvitem.iItem = pDlg->m_List.GetItemCount();
	lvitem.pszText = szText;
	int nIndex = pDlg->m_List.InsertItem(&lvitem);

	_stprintf_s(szText, _T("%d"), nIndex + 1);
	pDlg->m_List.SetItemText(nIndex, COLUMN_NO, szText);
	pDlg->m_List.SetItemData(nIndex, nIndex);

	pDlg->m_List.SetItemText(nIndex, COLUMN_SRCFILE, szFilePath);
	pDlg->m_List.SetItemText(nIndex, COLUMN_STAT, _T("Checking..."));

	pDlg->m_List.EnsureVisible(nIndex, FALSE);

	while (g_dwThreadRunningCount > pDlg->m_ThreadCount)
	{
		Sleep(20);
	} 

	InterlockedIncrement((LONG volatile *)&g_dwThreadRunningCount);
	HANDLE hProc = CreateThread(0, 0, SymCheckThread, (LPVOID)nIndex, 0, 0);
	CloseHandle(hProc);	
}
 
BOOL isPE(CString szFilePath)
{
	BOOL bRet = FALSE;
	DWORD dwAttr = GetFileAttributes(szFilePath);

	if (dwAttr == 0xFFFFFFFF)
		return bRet;

	HANDLE hFile = CreateFile(szFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, dwAttr, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		BYTE buffer[2] = { 0 };
		DWORD dwRead;
		ReadFile(hFile, buffer, (DWORD)2, &dwRead, 0);
		CloseHandle(hFile);  

		if (buffer[0] == 0x4D && buffer[1] == 0x5A)
			bRet = TRUE;
	}

	return bRet;
}

DWORD WINAPI SymCheckThread(LPVOID lp)
{
	int index = (int)lp; 

	CString strFile;
	CString strError = _T("Error occured.");
	BOOL bSuccess = FALSE;

	strFile = pDlg->m_List.GetItemText(index, COLUMN_SRCFILE);

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	TCHAR szRun[1024] = { 0 };
	TCHAR cmdline[1024] = { 0 };
	TCHAR szTmpFilePath[1024];
	TCHAR szFileTitle[260];
	
	GetFileTitle(strFile, szFileTitle, 250);
	GetTempPath(1024, szTmpFilePath);
	_tcscat_s(szTmpFilePath, szFileTitle);
	_tcscat_s(szTmpFilePath, _T(".log"));

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	_stprintf_s(szRun, _T("symchk.exe /r /if %s /s SRV*%s*%s"), (LPCTSTR)strFile, (LPCTSTR)pDlg->m_DestPath, szSymUrl);
	_stprintf_s(cmdline, _T("cmd.exe /c \"%s > %s\""), szRun, szTmpFilePath);

	CreateProcess(NULL, cmdline, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
	WaitForSingleObject(pi.hProcess, INFINITE);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	HANDLE hFile = CreateFile(szTmpFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		DWORD dwRead;
		DWORD dwSize = GetFileSize(hFile, NULL);
		char* szBuf = (char*)LocalAlloc(LPTR, dwSize);
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
				strError = "Success.";
			}
		}
	}


	if (bSuccess)
	{
		pDlg->m_SuccCount++;
		pDlg->m_List.SetItemText(index, COLUMN_STAT, _T("Success."));
	}
	else
	{
		pDlg->m_FailCount++;
		pDlg->m_List.SetItemText(index, COLUMN_STAT, strError);
	}

	DeleteFile(szTmpFilePath);

	InterlockedDecrement((LONG volatile *)&g_dwThreadRunningCount);
	return 0;
}

typedef struct _SortData{
	TCHAR	name[MAX_PATH];
}SortData, *PSortData;

static int CALLBACK GenSort(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	SortData* d1 = (SortData*)lParam1;
	SortData* d2 = (SortData*)lParam2;
	int result;

	if (_tcslen(d1->name) > _tcslen(d2->name))
		result = 1;
	else if (_tcslen(d1->name) < _tcslen(d2->name))
		result = -1;
	else
		result = _tcscmp(d1->name, d2->name);
	
	return result;
}

void CGetSymbolDlg::OnColumnclickList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	int count = m_List.GetItemCount();

	SortData *arStr = new SortData[count];
	for (int i = 0; i < count; i++)
	{
		_tcscpy_s(arStr[i].name, m_List.GetItemText(i, pNMListView->iSubItem));
		m_List.SetItemData(i, (LPARAM)(arStr + i));
	}
	m_List.SortItems(GenSort, 0);
	*pResult = 0;
}


char* strline(char* szStr, char* szLine)
{
	char* szEnd;

	if ((szEnd = strchr(szStr, '\n')) != NULL)
	{
		strncpy(szLine, szStr, (int)(szEnd - szStr));
		if (szLine[(int)(szEnd - szStr) - 1] == '\r')
			szLine[(int)(szEnd - szStr) - 1] = 0;
		return szEnd[1] != 0 ? szEnd + 1 : NULL;
	}
	else
	{
		strcpy(szLine, szStr);
		return NULL;
	}
}

void CGetSymbolDlg::OnBnClickedRadioMs()
{
	_tcscpy_s(szSymUrl, MS_SYMBOL_SERVER);
}


void CGetSymbolDlg::OnBnClickedRadioGoogle()
{
	_tcscpy_s(szSymUrl, GOOGLE_SYMBOL_SERVER);
}


void CGetSymbolDlg::OnBnClickedRadioMozilla()
{
	_tcscpy_s(szSymUrl, MOZILLA_SYMBOL_SERVER);
}


void CGetSymbolDlg::OnBnClickedRadioCitrix()
{
	_tcscpy_s(szSymUrl, CITRIX_SYMBOL_SERVER);
}