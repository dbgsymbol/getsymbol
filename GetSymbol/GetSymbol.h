
// GetSymbol.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols
#include "GetSymbolDlg.h"

// CGetSymbolApp:
// See GetSymbol.cpp for the implementation of this class
//

extern CGetSymbolDlg* pDlg;
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