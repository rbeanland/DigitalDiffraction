#pragma once
#include "resource.h"
#include "numedit.h"
#include "c:\program files (x86)\microsoft visual studio 9.0\vc\atlmfc\include\afxwin.h"
#include "CL/OpenCl.h"
#include "clAmdFft.h"
#include <complex>
#include "clKernel.h"
#include "clFourier.h"
#include "WorkerClass.h"
#include "DED_TiltCalibration.h"
#include "DED_Collect.h"


class Calibrate;
class Collect;


// MyDlg1 dialog
class MyDlg1 : public CDialog, public WorkerClass
{
	DECLARE_DYNAMIC(MyDlg1)

public:
	MyDlg1(CWnd* pParent = NULL);   // standard constructor
	virtual ~MyDlg1();

	// Dialog Data
	enum { IDD = IDD_DIALOG1 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	virtual BOOL PreTranslateMessage(MSG* pMsg);

public:
	// CL Stuff
	bool OpenCLAvailable;
	cl_command_queue cmdQueue;
	cl_context context;
	// Use this to check status after every API call
	cl_int status;
	cl_uint numDevices;
	cl_device_id* devices;
	clDevice* cldev;
	clQueue* clq;

	double expo_input, sleeptime_input;
	int binning_input;

	Calibrate* DEDCalibrate;
	Collect* DEDCollect;
	
	
	
	bool tiltcalibration;
	bool collect;

	// This functions is part of the WorkerClass.
	// If you call Start() it will in turn run this function in a seperate Worker Thread, thus not locking up the UI.
	virtual void DoWork();
	afx_msg void OnBnClickedButton1();
private:
	CString str_binning_input;
	CString str_expo_input;
	CString str_sleeptime_input;
public:
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton3();
};
