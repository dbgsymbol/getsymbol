 
// GetSymbol.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "GetSymbol.h"
#include "CMainDlg.h"
#include "GetSymbolDlg.h"
#include "CWinBinDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CMainDlg* pMainDlg;
CGetSymbolDlg* pGetSymbolDlg;
CWinBinDlg* pWinBinDlg;

TCHAR szExePath[1024];

// CGetSymbolApp

BEGIN_MESSAGE_MAP(CGetSymbolApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CGetSymbolApp construction

CGetSymbolApp::CGetSymbolApp()
{
	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CGetSymbolApp object

CGetSymbolApp theApp;


// CGetSymbolApp initialization

BOOL CGetSymbolApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	if (GetLastError() == ERROR_ALREADY_EXISTS)
		return FALSE;

	TCHAR szResFolder[30] = _T("HEX64");
	DWORD nDbgHelp = IDR_DBGHELP;
	DWORD nSymbolCheck = IDR_SYMBOLCHECK;
	DWORD nSymChk = IDR_SYMCHK;
	DWORD nDbgEng = IDR_DBGENG;
	DWORD nMsvcrt = IDR_MSVCRT;
	DWORD nSymSrv = IDR_SYMSRV;
	DWORD nSymYes = IDR_SYMSRV_YES;
	DWORD nDbgModel = IDR_DBGMODEL;

	HGLOBAL hGlobal;
	int dwLen;

	GetModuleFileName(NULL, szExePath, MAX_PATH);

	TCHAR tmpFileName[MAX_PATH];
	wsprintf(tmpFileName, L"%s%s", szExePath, L".tmp");

	if (PathFileExists(tmpFileName)) {
		MessageBoxW(0, L"Update installed successfully.", APP_NAME, MB_OK | MB_ICONINFORMATION);
		DeleteFile(tmpFileName);
	}


	*(_tcsrchr(szExePath, _T('\\'))) = 0;

	HINSTANCE hInst = (HINSTANCE)GetModuleHandle(NULL);
	
	hGlobal = LoadResource(hInst, FindResource(hInst, MAKEINTRESOURCE(nDbgHelp), szResFolder));
	dwLen = SizeofResource(hInst, FindResource(hInst, MAKEINTRESOURCE(nDbgHelp), szResFolder));

	GlobalUnlock(hGlobal);
	LPBYTE lpBuf = (LPBYTE)hGlobal;
	
	DWORD dwWrite;
	TCHAR tmpPath[MAX_PATH];
	wsprintf(tmpPath, _T("%s\\DbgHelp.dll"), szExePath);


	HANDLE hFile = CreateFile(tmpPath, GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		WriteFile(hFile, lpBuf, dwLen, &dwWrite, NULL);
		CloseHandle(hFile);
	}

	GlobalLock(hGlobal);

	////////////////////////

	hGlobal = LoadResource(hInst, FindResource(hInst, MAKEINTRESOURCE(nSymbolCheck), szResFolder));
	dwLen = SizeofResource(hInst, FindResource(hInst, MAKEINTRESOURCE(nSymbolCheck), szResFolder));

	GlobalUnlock(hGlobal);
	lpBuf = (LPBYTE)hGlobal;

	wsprintf(tmpPath, _T("%s\\SymbolCheck.dll"), szExePath);

    hFile = CreateFile(tmpPath, GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		WriteFile(hFile, lpBuf, dwLen, &dwWrite, NULL);
		CloseHandle(hFile);
	}
	
	GlobalLock(hGlobal);

	/////////////////////////
	
	hGlobal = LoadResource(hInst, FindResource(hInst, MAKEINTRESOURCE(nSymChk), szResFolder));
	dwLen = SizeofResource(hInst, FindResource(hInst, MAKEINTRESOURCE(nSymChk), szResFolder));

	GlobalUnlock(hGlobal);
	lpBuf = (LPBYTE)hGlobal;

	wsprintf(tmpPath, _T("%s\\symchk.exe"), szExePath);

	hFile = CreateFile(tmpPath, GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		WriteFile(hFile, lpBuf, dwLen, &dwWrite, NULL);
		CloseHandle(hFile);
	}

	GlobalLock(hGlobal);

	/////////////////////////

	hGlobal = LoadResource(hInst, FindResource(hInst, MAKEINTRESOURCE(nDbgEng), szResFolder));
	dwLen = SizeofResource(hInst, FindResource(hInst, MAKEINTRESOURCE(nDbgEng), szResFolder));

	GlobalUnlock(hGlobal);
	lpBuf = (LPBYTE)hGlobal;

	wsprintf(tmpPath, _T("%s\\dbgeng.dll"), szExePath);

	hFile = CreateFile(tmpPath, GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		WriteFile(hFile, lpBuf, dwLen, &dwWrite, NULL);
		CloseHandle(hFile);
	}

	GlobalLock(hGlobal);

	/////////////////////////

	hGlobal = LoadResource(hInst, FindResource(hInst, MAKEINTRESOURCE(nDbgModel), szResFolder));
	dwLen = SizeofResource(hInst, FindResource(hInst, MAKEINTRESOURCE(nDbgModel), szResFolder));

	GlobalUnlock(hGlobal);
	lpBuf = (LPBYTE)hGlobal;

	wsprintf(tmpPath, _T("%s\\DbgModel.dll"), szExePath);

	hFile = CreateFile(tmpPath, GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		WriteFile(hFile, lpBuf, dwLen, &dwWrite, NULL);
		CloseHandle(hFile);
	}

	GlobalLock(hGlobal);

	/////////////////////////

	hGlobal = LoadResource(hInst, FindResource(hInst, MAKEINTRESOURCE(nMsvcrt), szResFolder));
	dwLen = SizeofResource(hInst, FindResource(hInst, MAKEINTRESOURCE(nMsvcrt), szResFolder));

	GlobalUnlock(hGlobal);
	lpBuf = (LPBYTE)hGlobal;

	wsprintf(tmpPath, _T("%s\\msvcrt.dll"), szExePath);

	hFile = CreateFile(tmpPath, GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		WriteFile(hFile, lpBuf, dwLen, &dwWrite, NULL);
		CloseHandle(hFile);
	}

	GlobalLock(hGlobal);

	/////////////////////////

	hGlobal = LoadResource(hInst, FindResource(hInst, MAKEINTRESOURCE(nSymSrv), szResFolder));
	dwLen = SizeofResource(hInst, FindResource(hInst, MAKEINTRESOURCE(nSymSrv), szResFolder));

	GlobalUnlock(hGlobal);
	lpBuf = (LPBYTE)hGlobal;

	wsprintf(tmpPath, _T("%s\\symsrv.dll"), szExePath);

	hFile = CreateFile(tmpPath, GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		WriteFile(hFile, lpBuf, dwLen, &dwWrite, NULL);
		CloseHandle(hFile);
	}

	GlobalLock(hGlobal);

	/////////////////////////

	hGlobal = LoadResource(hInst, FindResource(hInst, MAKEINTRESOURCE(nSymYes), szResFolder));
	dwLen = SizeofResource(hInst, FindResource(hInst, MAKEINTRESOURCE(nSymYes), szResFolder));

	GlobalUnlock(hGlobal);
	lpBuf = (LPBYTE)hGlobal;

	wsprintf(tmpPath, _T("%s\\symsrv.yes"), szExePath);

	hFile = CreateFile(tmpPath, GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		WriteFile(hFile, lpBuf, dwLen, &dwWrite, NULL);
		CloseHandle(hFile);
	}

	GlobalLock(hGlobal);

	SetCurrentDirectory(szExePath);
	
	AfxEnableControlContainer();

	// Create the shell manager, in case the dialog contains
	// any shell tree view or shell list view controls.
	CShellManager *pShellManager = new CShellManager;

	// Activate "Windows Native" visual manager for enabling themes in MFC controls
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	CMainDlg dlg;
	m_pMainWnd = pMainDlg = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "Warning: dialog creation failed, so application is terminating unexpectedly.\n");
		TRACE(traceAppMsg, 0, "Warning: if you are using MFC controls on the dialog, you cannot #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS.\n");
	}

	// Delete the shell manager created above.
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

