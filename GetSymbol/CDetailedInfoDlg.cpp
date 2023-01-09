// CDetailedInfoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "GetSymbol.h"
#include "CDetailedInfoDlg.h"
#include "afxdialogex.h"


// CDetailedInfoDlg dialog

IMPLEMENT_DYNAMIC(CDetailedInfoDlg, CDialogEx)

CDetailedInfoDlg::CDetailedInfoDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DETAILED_INFO_DLG, pParent)
	, m_strDetailedInfo(_T(""))
{

}

CDetailedInfoDlg::~CDetailedInfoDlg()
{
}

void CDetailedInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_UPD_INFO, m_strDetailedInfo);
}


BEGIN_MESSAGE_MAP(CDetailedInfoDlg, CDialogEx)
END_MESSAGE_MAP()


// CDetailedInfoDlg message handlers
