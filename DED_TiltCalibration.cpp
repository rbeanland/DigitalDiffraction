#include "stdafx.h"
#include "DED_TiltCalibration.h"
#include <time.h>

void Calibrate::GetCoordsFromNTilts(long nTilts,long currentPoint, int &i, int &j)
{
	int side = 2*nTilts+1; //this is the length of the image XSh/YSh
	j = floor(((double)currentPoint)/((double)side))-nTilts; //getting the x-coord for the XSh/Ysh based on current pt number
	i = ((currentPoint % side) - nTilts)*pow((float)(-1),(int)(j % 2));//NB % means modulo, flips sign every row
}

float Calibrate::Tiltsize(float dTilt, float *T1X, float *T1Y, float *T2X, float *T2Y, int binning, DigitalMicrograph::Image *img1, DigitalMicrograph::Image *imgCC, DigitalMicrograph::Image *img0, float expo, float sleeptime, long _b)
{
	//magnitude of dTilt
	DigitalMicrograph::Result("Tiltsize function started\n");
	long TiltX0, TiltY0;
	bool quit, success;
	quit = false; success = false;
	ExtraGatan::EMGetBeamTilt(&TiltX0, &TiltY0); // get the current beam tilt DAC numbers

	DigitalMicrograph::Sleep(sleeptime);
	CalibrateAcquisition.AcquireImage2(*img1); //acquire the current zero tilt image
	(*img1).DataChanged();
	*imgCC = DigitalMicrograph::CrossCorrelate(*img1, *img0);
	(*imgCC).DataChanged();

	long x0, y0;//coords of untilted beam (in pixels)
	float maxval;

	maxval = ExtraGatan::Max(*imgCC);
	ExtraGatan::PixelPos(*imgCC, maxval, &x0, &y0, false); //finding pixel positioni of max intensity
	long tX = TiltX0 + (long)dTilt; 
	ExtraGatan::ABSTilt(tX, TiltY0); //tilting the beam by the initial dtilt DAC guess
	DigitalMicrograph::Sleep(sleeptime);//have to give the microscope time to respond

	CalibrateAcquisition.AcquireImage2(*img1);//update img1 to tilted image
	(*img1).DataChanged();

	*imgCC = DigitalMicrograph::CrossCorrelate(*img1, *img0); // cross correlate with the initial image to see how much the beam has shifted on the image in pixels
	(*imgCC).DataChanged();

	long x, y;//coords of tilted beam
	maxval = ExtraGatan::Max(*imgCC);
	ExtraGatan::PixelPos(*imgCC, maxval, &x, &y, false); // getting pixel position of max intensity pixel
	float tPix = sqrt((float)((x - x0)*(x - x0) + (y - y0)*(y - y0)));//magnitude of the difference of beam position on the image in pixels, so the amount the beam has shifted on the image in pixels
	dTilt = (float)(dTilt*0.25*_b / tPix); //setting dtilt to 1/4 of image/camera height, dtilt/tpix gives DAC tilt per 'pixel shift', 0.25*_b is 1/4 of camera/image size, so result is converting 1/4 camera height in pixels to DAC number
	tX = TiltX0 + (long)dTilt;
	
	//now measure tilt(pixels) per DAC
	//finding change in position(pixels) for x-direction
	DigitalMicrograph::Result("measuring tilt(pixels) per DAC...\n");
	ExtraGatan::ABSTilt(TiltX0 + dTilt, TiltY0); //tilting the beam to move the beam by 1/4 of the image in pixels on the image

	DigitalMicrograph::Sleep(sleeptime);
	CalibrateAcquisition.AcquireImage2(*img1);	//update img1 to new tilted image
	(*img1).DataChanged();

	*imgCC = DigitalMicrograph::CrossCorrelate(*img1, *img0); // find by how much this moves the beam position on the image
	(*imgCC).DataChanged();
	DigitalMicrograph::UpdateImage(*imgCC);

	maxval = ExtraGatan::Max(*imgCC); // finding max pixel intensity
	ExtraGatan::PixelPos(*imgCC, maxval, &x, &y, false);// finding position of max

	*T1X = (x - x0) / dTilt; // finding the change in beam position on the image in pixels in the x-direction per DAC number change, so x-pixels per DAC
	*T1Y = (y - y0) / dTilt;

	//finding change in position(pixels) for y-direction
	DigitalMicrograph::Result("At finding change in position(pixels for y-direction\n");
	ExtraGatan::ABSTilt(TiltX0, TiltY0 + dTilt);
	DigitalMicrograph::Sleep(sleeptime);

	CalibrateAcquisition.AcquireImage2(*img1);
	(*img1).DataChanged();
	*imgCC = DigitalMicrograph::CrossCorrelate(*img1, *img0);
	(*imgCC).DataChanged();

	maxval = ExtraGatan::Max(*imgCC);
	ExtraGatan::PixelPos(*imgCC, maxval, &x, &y, false);
	*T2X = (x - x0) / dTilt;
	*T2Y = (y - y0) / dTilt;
	// setting to 0 position (resetting beam position)
	DigitalMicrograph::Result("At setting beam to 0 position\n");
	ExtraGatan::ABSTilt(TiltX0, TiltY0);
	DigitalMicrograph::Sleep(sleeptime);
	CalibrateAcquisition.AcquireImage2(*img1);

	(*img1).DataChanged();
	DigitalMicrograph::Result("dTilt =" + boost::lexical_cast<std::string>(dTilt)+"\n");

	DigitalMicrograph::Result("Tiltsize function complete\n");
	return (dTilt); //returns the DAC required to tilt the beam and move the beam by 1/4 of the image (in pixels)
}

