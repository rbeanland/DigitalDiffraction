#pragma once

#include "TemplateMFCDialogPlugIn.h"
#include "afxwin.h"
#include "standardfunctions.h"
#include "numedit.h"
#include <complex>
#include "WorkerClass.h"
#include "DED_Process.h"



class Process;

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
	void InititalLoad();//Load settings when DigitalMicrograph is first opened

	afx_msg void OnBnClickedButton1();
	BOOL want_option_1;
	afx_msg void OnBnClickedButton3();// Load Settings button
	afx_msg void OnBnClickedButton2();//Save settings button
	BOOL create_ask_cropped_bool_input;
	BOOL show_dlacbed_stack;
	BOOL enable_colour_change;
	BOOL print_f_and_fimgx;
	BOOL print_x_y_in_lacbed;
	BOOL display_cluster_gvectors;
	BOOL display_mag_theta;
	BOOL display_y_yy_vy;
	BOOL display_x_xx_vx;
	BOOL print_g1x_g1y;
	BOOL display_avg_cc;
	BOOL print_rr_tinc;
	afx_msg void OnBnClickedCheck17();// Select all check box
	BOOL process_select_all;
	BOOL display_ysh_input;
	BOOL print_xtpx_input;
	BOOL print_xshpx_input;
	BOOL display_calibration_input;
	BOOL kunprocessed_selected;
	BOOL kdarksubtracted_selected;
	BOOL kgainnormalized_selected;
	BOOL kmaxprocessing_selected;
	afx_msg void OnBnClickedCheck19();//kUnprocessed check box
	afx_msg void OnBnClickedCheck20();//kDarkSubtracted check box
	afx_msg void OnBnClickedCheck21();//kGainNormalized check box
	afx_msg void OnBnClickedCheck22();//kMaxProcessing chgeck box
	BOOL print_shift_tilt_input;
	BOOL print_time_input;
};
