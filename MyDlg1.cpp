// MyDlg1.cpp : implementation file
//
#pragma OPENCL EXTENSION cl_khr_fp64 : enable

#include "stdafx.h"
#include "MyDlg1.h"
#include "boost/lexical_cast.hpp"
#include <sstream>
#include "DMPluginCamera.h"
#include "standardfunctions.h"
//#include "CLKernels.h"
//#include "clFourier.h"
#include "utility.h"
#include "OptionsDialog.h"
#include <iostream>

#include "ExtraGatan.h"



// GPU usually platform 0 dev 0, cpu is 0,1 with AMD Gfx or 1,0 with nVidia, 
// cant see Gfx over Remote Desktop though so CPU 0,0


// MyDlg1 dialog

IMPLEMENT_DYNAMIC(MyDlg1, CDialog)

// This constructor runs as soon as the plugin is loaded when this tab is generated. This looks in the global tags to find information on which OpenCL device/platform should be used.
// It then generates the Command Queue and Context ready to run kernels. If these tags are not present in leaves information in results window about how to set this up.
MyDlg1::MyDlg1(CWnd* pParent /*=NULL*/)
	: CDialog(MyDlg1::IDD, pParent)
	, str_binning_input(_T("4"))//setting the default values for the plugin
	, str_expo_input(_T("0.04"))
	, str_sleeptime_input(_T("0.02"))
	, tinc_input_string(_T("0.8"))
	, Grabbed_Shift_string(_T("00000, 00000"))
	, Grabbed_Tilt_String(_T("00000, 00000"))
	, Tag_Shift_Display_String(_T("00000, 00000"))
	, Tag_Tilt_Display_String(_T("00000, 00000"))
	//, current_process_string(_T(""))
{
// TODO: Check for persistent tags to get platform and devnumber
	//progress_ctrl.SetRange(0, 20);
	DEDCalibrate = new Calibrate;
	DEDCollect = new Collect;
	DEDProcess = new Process;
	tiltcalibration = false;
	collect = false;
	process = false;

	set_shift_tilt_tag = false;
	
	DM::TagGroup PersistentTags = DM::GetPersistentTagGroup();
	DM::String clPlatformTag;
	DM::String clDeviceTag;

	DM::TagGroupGetTagAsString(PersistentTags,"OpenCL:Platform",clPlatformTag);
	DM::TagGroupGetTagAsString(PersistentTags,"OpenCL:Device",clDeviceTag);

	int platformnumber;
	int devicenumber;
	try
	{
		platformnumber = boost::lexical_cast<int>(clPlatformTag);
		devicenumber = boost::lexical_cast<int>(clDeviceTag);
	}
	catch(boost::bad_lexical_cast e)
	{
		Utility::SetResultWindow("Open CL Device and Platform incorrectly set in global tags");
		Utility::SetResultWindow("Set Platform and Device 0,1,2 etc in tags OpenCl:Platform and OpenCL:Device");
		Utility::SetResultWindow("Run clinfo at command prompt to find available devices and platforms");
		return;
	}

	
}

MyDlg1::~MyDlg1()
{
}

void MyDlg1::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_CBString(pDX, IDC_COMBO1, str_binning_input);
	DDX_Text(pDX, IDC_EDIT1, str_expo_input);
	DDX_Text(pDX, IDC_EDIT2, str_sleeptime_input);
	DDX_Text(pDX, IDC_EDIT3, tinc_input_string);
	DDX_Text(pDX, IDC_EDIT8, Grabbed_Shift_string);
	DDX_Text(pDX, IDC_EDIT9, Grabbed_Tilt_String);
	DDX_Text(pDX, IDC_EDIT6, Tag_Shift_Display_String);
	DDX_Text(pDX, IDC_EDIT11, Tag_Tilt_Display_String);
	//DDX_Control(pDX, IDC_PROGRESS1, m_progressBar);
	DDX_Control(pDX, IDC_PROGRESS2, progress_ctrl);
	//DDX_Text(pDX, IDC_EDIT12, current_process_string);
}


