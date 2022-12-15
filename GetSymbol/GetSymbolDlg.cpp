
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


TCHAR szSymUrl[MAX_PATH];

DWORD WINAPI StartProc(LPVOID lp);
DWORD WINAPI ExitProc(LPVOID lp);
void FileSearch(CString strPath);
void GetPDB(CString szFilePath);
BOOL isPE(CString szFilePath);
DWORD WINAPI SymCheckThread(LPVOID lp);

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
	m_nThreadRunningCount = 0;
	m_hMainThread = NULL;
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

	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST, &CGetSymbolDlg::OnColumnclickList)
	ON_BN_CLICKED(IDC_RADIO_MS, &CGetSymbolDlg::OnBnClickedRadioMs)
	ON_BN_CLICKED(IDC_RADIO_GOOGLE, &CGetSymbolDlg::OnBnClickedRadioGoogle)
	ON_BN_CLICKED(IDC_RADIO_MOZILLA, &CGetSymbolDlg::OnBnClickedRadioMozilla)
	ON_BN_CLICKED(IDC_RADIO_CITRIX, &CGetSymbolDlg::OnBnClickedRadioCitrix)
END_MESSAGE_MAP()


// CGetSymbolDlg message handlers

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
	
	m_List.SetExtendedStyle(m_List.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
	m_List.InsertColumn(COLUMN_NO, _T("No"), LVCFMT_LEFT, 60);
	m_List.InsertColumn(COLUMN_SRCFILE, _T("Source File Path"), LVCFMT_LEFT, 340);
	m_List.InsertColumn(COLUMN_STAT, _T("Download Status"), LVCFMT_LEFT, 290);

	CheckRadioButton(IDC_RADIO_MS, IDC_RADIO_CITRIX, IDC_RADIO_MS);
	_tcscpy_s(szSymUrl, MS_SYMBOL_SERVER);

	int widths[2];
	m_StatusBar.Create(WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), this, IDC_STATUSBAR);

	widths[0] = 100;
	widths[1] = 700;

	m_StatusBar.SetParts(2, widths);

	m_StatusBar.SetText(_T(" Status "), 0, 0);
	m_StatusBar.SetText(_T(" Ready"), 1, 0);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CGetSymbolDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
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
	if (pMainDlg->m_bExitPending) {
		return;
	}
	
	if (AfxMessageBox(L"Really want to exit?", MB_YESNO) == IDYES) {
		pMainDlg->m_bExitPending = TRUE;
		CreateThread(NULL, 0, ExitProc, NULL, 0, NULL);
	}
}

DWORD WINAPI ExitProc(LPVOID lp) {
	pMainDlg->Close();
	return 0;
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
	if (m_bBusy) {
		if (!m_bPaused) {
			m_bPaused = TRUE;
			SuspendThread(m_hMainThread);
			pGetSymbolDlg->GetDlgItem(ID_BTN_START)->SetWindowTextW(L"Resume");
			pGetSymbolDlg->m_StatusBar.SetText(_T(" Paused"), 1, 0);
		}
		else {
			m_bPaused = FALSE;
			ResumeThread(m_hMainThread);
			pGetSymbolDlg->GetDlgItem(ID_BTN_START)->SetWindowTextW(L"Pause");
			pGetSymbolDlg->m_StatusBar.SetText(_T(" Downloading..."), 1, 0);
		}
		return;
	}
		

	UpdateData(TRUE);
	m_List.DeleteAllItems();

	m_hMainThread = CreateThread(NULL, 0, StartProc, NULL, 0, NULL);


}

DWORD WINAPI StartProc(LPVOID lp)
{
	pGetSymbolDlg->m_bBusy = TRUE;
	pGetSymbolDlg->m_bPaused = FALSE;
	pGetSymbolDlg->GetDlgItem(ID_BTN_START)->SetWindowTextW(L"Pause");
	pGetSymbolDlg->m_StatusBar.SetText(_T(" Downloading..."), 1, 0);

	pGetSymbolDlg->m_SuccCount = pGetSymbolDlg->m_FailCount = pGetSymbolDlg->m_nThreadRunningCount = 0;

	FileSearch(pGetSymbolDlg->m_SrcPath);

	while (TRUE)
	{
		if (pGetSymbolDlg->m_nThreadRunningCount == 0)
			break;
		Sleep(300);
	}

	DWORD dwTotal = pGetSymbolDlg->m_SuccCount + pGetSymbolDlg->m_FailCount;
	CString str;
	str.Format(_T("Symbol Download Finished! \r\n\r\n        Success  :  [%d / %d] \n        Failed     :  [%d / %d]"), pGetSymbolDlg->m_SuccCount, dwTotal, pGetSymbolDlg->m_FailCount, dwTotal);
	AfxMessageBox(str);

	pGetSymbolDlg->m_bBusy = FALSE;
	pGetSymbolDlg->GetDlgItem(ID_BTN_START)->SetWindowTextW(L"Start");
	pGetSymbolDlg->m_StatusBar.SetText(_T(" Download completed"), 1, 0);

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
	while (pGetSymbolDlg->m_nThreadRunningCount > pGetSymbolDlg->m_ThreadCount)
	{
		Sleep(20);
	}

	TCHAR szText[200];
	LV_ITEM lvitem;
	lvitem.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE;
	lvitem.iSubItem = 0;
	lvitem.stateMask = LVIS_STATEIMAGEMASK;
	lvitem.state = INDEXTOSTATEIMAGEMASK(1);
	lvitem.iImage = 2;
	lvitem.iItem = pGetSymbolDlg->m_List.GetItemCount();
	lvitem.pszText = szText;
	int nIndex = pGetSymbolDlg->m_List.InsertItem(&lvitem);

	_stprintf_s(szText, _T("%d"), nIndex + 1);
	pGetSymbolDlg->m_List.SetItemText(nIndex, COLUMN_NO, szText);
	pGetSymbolDlg->m_List.SetItemData(nIndex, nIndex);

	pGetSymbolDlg->m_List.SetItemText(nIndex, COLUMN_SRCFILE, szFilePath);
	pGetSymbolDlg->m_List.EnsureVisible(nIndex, FALSE);

	InterlockedIncrement((LONG volatile *)&pGetSymbolDlg->m_nThreadRunningCount);
	pGetSymbolDlg->m_List.SetItemText(nIndex, COLUMN_STAT, _T("Checking..."));
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

	strFile = pGetSymbolDlg->m_List.GetItemText(index, COLUMN_SRCFILE);

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

	_stprintf_s(szRun, _T("symchk.exe /r /if %s /s SRV*%s*%s"), (LPCTSTR)strFile, (LPCTSTR)pGetSymbolDlg->m_DestPath, szSymUrl);
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
				strError = "Success.";
			}
		}
		LocalFree(szBuf);
	}


	if (bSuccess)
	{
		pGetSymbolDlg->m_SuccCount++;
		pGetSymbolDlg->m_List.SetItemText(index, COLUMN_STAT, _T("Success."));
	}
	else
	{
		pGetSymbolDlg->m_FailCount++;
		pGetSymbolDlg->m_List.SetItemText(index, COLUMN_STAT, strError);
	}

	DeleteFile(szTmpFilePath);

	InterlockedDecrement((LONG volatile *)&pGetSymbolDlg->m_nThreadRunningCount);
	return 0;
}

typedef struct _SortData{
	TCHAR	name[4096];
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
	delete[] arStr;
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
