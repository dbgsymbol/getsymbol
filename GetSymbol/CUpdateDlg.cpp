// CUpdateDlg.cpp : implementation file
//

#include "stdafx.h"
#include "GetSymbol.h"
#include "CUpdateDlg.h"
#include "afxdialogex.h"


// CUpdateDlg dialog

IMPLEMENT_DYNAMIC(CUpdateDlg, CDialogEx)

CUpdateDlg::CUpdateDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_UPDATE_DLG, pParent)
	, m_strNewVersion(_T(""))
{

}

CUpdateDlg::~CUpdateDlg()
{
}

void CUpdateDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_APPVERSION, m_strNewVersion);
}


BEGIN_MESSAGE_MAP(CUpdateDlg, CDialogEx)
END_MESSAGE_MAP()


// CUpdateDlg message handlers