BEGIN_MESSAGE_MAP(MyDlg1, CDialog)
	ON_BN_CLICKED(IDC_BUTTON1, &MyDlg1::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &MyDlg1::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &MyDlg1::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON4, &MyDlg1::OnBnClickedButton4)
	ON_EN_CHANGE(IDC_EDIT3, &MyDlg1::OnEnChangeEdit3)
	ON_EN_CHANGE(IDC_EDIT8, &MyDlg1::OnEnChangeEdit8)
	ON_BN_CLICKED(IDC_BUTTON5, &MyDlg1::OnBnClickedButton5)
	ON_BN_CLICKED(IDC_BUTTON6, &MyDlg1::OnBnClickedButton6)
	ON_BN_CLICKED(IDC_BUTTON7, &MyDlg1::OnBnClickedButton7)
	ON_BN_CLICKED(IDC_BUTTON8, &MyDlg1::OnBnClickedButton8)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_PROGRESS2, &MyDlg1::OnNMCustomdrawProgress2)
	ON_BN_CLICKED(IDC_BUTTON9, &MyDlg1::OnBnClickedButton9)
	ON_BN_CLICKED(IDC_BUTTON10, &MyDlg1::OnBnClickedButton10)
END_MESSAGE_MAP()

BOOL MyDlg1::PreTranslateMessage(MSG* pMsg)
{
    if (pMsg->message == WM_KEYDOWN)
    {
        if ((pMsg->wParam == VK_RETURN) || (pMsg->wParam == VK_ESCAPE))
            pMsg->wParam = VK_TAB;
    }
    return CDialog::PreTranslateMessage(pMsg);
}

// This functions is part of the WorkerClass.
// If you call Start() it will in turn run this function in a seperate Worker Thread, thus not locking up the UI.
void MyDlg1::DoWork()//worker thread function
{
	bool _continue=true;
	UpdateData(); //updates the variables that take user input
	try//Check the values input by the user
	{
		double typchk1 = boost::lexical_cast<double>(str_expo_input);
		double typchk2 = boost::lexical_cast<double>(str_sleeptime_input);
		int typchk3 = boost::lexical_cast<int>(str_binning_input);
		double typchk4 = boost::lexical_cast<double>(tinc_input_string);
	}
	catch(boost::bad_lexical_cast &)
	{
		DigitalMicrograph::OkDialog("Expo, Sleeptime, Binning or tInc of incorrect type!\nTry again");
		_continue=false;
		OnBnClickedButton9();//reset buttons
		return;
	}

	if(_continue==true)//If inputs were fine, set relevant variables to the user values.
	{
		binning_input = boost::lexical_cast<int>(str_binning_input);
		expo_input = boost::lexical_cast<double>(str_expo_input);
		sleeptime_input=boost::lexical_cast<double>(str_sleeptime_input);
		tInc_input = boost::lexical_cast<double>(tinc_input_string);
		if((binning_input==1||binning_input==2||binning_input==3||binning_input==4)&&(tInc_input>0.25&&tInc_input<=1))//If user input values are correct
		{
		DEDCalibrate->binning = binning_input;
		DEDCollect->binning = binning_input;
		DEDCalibrate->expo = expo_input;
		DEDCollect->expo = expo_input;
		DEDCalibrate->sleeptime = sleeptime_input;
		DEDCollect->sleeptime = sleeptime_input;
		DEDCollect->tInc_factor = tInc_input;
		DEDProcess->tInc_factor = tInc_input;
		DigitalMicrograph::Result(boost::lexical_cast<std::string>(expo_input) + "  " + boost::lexical_cast<std::string>(binning_input) + "  " + boost::lexical_cast<std::string>(sleeptime_input) + "  " + "\n");	
		}
		else
		{
			if (!(binning_input == 1 || binning_input == 2 || binning_input == 3 || binning_input == 4))
			{
				DigitalMicrograph::OkDialog("Binning must take value 1,2,3 or 4 only\nPlease try again");
				OnBnClickedButton9();//reset buttons
				return;
			}
			else
			{
				DigitalMicrograph::OkDialog("tInc factor must take a value between 0.25 and 1\nPlease try again (the default is 0.8)");
				OnBnClickedButton9();//reset buttons
				return;
			}
		}
	}
	else
	{
		DigitalMicrograph::OkDialog("Error in starting selected action");
	}

	if(tiltcalibration == true)
	{
		DEDCalibrate->DoCalibration(progress_ctrl);
		GetDlgItem(IDC_BUTTON1)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON2)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON4)->EnableWindow(TRUE);
		tiltcalibration=false;
	}
	
	if(collect == true)
	{
		DEDCollect->DoCollection(progress_ctrl);
		GetDlgItem(IDC_BUTTON1)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON2)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON4)->EnableWindow(TRUE);
		collect=false;
	}

	if (process == true)
	{
		DEDProcess->DoProcess(progress_ctrl);
		GetDlgItem(IDC_BUTTON1)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON2)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON4)->EnableWindow(TRUE);
		//GetDlgItem(IDC_EDIT3)->EnableWindow(TRUE);
		process = false;
	}

	DigitalMicrograph::Result("At end of plugin worker thread\n");
	return;

}

