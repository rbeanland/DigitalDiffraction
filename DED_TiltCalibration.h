#pragma once
#include "stdafx.h"
#include "boost/lexical_cast.hpp"
#include <sstream>
#include "standardfunctions.h"
#include "utility.h"
#include "ExtraGatan.h"
#include "DMPlugInCamera.h"
#include "Acquisition.h"

	  //***     Digital Electron Diffraction      ***\\
	 //** Richard Beanland r.beanland@warwick.ac.uk **\\
	//**    Joel C Forster j.c.forster@warwick.ac.uk **\\
   //**       James Gott j.a.gott@warwick.ac.uk       **\\
  //*				 C++ v1.1 July 2014				    *\\

class Calibrate
{
public:
	long Camb, Camr;//camera bottom and right (t,l,b,r)
	int binning; // controlled by user input dropdown box
	double	expo, //controlled by text edit box
			_t,  
			_l, 
			_b, 
			_r, //camera pixel coordinates
			sleeptime; //delay while waiting for microscope to respond, controlled by text box
	DigitalMicrograph::Image img1, img0, imgCC; // live image, reference image & cross-correlation

	float Rr;//CBED disc radius
	float T1X, T1Y, T2X, T2Y, dTilt, dShift, detT, xTpX, xTpY, yTpX, yTpY;// Tilt variables
	float Sh1X, Sh1Y, Sh2X, Sh2Y, detSh, xShpX, xShpY, yShpX, yShpY; // Shift variables
	long ShiftX0, ShiftY0, TiltX0, TiltY0; // DAC shift and tilt coordinates
	long nTilts, tX, tY, x, y, dx, dy;
	float tIncX, tIncY, maxval;

	DigitalMicrograph::Image Xsh, Ysh, TiltCal;
	DigitalMicrograph::TagGroup Persistent;
	bool acq_paramas_changed;
	float max_wait;

	long t,l,b,r;
	DigitalMicrograph::Image medImg, IMG; 
	float thr;
	std::string mode_want, mode_is;

	void DoCalibration(CProgressCtrl&);
	void GetCoordsFromNTilts(long, long, int &, int &);
	float Tiltsize(float dTilt, float *T1X, float *T1Y, float *T2X, float *T2Y, int binning, DigitalMicrograph::Image *img1, DigitalMicrograph::Image *imgCC, DigitalMicrograph::Image *img0, float expo, float sleeptime, long _b);
	float Shiftsize(float dShift, float *Sh1X, float *Sh1Y, float *Sh2X, float *Sh2Y, int binning, DigitalMicrograph::Image *img1, DigitalMicrograph::Image *imgCC, DigitalMicrograph::Image *img0, float expo, float sleeptime, long _b);

	Acquisition CalibrateAcquisition;
};