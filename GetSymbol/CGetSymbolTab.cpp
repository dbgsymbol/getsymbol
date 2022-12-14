// CGetSymbolTab.cpp : implementation file
//

#include "stdafx.h"
#include "GetSymbol.h"
#include "CGetSymbolTab.h"
#include "CWinBinDlg.h"


// CGetSymbolTab

IMPLEMENT_DYNAMIC(CGetSymbolTab, CTabCtrl)

CGetSymbolTab::CGetSymbolTab()
{
	::ZeroMemory(&m_tabPages, sizeof(m_tabPages));

	pGetSymbolDlg = new CGetSymbolDlg;
	pWinBinDlg = new CWinBinDlg;

	m_tabPages[0] = pGetSymbolDlg;
	m_tabPages[1] = pWinBinDlg;

	m_nNumberOfPages = 2;
}

CGetSymbolTab::~CGetSymbolTab()
{
	for (int nCount = 0; nCount < m_nNumberOfPages; nCount++) {
		delete m_tabPages[nCount];
	}
}

void CGetSymbolTab::Init()
{
	m_tabCurrent = 0;

	m_tabPages[0]->Create(IDD_GETSYMBOL_DIALOG, this);
	m_tabPages[1]->Create(IDD_WIN_BIN_DLG, this);

	m_tabPages[0]->ShowWindow(SW_SHOW);
	m_tabPages[1]->ShowWindow(SW_HIDE);

	SetRectangle();
}

void CGetSymbolTab::SetRectangle()
{
	CRect tabRect, itemRect;
	int nX, nY, nXc, nYc;

	GetClientRect(&tabRect);
	GetItemRect(0, &itemRect);

	nX = itemRect.left;
	nY = itemRect.bottom + 1;
	nXc = tabRect.right - itemRect.left - 1;
	nYc = tabRect.bottom - nY - 1;

	m_tabPages[0]->SetWindowPos(&wndTop, nX, nY, nXc, nYc, SWP_SHOWWINDOW);
	for (int nCount = 1; nCount < m_nNumberOfPages; nCount++) {
		m_tabPages[nCount]->SetWindowPos(&wndTop, nX, nY, nXc, nYc, SWP_HIDEWINDOW);
	}
}

BEGIN_MESSAGE_MAP(CGetSymbolTab, CTabCtrl)
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()



// CGetSymbolTab message handlers


void CGetSymbolTab::OnLButtonDown(UINT nFlags, CPoint point)
{
	CTabCtrl::OnLButtonDown(nFlags, point);
	// TODO: Add your message handler code here and/or call default
	if (m_tabCurrent != GetCurFocus()) {
		m_tabPages[m_tabCurrent]->ShowWindow(SW_HIDE);
		m_tabCurrent = GetCurFocus();
		m_tabPages[m_tabCurrent]->ShowWindow(SW_SHOW);
		m_tabPages[m_tabCurrent]->SetFocus();
	}
	
}