void MyDlg1::OnBnClickedButton1()//Tilt calibration button
{
	// TODO: Add your control notification handler code here
	tiltcalibration = true;
	GetDlgItem(IDC_BUTTON1)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON2)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON4)->EnableWindow(FALSE);
	Start();
}


void MyDlg1::OnBnClickedButton2()//Collect button
{
	// TODO: Add your control notification handler code here
	collect = true;
	GetDlgItem(IDC_BUTTON1)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON2)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON4)->EnableWindow(FALSE);
	Start();
}


void MyDlg1::OnBnClickedButton3()//Draw cross on front image button
{
	try
	{
		DigitalMicrograph::Image Front;
		// TODO: Add your control notification handler code here
		if (DigitalMicrograph::GetFrontImage(Front))
		{
			Front = DigitalMicrograph::FindFrontImage();
			ExtraGatan::DrawRedX(&Front);
		}
		else
		{
			DigitalMicrograph::OkDialog("Could not find front image");
		}
	}
	catch (...)
	{
		DigitalMicrograph::Result("in button 3 catch\n");
		return;
	}
}


void MyDlg1::OnBnClickedButton4()//Process button
{
	// TODO: Add your control notification handler code here
	process = true;
	GetDlgItem(IDC_BUTTON1)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON2)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON4)->EnableWindow(FALSE);
	Start();
}


void MyDlg1::OnEnChangeEdit3()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the __super::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}


void MyDlg1::OnEnChangeEdit8()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the __super::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}


void MyDlg1::OnBnClickedButton5()//Grab shift and tilt button
{
	// TODO: Add your control notification handler code here
	if (ExtraGatan::CheckCamera())//Check a camera is inserted, then get current shift and tilt
	{
		ExtraGatan::EMGetBeamTilt(&TiltX0, &TiltY0);
		ExtraGatan::EMGetBeamShift(&ShiftX0, &ShiftY0);
		curr_shift = boost::lexical_cast<std::string>(ShiftX0)+", " + boost::lexical_cast<std::string>(ShiftY0);//Update the strings to display grabbed shift/tilt
		curr_tilt = boost::lexical_cast<std::string>(TiltX0)+", " + boost::lexical_cast<std::string>(TiltY0);
	}
	else
	{
		curr_shift = "failed to grab";
		curr_tilt = "requested values";
	}
	Grabbed_Shift_string = curr_shift.c_str();
	Grabbed_Tilt_String = curr_tilt.c_str();
	UpdateData(0);
}


void MyDlg1::OnBnClickedButton6()//"Get Shift and Tilt Values from Global Tags" button
{
	// TODO: Add your control notification handler code here
	Persistent = DigitalMicrograph::GetPersistentTagGroup();
	if (DigitalMicrograph::TagGroupGetTagAsLong(Persistent, "Info:ShiftX0", &TagShiftX0) && DigitalMicrograph::TagGroupGetTagAsLong(Persistent, "Info:ShiftY0", &TagShiftY0)
		&& DigitalMicrograph::TagGroupGetTagAsLong(Persistent, "Info:TiltX0", &TagTiltX0) && DigitalMicrograph::TagGroupGetTagAsLong(Persistent, "Info:TiltY0", &TagTiltY0))//Attempt to get information from the tag path
	{
		tag_shift = boost::lexical_cast<std::string>(TagShiftX0)+", " + boost::lexical_cast<std::string>(TagShiftY0);//Display the tag values by changing the relevant strings
		tag_tilt = boost::lexical_cast<std::string>(TagTiltX0)+", " + boost::lexical_cast<std::string>(TagTiltY0);
		DigitalMicrograph::Result("Got shift and tilt from tags\n");
	}
	else
	{
		tag_shift = "failed to get";
		tag_tilt = "tag data";
	}
	Tag_Shift_Display_String = tag_shift.c_str();
	Tag_Tilt_Display_String = tag_tilt.c_str();
	UpdateData(0);
	set_shift_tilt_tag = true;
	
}


