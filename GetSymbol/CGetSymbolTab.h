#pragma once


// CGetSymbolTab

class CGetSymbolTab : public CTabCtrl
{
	DECLARE_DYNAMIC(CGetSymbolTab)
	
	CDialog* m_tabPages[2];
	int m_tabCurrent;
	int m_nNumberOfPages;

public:
	void Init();
	void SetRectangle();

public:
	CGetSymbolTab();
	virtual ~CGetSymbolTab();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
//	afx_msg void OnSetFocus(CWnd* pOldWnd);
};


