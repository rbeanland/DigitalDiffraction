// MyDlg2.cpp : implementation file

// Enable double support


#include "stdafx.h"
#include "MyDlg2.h"
#include "NumEdit.h"
#include "boost/lexical_cast.hpp"
#include "time.h"





double diffclock(clock_t clock1,clock_t clock2)
{
	double diffticks=clock1-clock2;
	double diffms=(diffticks*1000)/CLOCKS_PER_SEC;
	return diffms;
} 


// MyDlg2 dialog
IMPLEMENT_DYNAMIC(MyDlg2, CDialog)

MyDlg2::MyDlg2(CWnd* pParent /*=NULL*/)
	: CDialog(MyDlg2::IDD, pParent)
{

}

MyDlg2::~MyDlg2()
{
}

void MyDlg2::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(MyDlg2, CDialog)
END_MESSAGE_MAP()


// MyDlg2 message handlers


// This functions is part of the WorkerClass.
// If you call Start() it will in turn run this function in a seperate Worker Thread, thus not locking up the UI.
void MyDlg2::DoWork()
{
}