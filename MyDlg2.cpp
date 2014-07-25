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
	DEDProcess = new Process;
}

MyDlg2::~MyDlg2()
{
}

void MyDlg2::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(MyDlg2, CDialog)
	ON_BN_CLICKED(IDC_BUTTON1, &MyDlg2::OnBnClickedButton1)
END_MESSAGE_MAP()


// MyDlg2 message handlers


// This functions is part of the WorkerClass.
// If you call Start() it will in turn run this function in a seperate Worker Thread, thus not locking up the UI.
void MyDlg2::DoWork()
{
}

void MyDlg2::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here

	//testing roi
	//if(DigitalMicrograph::GetFrontImage())
	//	{
	//		DigitalMicrograph::Image In = DigitalMicrograph::FindFrontImage();
	//		In.GetOrCreateImageDocument().Show();

	//		DEDProcess->AddBlueCircle(&In, 20,300,40,500); 

	//		DEDProcess->AddYellowArrow(&In, 60, 1000, 2000, 800);

	//		//DigitalMicrograph::Image Diff = ExtraGatan::DiffImage(&In,false);
	//		//Diff.GetOrCreateImageDocument().ShowAtPosition(415,200);
	//	}
	//	else
	//	{
	//		DigitalMicrograph::OkDialog("Could not find front image");
	//		return;
	//	}
	//return;
	//DEDProcess->AddBlueCircle();

	long nPeaks = 100;
	DigitalMicrograph::Image Test;
	Test = DigitalMicrograph::RealImage("Test", 4, 1, nPeaks);
	DigitalMicrograph::Result("After image created\n");
	if (DigitalMicrograph::OkCancelDialog("Continue?"))
	{
		Test.GetOrCreateImageDocument().ShowAtPosition(15, 30);

		DigitalMicrograph::ImageDocument Testdoc;
		Testdoc = DigitalMicrograph::ImageGetOrCreateImageDocument(Test);

		DigitalMicrograph::SetWindowSize(Test, 800, 800);
		DigitalMicrograph::SetName(Test, "Test image");
		//DigitalMicrograph::Sleep(1);

		Gatan::PlugIn::ImageDataLocker TestLock(Test);
		float* Testpix = (float*)TestLock.get();
		DigitalMicrograph::Result("Made locker and pointer to first pixel\n");

		double pixval;
		pixval = DigitalMicrograph::GetPixel(Test, 0, 40);
		int ii, jj;
		for (ii = 0; ii < nPeaks; ii++)
		{
			
				//DigitalMicrograph::SetPixel(Test, ii, jj, 0.2);
				Testpix[ii] = 0.5*(float)ii;
		
		}




		DigitalMicrograph::Image Xx, Xxtrans, Vx;

		Xx = DigitalMicrograph::RealImage("Xx", 4, nPeaks, nPeaks);
		Xxtrans = DigitalMicrograph::RealImage("XxTransposed", 4, nPeaks, nPeaks);
		Vx = DigitalMicrograph::RealImage("Vx", 4, nPeaks, nPeaks);


		Gatan::PlugIn::ImageDataLocker xlock(Test);
		float* xpix = (float*)xlock.get();

		Gatan::PlugIn::ImageDataLocker xxlock(Xx);
		float* xxpix = (float*)xxlock.get();

		Gatan::PlugIn::ImageDataLocker xxtranslock(Xxtrans);
		float* xxtranspix = (float*)xxtranslock.get();

		Gatan::PlugIn::ImageDataLocker vxlock(Vx);
		float* vxpix = (float*)vxlock.get();

		int iii, jjj;

		//create Xx matrix, copies a vertical single pixel width X image into all npeak collumns
		for (iii = 0; iii < nPeaks; iii++)
		{
			for (jjj = 0; jjj < nPeaks; jjj++)
			{
				xxpix[nPeaks*jjj + iii] = xpix[jjj];
			}
		}
		//create transpose of matrix Xx
		for (iii = 0; iii < nPeaks; iii++)
		{
			for (jjj = 0; jjj < nPeaks; jjj++)
			{
				xxtranspix[nPeaks*jjj + iii] = xxpix[nPeaks*iii + jjj];
			}
		}
		//Vx=xxtrans-xx
		for (iii = 0; iii < nPeaks; iii++)
		{
			for (jjj = 0; jjj < nPeaks; jjj++)
			{
				vxpix[nPeaks*jjj + iii] = xxtranspix[nPeaks*jjj + iii] - xxpix[nPeaks*jjj + iii];
			}
		}



		Xx.GetOrCreateImageDocument().ShowAtPosition(15, 30);
		Xxtrans.GetOrCreateImageDocument().ShowAtPosition(15, 30);
		Vx.GetOrCreateImageDocument().ShowAtPosition(15, 30);



		//DEDProcess->AddBlueCircle(&Test, 10, 20, 15, 25);
		//DEDProcess->AddYellowArrow(&Test, 5, 15, 10, 25);
		

		//DigitalMicrograph::Window Colourscriptwin;
		//Colourscriptwin = DigitalMicrograph::NewScriptWindowFromFile("C:\\ProgramData\\Gatan\\Plugins\\SetColour.s");
		//DigitalMicrograph::ScriptWindowExecute(Colourscriptwin);

		//
		//DigitalMicrograph::Result("[0,40] = "+boost::lexical_cast<std::string>(pixval)+"\n");
		return;
	}
	else
	{
		DigitalMicrograph::Window Colourscriptwin;
		Colourscriptwin = DigitalMicrograph::NewScriptWindowFromFile("C:\\ProgramData\\Gatan\\Plugins\\SetColour.s");
		DigitalMicrograph::ScriptWindowExecute(Colourscriptwin);

		return;
	}

}
