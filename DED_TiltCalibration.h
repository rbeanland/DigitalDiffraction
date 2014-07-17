#pragma once
#include "stdafx.h"
#include "boost/lexical_cast.hpp"
#include <sstream>
#include "standardfunctions.h"
#include "utility.h"
#include "ExtraGatan.h"
#include "DMPlugInCamera.h"

	  //***     Digital Electron Diffraction      ***\\
	 //** Richard Beanland r.beanland@warwick.ac.uk **\\
	//**    Joel C Forster j.c.forster@warwick.ac.uk **\\
   //**       James Gott j.a.gott@warwick.ac.uk       **\\
  //*				 C++ v1.0 July 2014				    *\\

class Calibrate
{
public:
	long Camb, Camr;
	int binning; // to be converted to plugin input control values
	double	expo, //used to
			_t, // not used?????
			_l, // not used?????
			_b, //used later with y1 to set the no. of pixels to move [x,y] 
			_r, //used later with x1 to set the no. of pixels to move [x,y]
			sleeptime; //delay while waiting for microscope to respond, to be converted to plugin input
	DigitalMicrograph::Image img1, img0, imgCC; // live image, reference image & cross-correlation
	DigitalMicrograph::ScriptObject img_src;

	bool quit, success;

	float Rr; //ln333 script
	float T1X, T1Y, T2X, T2Y, dTilt, dShift, detT, xTpX, xTpY, yTpX, yTpY;
	float Sh1X, Sh1Y, Sh2X, Sh2Y, detSh, xShpX, xShpY, yShpX, yShpY;
	long ShiftX0, ShiftY0, TiltX0, TiltY0;
	long nTilts, tX, tY, x, y, dx, dy;
	float tIncX, tIncY, maxval;


	DigitalMicrograph::Image Xsh, Ysh, TiltCal;

	// variables used in subroutines, may not be needed to be declared here
	bool acq_paramas_changed;
	float max_wait;

	long t,l,b,r;
	DigitalMicrograph::Image medImg,
						IMG; //may not need this image here, may be created/declared in function inputs
	float thr; // don't think it's needed, used as a function parameter in ExtraGatan.

	//EMChangeMode
	std::string mode_want, mode_is; // may not need both of these, or either, function in ExtraGatan




	void DoCalibration(); // aka main for calibration
	//void UpdateCameraImage(); // possibly done by Acquire in ExtraGatan, need to check
	//void RemoveOutliers(); // In ExtraGatan, not needed here, check though
	void GetCoordsFromNTilts(long, long, int &, int &);
	//void EMChangeMode(); // In ExtraGatan, not needed here, check though
	//void EMBeamCentre(); // In ExtraGatan, not needed here, check though
	//double TiltSize(); // In ExtraGatan, not needed here, check though!!!!!
	//double ShiftSize(); // In ExtraGatan, not needed here, check though, think these 3 are for the old rocking version, may need updating/changing
	//double DiscSize(); // In ExtraGatan, not needed here, check thpough




	// stuff in script main, may not need here

};