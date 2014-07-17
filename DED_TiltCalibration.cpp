#include "stdafx.h"
#include "DED_TiltCalibration.h"

void Calibrate::GetCoordsFromNTilts(long nTilts,long currentPoint, int &i, int &j)
{
	int side = 2*nTilts+1;
	j = floor(((double)currentPoint)/((double)side))-nTilts;
	i = ((currentPoint % side) - nTilts)*pow((float)(-1),(int)(j % 2));//NB % means modulo, flips sign every row
}

void Calibrate::DoCalibration()
{
	DigitalMicrograph::Result("In DoCalibration\n");
	DigitalMicrograph::TagGroup Persistent; //doesn't know which is default
	if(!ExtraGatan::CheckCamera()){DigitalMicrograph::OkDialog("No Camera Detected\nPlease ensure the camera is inserted"); return;}
	Persistent = DigitalMicrograph::GetPersistentTagGroup();
	_b = Camb/binning;
	_r = Camr/binning;
	float spot = ExtraGatan::EMGetSpotSize() + 1;
	float mag = ExtraGatan::EMGetMagnification();
	double CamL = ExtraGatan::EMGetCameraLength();
	float alpha = 3;
	if (!DigitalMicrograph::GetNumber("Alpha?", alpha, &alpha))
	{
		return; //maybe add error handler?? Maybe throw as dialogue box??
	}
	std::string Tag_Path;
	Tag_Path = "DigitalDiffraction:Alpha="+boost::lexical_cast<std::string>(alpha)+":Binning="+boost::lexical_cast<std::string>(binning)+":CamL="+boost::lexical_cast<std::string>(CamL)+"mm:";
	long f=0;
	char date[100], time[100];
	std::string datestring, timestring;
	DigitalMicrograph::GetDate(f,date,100);
	DigitalMicrograph::GetTime(f,time,100);
	int i;
	for(i=0; i<100; i++)
	{
		datestring += date[i];
		timestring += time[i];
	}
	std::string datetimestring=datestring+"_"+timestring;

	DigitalMicrograph::Result("\nStarting tilt calibration "+datetimestring+"\n");

	DigitalMicrograph::TagGroupSetTagAsString(Persistent, (Tag_Path+"Date").c_str(), datetimestring);
	DigitalMicrograph::TagGroupSetTagAsFloat(Persistent, (Tag_Path+"Spot size").c_str(), spot);

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
		return; // add error handling here
	}

	try
	{
		DigitalMicrograph::Sleep(sleeptime);
		img1= ExtraGatan::Acquire(binning, &quit, &success, expo);
		DigitalMicrograph::Sleep(sleeptime);
		img1.DataChanged();
		DigitalMicrograph::ImageDocumentShowAtPosition(DigitalMicrograph::ImageGetOrCreateImageDocument(img1),230,30); //Returns an image document containing the image, creating one if necessary.
		DigitalMicrograph::SetWindowSize(img1, 230, 30);

		//Set up reference image
		img0 = DigitalMicrograph::ImageClone(img1);
		img0.GetOrCreateImageDocument().ShowAtPosition(15,30);
		DigitalMicrograph::SetWindowSize(img0,200,200);
		DigitalMicrograph::SetName(img0,"Reference");

		//and cross correlation image
		imgCC = DigitalMicrograph::CrossCorrelate(img1,img0);
		DigitalMicrograph::SetName(imgCC,"Cross correlation");
		imgCC.GetOrCreateImageDocument().ShowAtPosition(445,30);
		DigitalMicrograph::SetWindowSize(imgCC, 200, 200);
	
		long img1_X, img1_Y; //size of img1
		DigitalMicrograph::Get2DSize(img1, &img1_X, &img1_Y);
		long data_type = DigitalMicrograph::GetDataType(img1);



		//Get radius of CBED disc
		long rX, rY; // don't think need????
		Rr=ExtraGatan::DiscSize(img1, &rX, &rY, 2);
		DigitalMicrograph::TagGroupSetTagAsFloat(Persistent, (Tag_Path+"Disc Radius").c_str(),Rr);



		//Set magnitude of tilt
		//dTilt gives diffraction pattern shift 1/4 of camera height
		//[T1X,T1Y] is the beam tilt in pixels [X,Y] per x DAC
		dTilt=ExtraGatan::Tiltsize(40000/CamL, &T1X, &T1Y, &T2X, &T2Y, binning, &img1, &imgCC, &img0, expo, sleeptime, _b);
	
		detT = T1X*T2Y - T2X*T1Y; // determinant
		xTpX = T2Y/detT; //EMSetBeamTilt(xTpX, xTpY) shifts the disc 1 x-pixel
		xTpY = -T1Y/detT;
		yTpX = -T2X/detT; //EMSetBeamTilt(yTpX, yTpY) shift the disc 1 y-pixel
		yTpY = T1X/detT;
		//Save them to global tag group
		DigitalMicrograph::TagGroupSetTagAsFloat(Persistent, (Tag_Path+"xTpX").c_str(), xTpX);
		DigitalMicrograph::TagGroupSetTagAsFloat(Persistent, (Tag_Path+"xTpY").c_str(), xTpY);
		DigitalMicrograph::TagGroupSetTagAsFloat(Persistent, (Tag_Path+"yTpX").c_str(), yTpX);
		DigitalMicrograph::TagGroupSetTagAsFloat(Persistent, (Tag_Path+"yTpY").c_str(), yTpY);

		//Go to imaging mode
		ExtraGatan::EMChangeMode("MAG1");

		//Calibrate beam shift - assume it's linear so a single measurement of x and y DAC shift is fine
		//[Sh1X,Sh1Y] is the beam shift in pixels [X,Y] per x DAC
		dShift = ExtraGatan::Shiftsize(20.0, &Sh1X, &Sh1Y, &Sh2X, &Sh2Y, binning, expo, sleeptime, _b);
		detSh = Sh1X*Sh2Y-Sh2X*Sh1Y;//determinant
		xShpX = Sh2Y/detSh;//EMSetBeamShift(xShpX,xShpY) shifts the beam 1 x-pixel
		xShpY = -Sh1Y/detSh;
		yShpX = -Sh2X/detSh;//EMSetBeamShift(yShpX,yShpY) shifts the beam 1 y-pixel
		yShpY = Sh1X/detSh;

		//Save them to the global tag group

		DigitalMicrograph::TagGroupSetTagAsFloat(Persistent, (Tag_Path+"xShpX").c_str(), xShpX);
		DigitalMicrograph::TagGroupSetTagAsFloat(Persistent, (Tag_Path+"xShpY").c_str(), xShpY);
		DigitalMicrograph::TagGroupSetTagAsFloat(Persistent, (Tag_Path+"yShpX").c_str(), yShpX);
		DigitalMicrograph::TagGroupSetTagAsFloat(Persistent, (Tag_Path+"yShpY").c_str(), yShpY);

		long X0, Y0;
		//Centre beam shift
		img1 = ExtraGatan::Acquire(img1, binning, &quit, &success, expo);
		img1.DataChanged();

		///////// may need to move???? to below BeamCentre
		maxval = ExtraGatan::Max(img1);
		////////
		ExtraGatan::PixelPos(img1, maxval, &X0, &Y0, false);
		//////


		ExtraGatan::EMBeamCentre(Tag_Path, img1); ///removed in previous version???

		ExtraGatan::EMGetBeamShift(&ShiftX0, &ShiftY0);
		DigitalMicrograph::Result("Centred beam shift : "+boost::lexical_cast<std::string>(ShiftX0)+","+boost::lexical_cast<std::string>(ShiftY0)+"\n");

		//Map shift as a function of tilt
		ExtraGatan::EMGetBeamTilt(&TiltX0, &TiltY0);
		DigitalMicrograph::Result("Initial beam tilt: "+boost::lexical_cast<std::string>(TiltX0)+","+boost::lexical_cast<std::string>(TiltY0)+"\n");
	


		img1 = ExtraGatan::Acquire(img1, binning, &quit, &success, expo);
		img1.DataChanged();
		img0 = DigitalMicrograph::ImageClone(img1); // reference image
		img0.DataChanged();
	
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
		//TiltCal = DigitalMicrograph::NewImage("Sift calibration", data_type, (2*nTilts)+1, (2*nTilts)+1, 2);
		TiltCal = DigitalMicrograph::RealImage("Sift calibration", 4, (2*nTilts)+1, (2*nTilts)+1, 2);

		Gatan::PlugIn::ImageDataLocker XshLock(Xsh);
		float* XshPix;
		XshPix = (float*) XshLock.get();

		Gatan::PlugIn::ImageDataLocker YshLock(Ysh);
		float* YshPix;
		YshPix = (float*) YshLock.get();

		Gatan::PlugIn::ImageDataLocker TiltCalLock(TiltCal);
		float* TiltCalPix;
		TiltCalPix = (float*) TiltCalLock.get();
	
		float sX, sY;


		GetCoordsFromNTilts(nTilts, pt, ii, jj);
		//increment in position of disc, in pixels
		tIncX = (_r/(2*(float)nTilts))*0.8; //max beam tilt is 80% of diffraction pattern width from centre
		tIncY = (_b/(2*(float)nTilts))*0.8; //max beam tilt is 80% of diffraction pattern height from centre

		tX = TiltX0 + (long)((ii*xTpX + jj*yTpX)*tIncX); //may need to chenge casting/types
		tY = TiltY0 + (long)((ii*xTpY + jj*yTpY)*tIncY);
		//tX = TiltX0 + (ii*xTpX + jj*yTpX)*tIncX;
		//tY = TiltY0 + (ii*xTpY + jj*yTpY)*tIncY;
		ExtraGatan::ABSTilt(tX, tY);
		DigitalMicrograph::Sleep(sleeptime);

		img1 = ExtraGatan::Acquire(img1, binning, &quit, &success, expo);//throw this image away
		img1.DataChanged();


		DigitalMicrograph::Image mark;
		Gatan::PlugIn::ImageDataLocker marklocker;
		float* markpix;
		mark = DigitalMicrograph::RealImage("Scan", 4, img1_X, img1_Y);
		marklocker.lock(mark);
		markpix = (float*) marklocker.get();
	
		DigitalMicrograph::ImageGetOrCreateImageDocument(mark).ShowAtPosition(15,30);

		int m, v;
		std::string Progress;

		while (pt<nPts)
		{
			//set tilt
			GetCoordsFromNTilts(nTilts, pt, ii, jj);

			Progress = "Please wait..."+boost::lexical_cast<std::string>(ExtraGatan::round(((float)pt/nPts)*100))+"%";
			DigitalMicrograph::OpenAndSetProgressWindow("Calibrating Beam Tilts", Progress.c_str(), "");

			tX = TiltX0 + (long)((ii*xTpX + jj*yTpX)*tIncX);
			tY = TiltY0 + (long)((ii*xTpY + jj*yTpY)*tIncY);
			ExtraGatan::ABSTilt(tX, tY);
			DigitalMicrograph::Sleep(sleeptime);
			//get image
			img1 = ExtraGatan::Acquire(img1, binning, &quit, &success, expo);
			DigitalMicrograph::Sleep(sleeptime);
			img1.DataChanged();
			ExtraGatan::tert(img1, 50); // may not work
			//measure beam position
			maxval = ExtraGatan::Max(img1);
			ExtraGatan::PixelPos(img1, maxval, &x, &y, false); //putting beam position coords into x and y
			//mark and save it
			for(m=-3; m<3; m++)
			{
				for(v=-3; v<3; v++) //think this marks a square on the mark image
				{
					markpix[x + m + (y+v)*img1_X] = 100;
				}
			}
			mark.DataChanged();

			dx = x - X0; //may be wrong
			dy = y - Y0; //may be wrong

			sX= ShiftX0 - (dx*xShpX + dy*yShpX);
			sY= ShiftY0 - (dx*xShpY + dy*yShpY);
			XshPix[(ii+nTilts)+(jj+nTilts)*((2*nTilts)+1)] = sX;//NB negative of measured value so shift cancels tilt
			YshPix[(ii+nTilts)+(jj+nTilts)*((2*nTilts)+1)] = sY;
			pt++;
		}
		//Save XSh and YSh for later
		long widthSh, heightSh;
		DigitalMicrograph::Get2DSize(Xsh, &widthSh, &heightSh);

		//XSh and YSh are temporary images/ intermediate images that are put together
		//into the TiltCal image, a 2d image

		for(i=0; i<(((2*nTilts)+1)*((2*nTilts)+1)); i++)
		{
			TiltCalPix[i] = XshPix[i];
			TiltCalPix[i + widthSh*heightSh] = YshPix[i];
		}
		//Gatan::PlugIn::~ImageDataLocker(TiltCalLock);

		//Add tags & save Calibration (stack of 2 images)
		DigitalMicrograph::TagGroup TiltCalTags;
		TiltCalTags = DigitalMicrograph::ImageGetTagGroup(TiltCal);
	
		DigitalMicrograph::TagGroupSetTagAsString(TiltCalTags,"Info:Date", datetimestring);
		DigitalMicrograph::TagGroupSetTagAsFloat(TiltCalTags,"Info:Camera Length", CamL);
		DigitalMicrograph::TagGroupSetTagAsFloat(TiltCalTags,"Info:Magnification", mag);
		DigitalMicrograph::TagGroupSetTagAsFloat(TiltCalTags,"Info:Alpha", alpha);
		DigitalMicrograph::TagGroupSetTagAsFloat(TiltCalTags,"Info:Spot size", spot);
		DigitalMicrograph::TagGroupSetTagAsFloat(TiltCalTags,"Info:Disc Radius",Rr);
		DigitalMicrograph::TagGroupSetTagAsFloat(TiltCalTags,"Tilts:xTpX",xTpX);
		DigitalMicrograph::TagGroupSetTagAsFloat(TiltCalTags,"Tilts:xTpY",xTpY);
		DigitalMicrograph::TagGroupSetTagAsFloat(TiltCalTags,"Tilts:yTpX",yTpX);
		DigitalMicrograph::TagGroupSetTagAsFloat(TiltCalTags,"Tilts:yTpY",yTpY);
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
		if(DigitalMicrograph::OkCancelDialog("View the Calibration?"))
		{
			TiltCal.GetOrCreateImageDocument().Show();
		}
	}
	catch(...)
	{ // We're here because an error happened, stop the acquisition
		DigitalMicrograph::OkDialog("An unexpected error has occured during calibration!");
		return;
	}
	// tidy up
	// reset tilts to original values
	ExtraGatan::ABSTilt(TiltX0, TiltY0);
	DigitalMicrograph::Result("Tilts reset to saved values\n");
	DigitalMicrograph::DeleteImage(img0);
	DigitalMicrograph::DeleteImage(img1);
	DigitalMicrograph::DeleteImage(imgCC);
	//End of main program
	DigitalMicrograph::Result("Calibration complete, dingdong\n\n");

}