void MyDlg1::OnBnClickedButton7()//"Set Shift and Tilt to Grabbed Values" button
{
	// TODO: Add your control notification handler code here
	if (ExtraGatan::CheckCamera() && TiltX0 != 0 && TiltY0 != 0 && ShiftX0 != 0 && ShiftY0 != 0)//Check a camera is inserted and the shift/tilt values are not 0
	{
		UpdateData();
		sleeptime_input = boost::lexical_cast<double>(str_sleeptime_input);
		ExtraGatan::ABSShift(ShiftX0, ShiftY0);
		DigitalMicrograph::Sleep(sleeptime_input);
		ExtraGatan::ABSTilt(TiltX0, TiltY0);
		DigitalMicrograph::Sleep(sleeptime_input);
	}
	else
	{
		DigitalMicrograph::Result("Failed to set to grabbed values\n");
	}
}


void MyDlg1::OnBnClickedButton8()//"Set Shift and Tilt to Global Tag Values" button
{
	// TODO: Add your control notification handler code here
	if (ExtraGatan::CheckCamera() && TagTiltX0 != 0 && TagTiltY0 != 0 && TagShiftX0 != 0 && TagShiftY0 != 0)//Check a camera is inserted and the shift/tilt values are not 0
	{
		UpdateData();
		sleeptime_input = boost::lexical_cast<double>(str_sleeptime_input);
		ExtraGatan::ABSShift(TagShiftX0, TagShiftY0);
		DigitalMicrograph::Sleep(sleeptime_input);
		ExtraGatan::ABSTilt(TagTiltX0, TagTiltY0);
		DigitalMicrograph::Sleep(sleeptime_input);
		DigitalMicrograph::Result("Shift and tilt set to saved values\n");
	}
	else
	{
		DigitalMicrograph::Result("Failed to set to saved values\n");
	}
}


void MyDlg1::OnNMCustomdrawProgress2(NMHDR *pNMHDR, LRESULT *pResult)//From progress bar
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;
}

void MyDlg1::OnBnClickedButton9()// The reset button, this re-enables the buttons on the plugin in case one part fails
{
	GetDlgItem(IDC_BUTTON1)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON2)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON4)->EnableWindow(TRUE);
	// TODO: Add your control notification handler code here
}


void MyDlg1::OnBnClickedButton10()// "Save current Shift and Tilt to Global Tags" button
{
	Persistent = DigitalMicrograph::GetPersistentTagGroup();
	if (ExtraGatan::CheckCamera())//check a camera is inserted
	{
		ExtraGatan::EMGetBeamTilt(&TiltX0, &TiltY0);
		ExtraGatan::EMGetBeamShift(&ShiftX0, &ShiftY0);
		try
		{
			DigitalMicrograph::TagGroupSetTagAsLong(Persistent, "Info:ShiftX0", ShiftX0);//Save current shift/tilt to tags
			DigitalMicrograph::TagGroupSetTagAsLong(Persistent, "Info:ShiftY0", ShiftY0);
			DigitalMicrograph::TagGroupSetTagAsLong(Persistent, "Info:TiltX0", TiltX0);
			DigitalMicrograph::TagGroupSetTagAsLong(Persistent, "Info:TiltY0", TiltY0);
			DigitalMicrograph::Result("Shift and tilt saved to tags\n");
		}
		catch (...)
		{
			DigitalMicrograph::Result("Failed to save shift and tilt to tags\n");
		}
	}
	else
	{
		DigitalMicrograph::Result("Failed, no camera detected\n");
	}
	// TODO: Add your control notification handler code here
}
