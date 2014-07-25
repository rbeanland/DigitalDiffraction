// MyDlg1.cpp : implementation file
//
#pragma OPENCL EXTENSION cl_khr_fp64 : enable

#include "stdafx.h"
#include "MyDlg1.h"
#include "boost/lexical_cast.hpp"
#include <sstream>
#include "DMPluginCamera.h"
#include "standardfunctions.h"
#include "CLKernels.h"
#include "clFourier.h"
#include "utility.h"
#include "OptionsDialog.h"


// GPU usually platform 0 dev 0, cpu is 0,1 with AMD Gfx or 1,0 with nVidia, 
// cant see Gfx over Remote Desktop though so CPU 0,0


// MyDlg1 dialog

IMPLEMENT_DYNAMIC(MyDlg1, CDialog)

// This constructor runs as soon as the plugin is loaded when this tab is generated. This looks in the global tags to find information on which OpenCL device/platform should be used.
// It then generates the Command Queue and Context ready to run kernels. If these tags are not present in leaves information in results window about how to set this up.
MyDlg1::MyDlg1(CWnd* pParent /*=NULL*/)
	: CDialog(MyDlg1::IDD, pParent)
	, str_binning_input(_T("4"))
	, str_expo_input(_T("0.04"))
	, str_sleeptime_input(_T("0.02"))
{
// TODO: Check for persistent tags to get platform and devnumber


	DEDCalibrate = new Calibrate;
	DEDCollect = new Collect;
	tiltcalibration = false;
	collect = false;
	
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


	//Setup OpenCL
	OpenCLAvailable = false;
	context = NULL;
	numDevices = 0;
	devices = NULL;

	// Maybe Can Do OpenCL setup and device registering here - Print to Ouput with device data?
	// Discover and initialize available platforms
	cl_uint numPlatforms = 0;
	cl_platform_id * platforms = NULL;

	// Use clGetPlatformIds() to retrieve the number of platforms
	status = clGetPlatformIDs(0,NULL,&numPlatforms);

	// Allocate enough space for each platform
	platforms = (cl_platform_id*)malloc(numPlatforms*sizeof(cl_platform_id));

	// Fill in platforms with clGetPlatformIDs()
	status = clGetPlatformIDs(numPlatforms,platforms,NULL);

	// Discover and initialize available devices	
	// use clGetDeviceIDs() to retrieve number of devices present
	status = clGetDeviceIDs(platforms[platformnumber],CL_DEVICE_TYPE_ALL,0,NULL,&numDevices);

	// Allocate enough space for each device
	devices = (cl_device_id*)malloc(numDevices*sizeof(cl_device_id));

	// Fill in devices with clGetDeviceIDs()
	status = clGetDeviceIDs(platforms[platformnumber],CL_DEVICE_TYPE_ALL,numDevices,devices,NULL);

	// Most of initialisation is done, would be nice to print device information...
	//Getting the device name
	size_t deviceNameLength = 4096;
	size_t actualSize;
	char* tempDeviceName = (char*)malloc(4096);
	char* deviceName;
	status |= clGetDeviceInfo(devices[devicenumber], CL_DEVICE_NAME, deviceNameLength, tempDeviceName, &actualSize);

	if(status == CL_SUCCESS)
	{
		deviceName = (char*)malloc(actualSize);
		memcpy(deviceName, tempDeviceName, actualSize);
		free(tempDeviceName);

		std::string devName(deviceName);
		Gatan::DM::Result("Using OpenCL on device "+devName+" - OCL - EWR\n");
		Gatan::DM::Result("To change edit the Global Tags OpenCL:Platform and OpenCL:Device then restart DM\n");
		OpenCLAvailable = true;

		context = clCreateContext(NULL,numDevices,devices,NULL,NULL,&status);
		clq = new clQueue();
		clq->SetupQueue(context,devices[devicenumber]);
		cldev = new clDevice(numDevices,devices);
	}
	if(status!=CL_SUCCESS)
	{
		Gatan::DM::Result("Could not setup OpenCL on this computer, run clinfo to check availability\n");
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
}


BEGIN_MESSAGE_MAP(MyDlg1, CDialog)
	ON_BN_CLICKED(IDC_BUTTON1, &MyDlg1::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &MyDlg1::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &MyDlg1::OnBnClickedButton3)
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
void MyDlg1::DoWork()
{
	bool _continue=true;
	UpdateData(); //updates the variables that take user input
	try
	{
		double typchk1 = boost::lexical_cast<double>(str_expo_input);
		double typchk2 = boost::lexical_cast<double>(str_sleeptime_input);
		int typchk3 = boost::lexical_cast<int>(str_binning_input);
	}
	catch(boost::bad_lexical_cast &)
	{
		DigitalMicrograph::OkDialog("Expo, Sleeptime or Binning of incorrect type!\nTry again");
		_continue=false;
		return;
	}

	

	if(_continue==true)
	{
		binning_input = boost::lexical_cast<int>(str_binning_input);
		expo_input = boost::lexical_cast<double>(str_expo_input);
		sleeptime_input=boost::lexical_cast<double>(str_sleeptime_input);
		if(binning_input==1||binning_input==2||binning_input==3||binning_input==4)
		{
		DEDCalibrate->binning = binning_input;
		DEDCollect->binning = binning_input;
		DEDCalibrate->expo = expo_input;
		DEDCollect->expo = expo_input;
		DEDCalibrate->sleeptime = sleeptime_input;
		DEDCollect->sleeptime = sleeptime_input;
		DigitalMicrograph::Result(boost::lexical_cast<std::string>(expo_input) + "  " + boost::lexical_cast<std::string>(binning_input) + "  " + boost::lexical_cast<std::string>(sleeptime_input) + "  " + "\n");
		
		}
		else
		{
			DigitalMicrograph::OkDialog("Binning must take value 1,2,3 or 4 only\nPlease try again");
			return;
		}
		//DEDCalibrate->binning = binning_input;
		//DEDCollect->binning = binning_input;
		//DEDCalibrate->expo = expo_input;
		//DEDCollect->expo = expo_input;
		//DEDCalibrate->sleeptime = sleeptime_input;
		//DEDCollect->sleeptime = sleeptime_input;
		//DigitalMicrograph::Result(boost::lexical_cast<std::string>(expo_input) + "  " + boost::lexical_cast<std::string>(binning_input) + "  " + boost::lexical_cast<std::string>(sleeptime_input) + "  " + "\n");
	}
	else
	{
		DigitalMicrograph::OkDialog("Failed to calibrate");// need to change this
	}

	if(tiltcalibration == true)
	{
		DEDCalibrate->DoCalibration();
		tiltcalibration=false;
	}

	if(collect == true)
	{
		DEDCollect->DoCollection();
		collect=false;
	}
}

void MyDlg1::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
	tiltcalibration = true;
	Start();
}



void MyDlg1::OnBnClickedButton2()
{
	// TODO: Add your control notification handler code here
	collect = true;
	Start();
}


void MyDlg1::OnBnClickedButton3()
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


	}
	// TODO: Add your control notification handler code here
}