float Calibrate::Shiftsize(float dShift, float *Sh1X, float *Sh1Y, float *Sh2X, float *Sh2Y, int binning, DigitalMicrograph::Image *img1, DigitalMicrograph::Image *imgCC, DigitalMicrograph::Image *img0, float expo, float sleeptime, long _b)
{
	DigitalMicrograph::Result("Shiftsize function started\n");
	//magnitude of dShift
	long ShiftX0, ShiftY0;
	ExtraGatan::EMGetBeamShift(&ShiftX0, &ShiftY0); //getting shift DAC numbers for beam at initial position
	bool quit, success;
	quit = false; success = true;
	CalibrateAcquisition.AcquireImage2(*img1); // zero shift image
	//(*img1).DigitalMicrograph::GetImageDocument().ShowAtPosition(15, 245);
	*img0 = DigitalMicrograph::ImageClone(*img1);//put into img0, cloning to compare
	*imgCC = DigitalMicrograph::CrossCorrelate(*img1, *img0);
	
	long x0, y0;//coords of unshifted beam
	float maxval;
	maxval = ExtraGatan::Max(*imgCC);
	ExtraGatan::PixelPos(*imgCC, maxval, &x0, &y0, false); //this should then get coordinates of image centre
	long sX = (long)(ShiftX0 + dShift); //used to shift beam x direction
	long sY = ShiftY0;

	ExtraGatan::ABSShift(sX, ShiftY0);// shifting beam by first guess dshift, DAC number
	DigitalMicrograph::Sleep(sleeptime);//have to give the microscope time to respond
	CalibrateAcquisition.AcquireImage2(*img1);//getting shifted image
	(*img1).DataChanged();
	
	*imgCC = DigitalMicrograph::CrossCorrelate(*img1, *img0); //crosscorrelate with unshifted beam img
	(*imgCC).DataChanged();
	long x, y;//coords of shifted beam
	maxval = ExtraGatan::Max(*imgCC);
	ExtraGatan::PixelPos(*imgCC, maxval, &x, &y, false);
	float tPix = sqrt((float)((x - x0)*(x - x0) + (y - y0)*(y - y0)));//magnitude of the difference (in pixels), or amount beam shifted on image in pixels
	ExtraGatan::ABSShift(ShiftX0, ShiftY0);//setting shift back to 0 position
	DigitalMicrograph::Sleep(sleeptime);
	dShift = (float)(ExtraGatan::round(dShift*0.25*((float)_b) / tPix)); //1/4 initial DAC shift guess/shift in pixels * img height => dac per pixel * 1/4 img height, so dshift is now DAC required to move beam 1/4 img height, pixel has been convverted to DAC
	sX = ShiftX0 + (long)dShift;// added cast to long, updating to new dshift value
	sY = ShiftY0;
	//now measure shift(pixels) per DAC
	// ln 213
	ExtraGatan::ABSShift(sX, sY);// shifting beam in x direction using new dshift value
	DigitalMicrograph::Sleep(sleeptime);//have to give the microscope time to respond
	CalibrateAcquisition.AcquireImage2(*img1); //shifted image
	(*img1).DataChanged();
	*imgCC = DigitalMicrograph::CrossCorrelate(*img1, *img0); //comparing current beam position to initial position
	(*imgCC).DataChanged();
	maxval = ExtraGatan::Max(*imgCC);
	ExtraGatan::PixelPos(*imgCC, maxval, &x, &y, false); // getting coordinates for the amount the beam has shifted in pixels
	*Sh1X = ((float)(x - x0)) / dShift;
	*Sh1Y = ((float)(y - y0)) / dShift;
	ExtraGatan::ABSShift(ShiftX0, ShiftY0);//resetting beam shift
	DigitalMicrograph::Sleep(sleeptime);
	sX = ShiftX0; // repeating for y with same dshift
	sY = ShiftY0 + (long)dShift;//added cast to long
	ExtraGatan::ABSShift(sX, sY);// shifting beam in y direction using new dshift
	DigitalMicrograph::Sleep(sleeptime);//have to give the microscope time to respond
	CalibrateAcquisition.AcquireImage2(*img1);//shifted image
	DigitalMicrograph::ImageDataChanged(*img1);
	*imgCC = DigitalMicrograph::CrossCorrelate(*img1, *img0);

	DigitalMicrograph::ImageDataChanged(*imgCC);
	maxval = ExtraGatan::Max(*imgCC);
	ExtraGatan::PixelPos(*imgCC, maxval, &x, &y, false);
	*Sh2X = ((float)(x - x0)) / dShift;
	*Sh2Y = ((float)(y - y0)) / dShift; // the result of a DAC shift in Y direction on pixel position
	ExtraGatan::ABSShift(ShiftX0, ShiftY0); // resetting shift poisition
	DigitalMicrograph::Sleep(sleeptime);//have to give the microscope time to respond

	DigitalMicrograph::Result("Shiftsize function complete\n");
	return(dShift); // returning dshift(DAC) required to move the beam 1/4 of image height (pixels)
}

