#pragma once
#include "resource.h"
#include "numedit.h"
#include "c:\program files (x86)\microsoft visual studio 9.0\vc\atlmfc\include\afxwin.h"
#include <complex>
#include "WorkerClass.h"
#include "DED_TiltCalibration.h"
#include "DED_Collect.h"
#include "DED_Process.h"
#include "C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\atlmfc\include\afxcmn.h"

class Calibrate;
class Collect;
class Process;

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
	
	double expo_input, sleeptime_input, tInc_input;
	int binning_input;
	Calibrate* DEDCalibrate;
	Collect* DEDCollect;
	Process* DEDProcess;
	bool tiltcalibration;
	bool collect;
	bool process;
	long TiltX0, TiltY0, ShiftX0, ShiftY0;
	long TagTiltX0, TagTiltY0, TagShiftX0, TagShiftY0;
	std::string curr_shift, curr_tilt;
	std::string tag_shift, tag_tilt;
	DigitalMicrograph::TagGroup Persistent;
	bool set_shift_tilt_tag;

	// This functions is part of the WorkerClass.
	// If you call Start() it will in turn run this function in a seperate Worker Thread, thus not locking up the UI.
	virtual void DoWork();
	afx_msg void OnBnClickedButton1();//Tilt calibration button
	afx_msg void OnBnClickedButton2();//Collect button
	afx_msg void OnBnClickedButton3();//Draw cross on front image button
	afx_msg void OnBnClickedButton4();//Process button
	afx_msg void OnEnChangeEdit3();
	afx_msg void OnEnChangeEdit8();
	afx_msg void OnBnClickedButton5();//Grab shift and tilt button
	afx_msg void OnBnClickedButton6();//"Get Shift and Tilt Values from Global Tags" button
	afx_msg void OnBnClickedButton7();//"Set Shift and Tilt to Grabbed Values" button
	afx_msg void OnBnClickedButton8();//"Set Shift and Tilt to Global Tag Values" button
	afx_msg void OnNMCustomdrawProgress2(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButton9();// The reset button, this re-enables the buttons on the plugin in case one part fails
	afx_msg void OnBnClickedButton10();// "Save current Shift and Tilt to Global Tags" button
	CProgressCtrl progress_ctrl;
private:
	CString str_binning_input;
	CString str_expo_input;
	CString str_sleeptime_input;
	CString tinc_input_string;
	CString Grabbed_Shift_string;
	CString Grabbed_Tilt_String;
	CString Tag_Shift_Display_String;
	CString Tag_Tilt_Display_String;
};
