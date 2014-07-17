// OptionsDialog.cpp : implementation file
//

#include "stdafx.h"
#include "OptionsDialog.h"
// #include "afxdialogex.h"
#include "MyDlg1.h"


// COptionsDialog dialog

IMPLEMENT_DYNAMIC(COptionsDialog, CDialog)

//COptionsDialog::COptionsDialog(CWnd* pParent /*=NULL*/)
COptionsDialog::COptionsDialog(CWnd* pParent)
	: CDialog(COptionsDialog::IDD, pParent)
{
	// Set values of controls with parameters passed in via constructor on dialog creation.
}

COptionsDialog::~COptionsDialog()
{
}

void COptionsDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

void COptionsDialog::OnOK() 
{
   // TODO: Add extra validation here
   
	/*
   // Ensure that your UI got the necessary input 
   // from the user before closing the dialog. The 
   // default OnOK will close this.
   if ( m_nMyValue == 0 ) // Is a particular field still empty?
   {
      AfxMessageBox("Please enter a value for MyValue");
      return; // Inform the user that he can't close the dialog without
              // entering the necessary values and don't close the 
              // dialog.
   }
   */

	UpdateData(true);

	// Call function in parent dialog by casting CWnd* to the parent classes type
	// to pass back values of all controls and options specified in this dialog.
	CDialog::OnOK(); // This will close the dialog and DoModal will return.
}


BEGIN_MESSAGE_MAP(COptionsDialog, CDialog)
END_MESSAGE_MAP()


// COptionsDialog message handlers
