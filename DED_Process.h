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
    //**      James Gott j.a.gott@warwick.ac.uk      **\\
   //*				 C++ v1.0 July 2014				   *\\

class Process
{
public:

	double t,l,b,r;


	void ROIpos(DigitalMicrograph::Image,std::string, double&, double&, double&, double&);
	void GetCoordsFromNTilts(long, long, int&, int&);
	void AddBlueCircle(DigitalMicrograph::Image*,float,float,float,float);
	void AddYellowArrow(DigitalMicrograph::Image*, float, float, float, float);
	void UserG(DigitalMicrograph::Image,float,float&,float&,float&, float&,float&,float&);
	void GetMeanG(DigitalMicrograph::Image, DigitalMicrograph::Image&, float*, double&, double&, double&, double&, int&);
	void GetG_Vectors(DigitalMicrograph::Image Avg, float Rr, float&, float&, float&, float&, long&, long&);
	void DoProcess();

};
