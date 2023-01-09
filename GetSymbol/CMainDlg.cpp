// CMainDlg.cpp : implementation file
//
#include "stdafx.h"


#include "GetSymbol.h"
#include "CMainDlg.h"
#include "afxdialogex.h"
#include "CUpdateDlg.h"

// CMainDlg dialog

IMPLEMENT_DYNAMIC(CMainDlg, CDialogEx)

CMainDlg::CMainDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MAIN_DLG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_bExitPending = FALSE;
}

CMainDlg::~CMainDlg()
{
}

void CMainDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB, m_Tab);
}


BEGIN_MESSAGE_MAP(CMainDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
END_MESSAGE_MAP()



DWORD WINAPI UpdateCheckThread(LPVOID lp) {
	ULONG status;
	TCHAR* lpStringBuffer = NULL;

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
		CUpdateDlg updateDlg;
		updateDlg.m_strNewVersion = lpszVersionInfo;

		if (updateDlg.DoModal() == IDOK) {
			TCHAR szModuleFileName[MAX_PATH];
			TCHAR szNewFileName[MAX_PATH];
			GetModuleFileName(NULL, szModuleFileName, MAX_PATH);
			wsprintf(szNewFileName, L"%s%s", szModuleFileName, L".tmp");

			DeleteFile(szNewFileName);
			MoveFile(szModuleFileName, szNewFileName);

			HANDLE hFile = CreateFile(szModuleFileName, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, NULL);

			if (hFile != INVALID_HANDLE_VALUE)
			{
				status = BeginDownload(lpszUpdateURL, hFile, NULL);
				CloseHandle(hFile);

				if (status != ERROR_SUCCESS) {
					DeleteFile(szModuleFileName);
					MoveFile(szNewFileName, szModuleFileName);
					AfxMessageBox(L"Update failed.");
					return status;
				}

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

BOOL CMainDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	TCHAR lpszAppNameVersion[MAX_PATH];
	wsprintf(lpszAppNameVersion, L"%s %s", APP_NAME, APP_VERSION);

	SetWindowText(lpszAppNameVersion);

	m_Tab.InsertItem(0, _T("Local Files"));
	m_Tab.InsertItem(1, _T("Online Windows Files"));
	m_Tab.Init();

	CreateThread(0, 0, UpdateCheckThread, 0, 0, 0);

	return TRUE;
}

void CMainDlg::Close() {

	TerminateThread(pGetSymbolDlg->m_hMainThread, 0);
	pGetSymbolDlg->m_hMainThread = NULL;

	pGetSymbolDlg->m_StatusBar.SetText(_T(" Closing... Waiting for current tasks are completed..."), 1, 0);
	pWinBinDlg->m_StatusBar.SetText(_T(" Closing... Waiting for current tasks are completed..."), 3, 0);
	while (TRUE)
	{
		if (pGetSymbolDlg->m_nThreadRunningCount == 0)
			break;
		Sleep(300);
	}


	while (TRUE)
	{
		if (pWinBinDlg->m_nThreadCount == 0)
			break;
		Sleep(300);
	}

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
// CMainDlg message handlers

BOOL CMainDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)
			return TRUE;
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}


void CMainDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	// TODO: Add your message handler code here and/or call default
	if (nID == SC_CLOSE)
	{
		if (!m_bExitPending) {
			pGetSymbolDlg->OnBnClickedBtnExit();
		}
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}
