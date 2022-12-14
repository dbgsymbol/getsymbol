
// GetSymbol.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols
#include "CMainDlg.h"
#include "GetSymbolDlg.h"
#include "CWinBinDlg.h"
// CGetSymbolApp:
// See GetSymbol.cpp for the implementation of this class
//

#define MS_SYMBOL_SERVER			_T("http://msdl.microsoft.com/download/symbols")
#define MOZILLA_SYMBOL_SERVER		_T("http://symbols.mozilla.org/firefox")
#define GOOGLE_SYMBOL_SERVER		_T("https://chromium-browser-symsrv.commondatastorage.googleapis.com/tmpq4t67i74")
#define CITRIX_SYMBOL_SERVER		_T("http://ctxsym.citrix.com/symbols")

extern CMainDlg* pMainDlg;
extern CGetSymbolDlg* pGetSymbolDlg;
extern CWinBinDlg* pWinBinDlg;

extern TCHAR szExePath[1024];

class CGetSymbolApp : public CWinApp
{
public:
	CGetSymbolApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CGetSymbolApp theApp;