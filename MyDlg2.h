#pragma once

#include "TemplateMFCDialogPlugIn.h"
#include "afxwin.h"
#include "standardfunctions.h"
#include "numedit.h"
#include "CL/OpenCl.h"
#include "clAmdFft.h"
#include <complex>
#include "WorkerClass.h"



// MyDlg2 dialog
class MyDlg2 : public CDialog, public WorkerClass
{
	DECLARE_DYNAMIC(MyDlg2)

public:
	MyDlg2(CWnd* pParent = NULL);   // standard constructor
	virtual ~MyDlg2();

	// Dialog Data
	enum { IDD = IDD_DIALOG2 };



protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:	
	// This functions is part of the WorkerClass.
	// If you call Start() it will in turn run this function in a seperate Worker Thread, thus not locking up the UI.
	virtual void DoWork();
	

};
