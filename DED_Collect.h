#pragma once
#include "stdafx.h"
#include "boost/lexical_cast.hpp"
#include <sstream>
#include "standardfunctions.h"
#include "utility.h"
#include "ExtraGatan.h"
#include "DMPlugInCamera.h"

#include "MyDlg1.h"
#include "Acquisition.h"

	  //***     Digital Electron Diffraction      ***\\
	 //** Richard Beanland r.beanland@warwick.ac.uk **\\
	//**    Joel C Forster j.c.forster@warwick.ac.uk **\\
   //**       James Gott j.a.gott@warwick.ac.uk       **\\
  //*				 C++ v1.1 July 2014				    *\\

class Collect
{
public:
	long Camb, Camr;// Camera pixel height and width
	double CamL; // Camera length
	float mag, alpha; // Microscope variables
	int binning;//controlled by drop down
	double expo, _t, _l, _b, _r, sleeptime;
	double tInc_factor; // controlled by text edit
	DigitalMicrograph::Image img1, TiltCal, Xsh, Ysh, CBED;
	Gatan::PlugIn::ImageDataLocker XshLock, YshLock, TiltLock, CBEDLock, img1Lock;
	float* XshPix;
	float* YshPix;
	float* TiltCalPix;
	float* CBEDPix;
	float* img1Pix;
	DigitalMicrograph::TagGroup Persistent;
	std::string pathname, file_name;

	double xCal, yCal;
	float nTilts, tInc;
	std::string prompt;
	long nPts;
	long TiltX0, TiltY0, ShiftX0, ShiftY0, imgX, imgY; // tilt and shift DAC coordinates and image size in pixels
	bool quit, success;
	float pX, pY, tX, tY, sX, sY;
	long nX, nY, nnX, nnY;
	void GetCoordsFromNTilts(long, long, int &, int &);
	DigitalMicrograph::TagGroup CBEDGroup;
	DigitalMicrograph::Image Dark, Gain; // Reference images

	float Rr,spot,xTpX,xTpY,yTpX,yTpY,xShpX,xShpY,yShpX,yShpY;
	long nCals;

	bool LoadTiltCalibTagInfo(DigitalMicrograph::Image*);
	bool LoadTiltCalibImages(DigitalMicrograph::Image*, DigitalMicrograph::Image*, DigitalMicrograph::Image*);
	void DoCollection(CProgressCtrl&);

	Acquisition CollectAcquisition;
};