void Calibrate::DoCalibration(CProgressCtrl& progress_ctrl)// progress_ctrl controls the progress bar on the plugin
{
	DigitalMicrograph::Result("In DoCalibration\n");
	DigitalMicrograph::Result("binning: " + boost::lexical_cast<std::string>(binning)+"\n");

	DigitalMicrograph::TagGroup Persistent;
	if(!ExtraGatan::CheckCamera()){DigitalMicrograph::OkDialog("No Camera Detected\nPlease ensure the camera is inserted"); return;} //check a camera is detected before doing anything
	Persistent = DigitalMicrograph::GetPersistentTagGroup(); // Getting the global tags tag group
	Camb = 2672;
	Camr = 2688;
	_b = Camb/binning;
	_r = Camr/binning;

	long CamX, CamY;
	CamX = _r;
	CamY = _b;
	float spot = ExtraGatan::EMGetSpotSize() + 1;
	float mag = ExtraGatan::EMGetMagnification();
	double CamL = ExtraGatan::EMGetCameraLength();
	DigitalMicrograph::Result("CamL = " + boost::lexical_cast<std::string>(CamL)+"\n");
	float alpha = 3;
	if (!DigitalMicrograph::GetNumber("Alpha?", alpha, &alpha))
	{
		return;
	}
	std::string Tag_Path, settings_Tag_path; //Tag paths for setting global tags
	Tag_Path = "DigitalDiffraction:Alpha="+boost::lexical_cast<std::string>(alpha)+":Binning="+boost::lexical_cast<std::string>(binning)+":CamL="+boost::lexical_cast<std::string>(CamL)+"mm:";
	settings_Tag_path = "DigitalDiffraction:Settings:";
	long f=0; //date format for GetDate & GetTime
	char date[11], time[6]; // strings to hold current date/time
	std::string datestring, timestring;
	DigitalMicrograph::GetDate(f,date,11);
	DigitalMicrograph::GetTime(f,time,6);
	int i;
	for(i=0; i<10; i++)//copying the date and time to strings used to print values
	{
		datestring += date[i];
	}
	for (i = 0; i < 5; i++)
	{
		timestring += time[i];
	}
	std::string datetimestring=datestring+"_"+timestring;

	DigitalMicrograph::Result("\nStarting tilt calibration "+datetimestring+"\n");//Print the current date and time

	DigitalMicrograph::TagGroupSetTagAsString(Persistent, (Tag_Path+"Date").c_str(), datetimestring);
	DigitalMicrograph::TagGroupSetTagAsFloat(Persistent, (Tag_Path+"Spot size").c_str(), spot);
	
	//Preparing parameters ready for acquisitions using the Acquisition class
	CalibrateAcquisition.expo = expo;
	CalibrateAcquisition.binning = binning;
	CalibrateAcquisition.CheckCamera();
	DigitalMicrograph::Result("Checked camera\n");
	DigitalMicrograph::Result("Setting up acquire parameters for acquisitions...\n");
	CalibrateAcquisition.SetAcquireParameters();
	DigitalMicrograph::Result("parameters set\n");
	
	//Get the user to change the mode to DIFF mode
	ExtraGatan::EMChangeMode("DIFF");
	//Stop any current camera viewer
	bool close_view = true, stop_view = false;
	try
	{
		Gatan::CM::StopCurrentCameraViewer(close_view);
	}
	catch(...) // ... cathces all exceptions thrown
	{
		DigitalMicrograph::OkDialog("Couldn't stop camera properly, try again!");
		return;
	}

	try
	{
		Gatan::uint32 bx, by;								//binningx and binningy
		double expochk;										//used to check the correct values were set in acquisition parameters
		Gatan::Camera::AcquisitionProcessing processingchk;	//
		try{ //checking values of acquisition parameters just set
			DigitalMicrograph::Result("Checking binning ...");
			Gatan::Camera::GetBinning(CalibrateAcquisition.acqparams, &bx, &by);
			Gatan::Camera::GetExposure(CalibrateAcquisition.acqparams, &expochk);
			Gatan::Camera::GetProcessing(CalibrateAcquisition.acqparams, &processingchk);
			DigitalMicrograph::Result("From acquisition parameters: binx = " + boost::lexical_cast<std::string>(bx)+", biny = " + boost::lexical_cast<std::string>(by)+"\n");
			DigitalMicrograph::Result("From acquisition parameters: expo = " + boost::lexical_cast<std::string>(expochk)+"\n");
			DigitalMicrograph::Result("From acquisition parameters: processing = " + boost::lexical_cast<std::string>(processingchk)+"\n");
		}
		catch (...)
		{
			DigitalMicrograph::Result("Failed to check acquisition parameter values\n");
		}
		img1 = Gatan::Camera::CreateImageForAcquire(CalibrateAcquisition.acq, "Acquired Image");//could be moved to SetAcquireParameters
		CalibrateAcquisition.acqsource->BeginAcquisition();//needed before any AcquireImage2() can be called
		clock_t start = clock();//clock to time acquisition
		CalibrateAcquisition.AcquireImage2(img1);//acquiring image
	
		clock_t finish = clock() - start;
		DigitalMicrograph::Result("AcquireImage2 time = " + boost::lexical_cast<std::string>(((float)finish) / CLOCKS_PER_SEC) + " seconds\n");

		DigitalMicrograph::Sleep(sleeptime);
		DigitalMicrograph::ImageDataChanged(img1);//to update the image displayed
		DigitalMicrograph::ImageDocumentShowAtPosition(DigitalMicrograph::ImageGetOrCreateImageDocument(img1),230,30); //Returns an image document containing the image, creating one if necessary.
		DigitalMicrograph::SetWindowSize(img1, 200, 200);

		//Set up reference image
		img0 = DigitalMicrograph::ImageClone(img1);
		DigitalMicrograph::SetName(img0,"Reference");
		//and cross correlation image
		imgCC = DigitalMicrograph::CrossCorrelate(img1,img0);
		DigitalMicrograph::SetName(imgCC,"Cross correlation");
	
		long img1_X, img1_Y; //size of img1
		DigitalMicrograph::Get2DSize(img1, &img1_X, &img1_Y);
		long data_type = DigitalMicrograph::GetDataType(img1);

		//Get radius of CBED disc
		long rX, rY;
		Rr=ExtraGatan::DiscSize(img1, &rX, &rY, 2);
		DigitalMicrograph::TagGroupSetTagAsFloat(Persistent, (Tag_Path+"Disc Radius").c_str(),Rr);
		DigitalMicrograph::Result("At get radius of CBED disc, radius Rr =" + boost::lexical_cast<std::string>(Rr)+"\n");

		//Set magnitude of tilt
		//dTilt gives diffraction pattern shift 1/4 of camera height															//convert DAC to	Desired DAC	 Desired shift 		
		//[T1X,T1Y] is the beam tilt in pixels [X,Y] per x DAC																	//pixels	|	      | shift      |   in pixels
		DigitalMicrograph::Result("Calling TiltSize...\n");																		//		(T1X T2X) x (DACX)  =  (pShX)
		dTilt = Tiltsize(40000 / CamL, &T1X, &T1Y, &T2X, &T2Y, binning, &img1, &imgCC, &img0, expo, sleeptime, _b);				//		(T1Y T2Y)   (DACY)     (pShY)		
	
		detT = T1X*T2Y - T2X*T1Y; //getting the determinant to find the inverse of the DAC to pixel conversion factor matrix
		xTpX = T2Y/detT; //EMSetBeamTilt(xTpX, xTpY) shifts the disc 1 x-pixel, converting each DAC->pixel to pixel->DAC using inverse of matrix
		xTpY = -T1Y/detT;
		yTpX = -T2X/detT; //EMSetBeamTilt(yTpX, yTpY) shift the disc 1 y-pixel
		yTpY = T1X/detT;
		//Save them to global tag group, so these can be used by other routines
		DigitalMicrograph::TagGroupSetTagAsFloat(Persistent, (Tag_Path+"xTpX").c_str(), xTpX);
		DigitalMicrograph::TagGroupSetTagAsFloat(Persistent, (Tag_Path+"xTpY").c_str(), xTpY);
		DigitalMicrograph::TagGroupSetTagAsFloat(Persistent, (Tag_Path+"yTpX").c_str(), yTpX);
		DigitalMicrograph::TagGroupSetTagAsFloat(Persistent, (Tag_Path+"yTpY").c_str(), yTpY);

		bool print_xtpx = true;
		try
		{
			DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (settings_Tag_path + "Print xTpX etc").c_str(), &print_xtpx);
		}
		catch (...)	{	}
		if (print_xtpx == true)
		{
			DigitalMicrograph::Result("xTpX = " + boost::lexical_cast<std::string>(xTpX)+", xTpY = " + boost::lexical_cast<std::string>(xTpY)+", yTpX = " + boost::lexical_cast<std::string>(yTpX)+", yTpY = " + boost::lexical_cast<std::string>(yTpY)+"\n");
		}
		//Ask user to go to imaging mode
		ExtraGatan::EMChangeMode("MAG1");
		
		//Calibrate beam shift - assume it's linear so a single measurement of x and y DAC shift is fine
		//[Sh1X,Sh1Y] is the beam shift in pixels [X,Y] per x DAC
		DigitalMicrograph::Result("Calling ShiftSize...\n");
		dShift = Shiftsize(20.0, &Sh1X, &Sh1Y, &Sh2X, &Sh2Y, binning, &img1, &imgCC, &img0, expo, sleeptime, _b);
		detSh = Sh1X*Sh2Y-Sh2X*Sh1Y;//determinant
		xShpX = Sh2Y/detSh;//EMSetBeamShift(xShpX,xShpY) shifts the beam 1 x-pixel
		DigitalMicrograph::Result("xShpX = " + boost::lexical_cast<std::string>(xShpX)+"\n");
		xShpY = -Sh1Y/detSh;
		yShpX = -Sh2X/detSh;//EMSetBeamShift(yShpX,yShpY) shifts the beam 1 y-pixel
		yShpY = Sh1X/detSh;

		//Save them to the global tag group
		DigitalMicrograph::Result("Saving xShpX etc to persistent tags...\n");
		DigitalMicrograph::TagGroupSetTagAsFloat(Persistent, (Tag_Path+"xShpX").c_str(), xShpX);
		DigitalMicrograph::TagGroupSetTagAsFloat(Persistent, (Tag_Path+"xShpY").c_str(), xShpY);
		DigitalMicrograph::TagGroupSetTagAsFloat(Persistent, (Tag_Path+"yShpX").c_str(), yShpX);
		DigitalMicrograph::TagGroupSetTagAsFloat(Persistent, (Tag_Path+"yShpY").c_str(), yShpY);

		bool print_xshpx = true;
		try
		{
			DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (settings_Tag_path + "Print xShpX etc").c_str(), &print_xshpx);
		}
		catch (...) {		}
		if (print_xshpx == true)
		{
			DigitalMicrograph::Result("xShpX = " + boost::lexical_cast<std::string>(xShpX)+", xShpY = " + boost::lexical_cast<std::string>(xShpY)+", yShpX = " + boost::lexical_cast<std::string>(yShpX)+", yShpY = " + boost::lexical_cast<std::string>(yShpY)+"\n");
		}
		
		long X0, Y0;
		//Centre beam shift
		CalibrateAcquisition.AcquireImage2(img1); //getting an image of the beam in initial position
		DigitalMicrograph::ImageDataChanged(img1);
		//find the position of the max intensity
		maxval = ExtraGatan::Max(img1);
		ExtraGatan::PixelPos(img1, maxval, &X0, &Y0, false);

		DigitalMicrograph::Result("About to centre the beam\n");
		ExtraGatan::EMBeamCentre(Tag_Path, img1);//this puts the beam in the centre of the image, independent of initial beam position
		DigitalMicrograph::Sleep(sleeptime);
		//Get shift and tilt values with the beam now set to the cetnre of the image.
		ExtraGatan::EMGetBeamShift(&ShiftX0, &ShiftY0);
		DigitalMicrograph::Result("Centred beam shift : "+boost::lexical_cast<std::string>(ShiftX0)+","+boost::lexical_cast<std::string>(ShiftY0)+"\n");

		//Map shift as a function of tilt
		ExtraGatan::EMGetBeamTilt(&TiltX0, &TiltY0);
		DigitalMicrograph::Result("Initial beam tilt: "+boost::lexical_cast<std::string>(TiltX0)+","+boost::lexical_cast<std::string>(TiltY0)+"\n");

		CalibrateAcquisition.AcquireImage2(img1);
		DigitalMicrograph::ImageDataChanged(img1);
		img0 = DigitalMicrograph::ImageClone(img1); // reference image
		DigitalMicrograph::ImageDataChanged(img0);
		DigitalMicrograph::Result("Set up reference image...\n");
		/////////////////////////
		//number of points +/- to measure

		nTilts = 8;
		DigitalMicrograph::TagGroupSetTagAsFloat(Persistent, (Tag_Path+"nCals").c_str(), nTilts);
		int ii, jj;
		long pt=0;
		long nPts = ((2*nTilts)+1)*((2*nTilts)+1);

		//set up arrays holding shift calibration
		Xsh = DigitalMicrograph::RealImage("X-Shift with tilt",4,(2*nTilts+1),(2*nTilts)+1);
		Ysh = DigitalMicrograph::RealImage("Y-shift with tilt",4,(2*nTilts)+1,(2*nTilts)+1);
		TiltCal = DigitalMicrograph::RealImage("Shift calibration", 4, (2*nTilts)+1, (2*nTilts)+1, 2);

		Gatan::PlugIn::ImageDataLocker XshLock(Xsh);
		float* XshPix;
		XshPix = (float*) XshLock.get();

		Gatan::PlugIn::ImageDataLocker YshLock(Ysh);
		float* YshPix;
		YshPix = (float*) YshLock.get();

		Gatan::PlugIn::ImageDataLocker TiltCalLock(TiltCal);
		float* TiltCalPix;
		TiltCalPix = (float*) TiltCalLock.get();

		GetCoordsFromNTilts(nTilts, pt, ii, jj);
		//increment in position of disc, in pixels
		tIncX = (_r/(2*(float)nTilts))*0.8; //max beam tilt is 80% of diffraction pattern width from centre
		tIncY = (_b/(2*(float)nTilts))*0.8; //max beam tilt is 80% of diffraction pattern height from centre

		DigitalMicrograph::Result("tIncX = " + boost::lexical_cast<std::string>(tIncX)+", tIncY = " + boost::lexical_cast<std::string>(tIncY)+"\n");
		//Set the initial tilt DAC values
		tX = TiltX0 + (long)((ii*xTpX + jj*yTpX)*tIncX);
		tY = TiltY0 + (long)((ii*xTpY + jj*yTpY)*tIncY);
		//move the beam
		ExtraGatan::ABSTilt(tX, tY);
		DigitalMicrograph::Sleep(sleeptime);
		CalibrateAcquisition.AcquireImage2(img1);
		DigitalMicrograph::ImageDataChanged(img1);

		DigitalMicrograph::Image mark;//image used to map where the beam has been tilted
		Gatan::PlugIn::ImageDataLocker marklocker;
		float* markpix;
		mark = DigitalMicrograph::RealImage("Mark", 4, img1_X, img1_Y);
		marklocker.lock(mark);
		markpix = (float*) marklocker.get();
		DigitalMicrograph::ImageGetOrCreateImageDocument(mark).ShowAtPosition(50,30);

		int m, v;
		std::string Progress;
		progress_ctrl.SetRange(0, (short)nPts);//settiong the plugin progress bar.
		start = clock();
		while (pt<nPts)
		{
			//set tilt
			GetCoordsFromNTilts(nTilts, pt, ii, jj);
			Progress = "Please wait..."+boost::lexical_cast<std::string>(ExtraGatan::round(((float)pt/nPts)*100))+"%";
			DigitalMicrograph::OpenAndSetProgressWindow("Calibrating Beam Tilts", Progress.c_str(), "");
			progress_ctrl.SetPos(pt);
			tX = TiltX0 + (long)((ii*xTpX + jj*yTpX)*tIncX);//setting up the DAC shift to be sent to the microscope to move the beam in the x-direction(pixels)
			tY = TiltY0 + (long)((ii*xTpY + jj*yTpY)*tIncY);
			ExtraGatan::ABSTilt(tX, tY);
			DigitalMicrograph::Sleep(sleeptime);
			//get image
			CalibrateAcquisition.AcquireImage2(img1);
			DigitalMicrograph::Sleep(sleeptime);
			DigitalMicrograph::ImageDataChanged(img1);
			ExtraGatan::tert(img1, 50);//Remove outliers by comparing with an identical image with median filer applied
			//measure beam position
			maxval = ExtraGatan::Max(img1);
			ExtraGatan::PixelPos(img1, maxval, &x, &y, false); //putting beam position coords into x and y
			//mark and save it
			for(m=-3; m<3; m++)
			{
				for(v=-3; v<3; v++) //this marks a square on the mark image
				{
					markpix[x + m + (y+v)*img1_X] = 100;
				}
			}
			DigitalMicrograph::ImageDataChanged(mark);
			dx = x - (_r / 2); //getting the difference in the position in the x-direction from the centre of the image (in pixels)
			dy = y - (_b / 2);
			XshPix[(ii + nTilts) + (jj + nTilts)*((2 * nTilts) + 1)] = -dx; //NB negative of measured value so shift cancels tilt
			YshPix[(ii + nTilts) + (jj + nTilts)*((2 * nTilts) + 1)] = -dy; //saving the difference in beam position (both x and y-direction) on the image, for this specific beam poisiton (DAC) to images XSh & YSh
			pt++;
		}
		CalibrateAcquisition.acqsource->FinishAcquisition();
		finish = clock() - start;
		DigitalMicrograph::Result("Acquisition while loop time = " + boost::lexical_cast<std::string>(((float)finish) / CLOCKS_PER_SEC) + " seconds\n");
		//Save XSh and YSh for later
		long widthSh, heightSh;
		DigitalMicrograph::Get2DSize(Xsh, &widthSh, &heightSh);//getting the size of the Xsh and Ysh images

		//loading settings information to decide whether to show the Xsh Ysh images
		bool display_xsh = false;
		bool display_ysh = false;
		try
		{
			DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (settings_Tag_path + "Display XSh").c_str(), &display_xsh);
			DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (settings_Tag_path + "Display YSh").c_str(), &display_ysh);
		}
		catch (...)	{	}
		if (display_xsh == true){
			DigitalMicrograph::ImageGetOrCreateImageDocument(Xsh).ShowAtPosition(25, 30);	}
		if (display_ysh == true){
			DigitalMicrograph::ImageGetOrCreateImageDocument(Ysh).ShowAtPosition(45, 30);	}

		//XSh and YSh are temporary images/ intermediate images that are put together
		//into the TiltCal image, a 3D image with stack height 2
		DigitalMicrograph::Result("Making tiltcal image...\n");
		for(i=0; i<(((2*nTilts)+1)*((2*nTilts)+1)); i++)
		{
			TiltCalPix[i] = XshPix[i];
			TiltCalPix[i + widthSh*heightSh] = YshPix[i];
		}
		//Add tags & save Calibration (stack of 2 images)
		DigitalMicrograph::TagGroup TiltCalTags;
		TiltCalTags = DigitalMicrograph::ImageGetTagGroup(TiltCal);
		DigitalMicrograph::Result("Saving tags to TiltCal image...\n");
		//Info:
		DigitalMicrograph::TagGroupSetTagAsString(TiltCalTags,"Info:Date", datetimestring);
		DigitalMicrograph::TagGroupSetTagAsFloat(TiltCalTags,"Info:Camera Length", CamL);
		DigitalMicrograph::TagGroupSetTagAsFloat(TiltCalTags,"Info:Magnification", mag);
		DigitalMicrograph::TagGroupSetTagAsFloat(TiltCalTags,"Info:Alpha", alpha);
		DigitalMicrograph::TagGroupSetTagAsFloat(TiltCalTags,"Info:Spot size", spot);
		DigitalMicrograph::TagGroupSetTagAsFloat(TiltCalTags,"Info:Disc Radius",Rr);
		//Tilts:
		DigitalMicrograph::TagGroupSetTagAsFloat(TiltCalTags,"Tilts:xTpX",xTpX);
		DigitalMicrograph::TagGroupSetTagAsFloat(TiltCalTags,"Tilts:xTpY",xTpY);
		DigitalMicrograph::TagGroupSetTagAsFloat(TiltCalTags,"Tilts:yTpX",yTpX);
		DigitalMicrograph::TagGroupSetTagAsFloat(TiltCalTags,"Tilts:yTpY",yTpY);
		//Shifts:
		DigitalMicrograph::TagGroupSetTagAsFloat(TiltCalTags,"Shifts:xShpX",xShpX);
		DigitalMicrograph::TagGroupSetTagAsFloat(TiltCalTags,"Shifts:xShpY",xShpY);
		DigitalMicrograph::TagGroupSetTagAsFloat(TiltCalTags,"Shifts:yShpX",yShpX);
		DigitalMicrograph::TagGroupSetTagAsFloat(TiltCalTags,"Shifts:yShpY",yShpY);
		//Save Calibration
		std::string pathname=DigitalMicrograph::PathConcatenate( DigitalMicrograph::GetApplicationDirectory("common_app_data",0),"Reference Images\\");
		std::string nameImg=pathname+"D-ED_TiltCal_A"+boost::lexical_cast<std::string>(alpha)+"_B"+boost::lexical_cast<std::string>(binning)+"_C"+boost::lexical_cast<std::string>(CamL)+".dm4";
		DigitalMicrograph::TagGroupSetTagAsString(Persistent, (Tag_Path+"Calibration file path").c_str(),pathname);
		DigitalMicrograph::TagGroupSetTagAsString(TiltCalTags,"Info:Path",pathname);
		DigitalMicrograph::Result("Tags Saved\n");
		if(DigitalMicrograph::DoesFileExist(nameImg))
		{
			DigitalMicrograph::DeleteFileA(nameImg);
		}
		DigitalMicrograph::SaveAsGatan(TiltCal,nameImg);

		bool display_calibration = false;
		try
		{
			DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (settings_Tag_path + "Display Calibration Image").c_str(), &display_calibration);
		}
		catch (...)	{	}
		if (display_calibration == true)
		{
			TiltCal.GetOrCreateImageDocument().Show();
		}
	}
	catch(...)
	{ // We're here because an error happened, stop the acquisition
		CalibrateAcquisition.acqsource->FinishAcquisition();
		DigitalMicrograph::OkDialog("An unexpected error has occured during calibration!");
		return;
	}
	// tidy up
	// reset tilts to original values
	ExtraGatan::ABSTilt(TiltX0, TiltY0);
	DigitalMicrograph::Result("Tilts reset to origianl values\n");
	
	DigitalMicrograph::DeleteImage(img0);
	DigitalMicrograph::CloseImage(img0);//may not make a difference
	DigitalMicrograph::DeleteImage(img1);
	DigitalMicrograph::DeleteImage(imgCC);
	//End of main program
	DigitalMicrograph::Result("Calibration complete, dingdong\n\n");
}