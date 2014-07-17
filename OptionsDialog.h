#pragma once
#include "amsEdit.h"
#include "resource.h"

// COptionsDialog dialog

// This can be used to create an additional modal dialog to set further options without cluttering up the small dialog window pinned to DM.
// The easiest way is to pass all the initial dialog values from the parent class upon creating the dialog, and then send them all back to parent dialog using the OnOk(); function.


class COptionsDialog : public CDialog
{
	DECLARE_DYNAMIC(COptionsDialog)

public:
	//COptionsDialog(CWnd* pParent = NULL);   // standard constructor
	COptionsDialog(CWnd* pParent = NULL);   // standard constructor , add any input parameters to dialog here to initialise control values...
	virtual ~COptionsDialog();
	virtual void OnOK();

// Dialog Data
	enum { IDD = IDD_OPTIONS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	CWnd* mParent;
};
