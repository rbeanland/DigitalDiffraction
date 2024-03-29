// DMDialog.cpp : implementation file

// This is the base dialog for the plugin.
// It only contains a Tab Control which then has a seperate dialog on each Tab.

#include "stdafx.h"
#include "TemplateMFCDialogPlugIn.h"
#include "DMPluginStubs.h"
#include "DMDialog.h"


IMPLEMENT_DYNCREATE(CDMDialog, CDialog)
CDMDialog::CDMDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CDMDialog::IDD, pParent)
{

}

CDMDialog::~CDMDialog()
{
}

void CDMDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB1, m_tbCtrl);

}


BEGIN_MESSAGE_MAP(CDMDialog, CDialog)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, &CDMDialog::OnTcnSelchangeTab1)
END_MESSAGE_MAP()


// CDMDialog message handlers

BOOL CDMDialog::Create(UINT templateID, CWnd* pParentWnd)
{
	// TODO: Add your specialized code here and/or call the base clase
	return CDialog::Create(IDD, pParentWnd);
}


// Set up the different tabs on loading of dialog
BOOL CDMDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	 m_tbCtrl.InitDialogs();
	
	 // Tab Names
	 m_tbCtrl.InsertItem(0,"DED");
	 m_tbCtrl.InsertItem(1,"Settings");

	 m_tbCtrl.ActivateTabDialogs();


	// TODO:  Add extra initialization here
	ModifyStyle(0, WS_GROUP | WS_TABSTOP);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CDMDialog::OnPaint()
{
	/*
	CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here
	// Do not call CDialog::OnPaint() for painting messages
	dc.SelectStockObject(WHITE_BRUSH);
	dc.DrawText("Bla Bla Bla", CRect(0, 0, 50, 50), 0); 

	cide << __FUNCTION__ << std::endl;
	*/
	CDialog::OnPaint();
}

BOOL CDMDialog::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default

	return CDialog::OnEraseBkgnd(pDC);
}

std::ostream& operator<<(std::ostream& os, const CRect& rect)
{
	return os << "Rect=(" << rect.TopLeft().x << ", " << rect.TopLeft().y << ", " << rect.BottomRight().x << ", " << rect.BottomRight().y << ")";
}



void CDMDialog::OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	*pResult = 0;
}
