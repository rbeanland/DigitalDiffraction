#include "stdafx.h"
#include "DED_Collect.h"
void Collect::GetCoordsFromNTilts(long nTilts,long currentPoint, int &i, int &j)//based on nTilts(user input) and current pt on stack image, npts given by ntilts
{
	int side = 2*nTilts+1;
	j = floor(((double)currentPoint)/((double)side))-nTilts;
	i = ((currentPoint % side) - nTilts)*pow((float)(-1),(int)(j % 2));//NB % means modulo, flips sign every row
}

void Collect::TestDia()
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
	long sem;
	sem = DigitalMicrograph::NewSemaphore();
	try
	{

		DigitalMicrograph::FloatingModelessDialog("Testing dialog", "OK", sem);
		DigitalMicrograph::Result("sem = " + boost::lexical_cast<std::string>(sem)+"\n");


		DigitalMicrograph::GrabSemaphore(sem);
		DigitalMicrograph::ReleaseSemaphore(sem);
		DigitalMicrograph::FreeSemaphore(sem);
	}
	catch (...)
	{
		DigitalMicrograph::ReleaseSemaphore(sem);
		DigitalMicrograph::FreeSemaphore(sem);
		DigitalMicrograph::Result("In catch\n");
	}

	DigitalMicrograph::Result("passed\n");
}

bool Collect::LoadTiltCalibImages(DigitalMicrograph::Image *TiltCal, DigitalMicrograph::Image *Xsh, DigitalMicrograph::Image *Ysh)
{
	file_name="D-ED_TiltCal_A"+boost::lexical_cast<std::string>(alpha)+"_B"+boost::lexical_cast<std::string>(binning)+"_C"+boost::lexical_cast<std::string>(CamL)+".dm4";
	std::string filepath=pathname+file_name;

	long temp1, temp2;

	if(DigitalMicrograph::DoesFileExist(filepath))
	{
		*TiltCal = DigitalMicrograph::NewImageFromFile(filepath);
		DigitalMicrograph::Get3DSize(*TiltCal, &nCals, &temp1, &temp2);
		nCals = (nCals-1)/2;
	}
	if (!DigitalMicrograph::DoesFileExist(filepath))
	{
		DigitalMicrograph::OkDialog((filepath+" not found").c_str());
		DigitalMicrograph::Result("No Calibration Image Found\n");
		return(false);
	}

	*Xsh = DigitalMicrograph::RealImage("X-shift with tilt", 4, (2*nCals)+1, (2*nCals)+1);
	*Ysh = DigitalMicrograph::RealImage("Y-shift with tilt", 4, (2*nCals)+1, (2*nCals)+1);
	
	XshLock.lock(*Xsh);//putting Xsh image into xshlock locker
	XshPix = (float*) XshLock.get();//pointing to start of locker array
	
	YshLock.lock(*Ysh);
	YshPix = (float*) YshLock.get();
	
	TiltLock.lock(*TiltCal);
	TiltCalPix = (float*) TiltLock.get();
	
	int i;
	for(i=0; i<(((2*nCals)+1)*((2*nCals)+1));i++)//copying(loading) calibration images into new images
	{
		XshPix[i] = TiltCalPix[i];
		YshPix[i] = TiltCalPix[(int)(i + (((2 * nCals) + 1)*((2 * nCals) + 1)))];
	}
	//for(i=0; i<(((2*nCals)+1)*((2*nCals)+1));i++)
	//{
	//	
	//}
	DigitalMicrograph::Result("Loaded Xsh and Ysh images\n");
	return (true);

}

bool Collect::LoadTiltCalibTagInfo(DigitalMicrograph::Image* TiltCal)
{
	DigitalMicrograph::TagGroup TiltCalTags;
	TiltCalTags = DigitalMicrograph::ImageGetTagGroup(*TiltCal);

	DigitalMicrograph::TagGroupGetTagAsFloat(TiltCalTags,"Info:Alpha",&alpha); //not in script?
	DigitalMicrograph::TagGroupGetTagAsFloat(TiltCalTags,"Info:Disc Radius",&Rr);
	DigitalMicrograph::TagGroupGetTagAsFloat(TiltCalTags,"Info:Spot size",&spot);

	DigitalMicrograph::TagGroupGetTagAsFloat(TiltCalTags,"Tilts:xTpX",&xTpX);
	DigitalMicrograph::TagGroupGetTagAsFloat(TiltCalTags,"Tilts:xTpY",&xTpY);
	DigitalMicrograph::TagGroupGetTagAsFloat(TiltCalTags,"Tilts:yTpX",&yTpX);
	DigitalMicrograph::TagGroupGetTagAsFloat(TiltCalTags,"Tilts:yTpY",&yTpY);

	DigitalMicrograph::TagGroupGetTagAsFloat(TiltCalTags,"Shifts:xShpX",&xShpX);
	DigitalMicrograph::TagGroupGetTagAsFloat(TiltCalTags,"Shifts:xShpY",&xShpY);
	DigitalMicrograph::TagGroupGetTagAsFloat(TiltCalTags,"Shifts:yShpX",&yShpX);
	DigitalMicrograph::TagGroupGetTagAsFloat(TiltCalTags,"Shifts:yShpY",&yShpY);

	DigitalMicrograph::Result("Tag information loaded...\n");
	return (true);
}

void Collect::DoCollection()
{
	DigitalMicrograph::Result("In DoCollection\n");
	if(!ExtraGatan::CheckCamera()){DigitalMicrograph::OkDialog("No Camera Detected\nPlease ensure the camera is inserted"); return;}

	Camb = 2672;
	Camr = 2688;
	Persistent=DigitalMicrograph::GetPersistentTagGroup();
	_t=0;
	_l=0;
	_b=Camb/binning;
	_r=Camr/binning;
	CamL=ExtraGatan::EMGetCameraLength();
	mag=ExtraGatan::EMGetMagnification();
	DigitalMicrograph::Result("Finished initial setup\n");
//Main program
	DigitalMicrograph::Result("Rr = " + boost::lexical_cast<std::string>(Rr)+"\n");
	//Check microscope status before starting
	ExtraGatan::EMChangeMode("DIFF");

	//Set up parameters with user input
	alpha = 3;
	if (!DigitalMicrograph::GetNumber("Alpha?", alpha, &alpha))
	{
		return; 
	}

	//Check Calibration - may need to add more
	// add check.
	std::string Tag_Path;
	Tag_Path = "DigitalDiffraction:Alpha="+boost::lexical_cast<std::string>(alpha)+":Binning="+boost::lexical_cast<std::string>(binning)+":CamL="+boost::lexical_cast<std::string>(CamL)+"mm:";

	pathname=DigitalMicrograph::PathConcatenate( DigitalMicrograph::GetApplicationDirectory("common_app_data",0),"Reference Images\\");
	DigitalMicrograph::Result("About to load calibration images...\n");
	if(!LoadTiltCalibImages(&TiltCal, &Xsh, &Ysh)) // attempt to load calibration images
	{
		DigitalMicrograph::OkDialog("Could not load Calibration Images\nPlease run calibration");
		DigitalMicrograph::Result("Collection stopped due to loading calibration error");
		return;
	}
	DigitalMicrograph::Result("About to load tag info...\n");
	if(!LoadTiltCalibTagInfo(&TiltCal))
	{
		DigitalMicrograph::OkDialog("Could not load Calibration Image Tag Information\nPlease run calibration or try changing path");
		DigitalMicrograph::Result("Collection stppped due to loading calibration tag information error");
		return;
	}


	DigitalMicrograph::ImageDocumentShowAtPosition(DigitalMicrograph::ImageGetOrCreateImageDocument(TiltCal), 15, 30);

	xCal = (0.8*_r)/(2*nCals); // distance between calibration points in pixels
	yCal = (0.8*_b)/(2*nCals);
	//Get nTilts
	nTilts = 3;
	prompt = "Number of beam tilts (+ & -): ";
	if(!DigitalMicrograph::GetNumber(prompt.c_str(), nTilts, &nTilts))
	{
		return;
	}
	tInc = Rr*0.8; // may need to change
	if(nTilts*tInc > nCals*xCal) //NB work on smallest camera dimension
	{
		nTilts=floor((nCals*xCal)/tInc);
	}
	DigitalMicrograph::Result("nTilts: "+boost::lexical_cast<std::string>(nTilts)+"\n");

	nPts = (2*nTilts+1)*(2*nTilts+1);
	DigitalMicrograph::Result("nPts: "+boost::lexical_cast<std::string>(nPts)+"\n");

	//Get initial state
	DigitalMicrograph::Result("Getting initial tilt and shift...\n");
	ExtraGatan::EMGetBeamTilt(&TiltX0, &TiltY0);
	//may need to add tag info here
	DigitalMicrograph::Result("Initial beam tilts: TiltX = "+boost::lexical_cast<std::string>(TiltX0)+", TiltY = "+boost::lexical_cast<std::string>(TiltY0)+"\n");
	ExtraGatan::EMGetBeamShift(&ShiftX0, &ShiftY0);
	DigitalMicrograph::Result("Initial beam shifts: ShiftX = "+boost::lexical_cast<std::string>(ShiftX0)+", ShiftY = "+boost::lexical_cast<std::string>(ShiftY0)+"\n");


	//Stop any current camera viewer
	try
	{
		DigitalMicrograph::Result("Stopping any current camera viewer...\n");
		Gatan::CM::StopCurrentCameraViewer(true);
	}
	catch (...)
	{
		DigitalMicrograph::OkDialog("Couldn't stop camera properly, try again!");
		return;
	}

	DigitalMicrograph::Result("Stopped any current camera viewer");


	//Start the camera running in fast mode
	//Use current camera
	try
	{
		DigitalMicrograph::Sleep(sleeptime);
		img1 = ExtraGatan::Acquire(binning, &quit, &success, expo);
		img1.DataChanged();
		DigitalMicrograph::UpdateImage(img1);
		DigitalMicrograph::ImageDocumentShowAtPosition(DigitalMicrograph::ImageGetOrCreateImageDocument(img1), 15, 30); //Returns an image document containing the image, creating one if necessary.
		DigitalMicrograph::SetWindowSize(img1, 200, 200);// may need to edit values
		long data_type = DigitalMicrograph::GetDataType(img1);
		DigitalMicrograph::Get2DSize(img1, &imgX, &imgY);

		//Create 3D destination data stack
		DigitalMicrograph::Result("Creating CBED 3D stack image...\n");
		CBED = DigitalMicrograph::RealImage("CBED Stack", 4, imgX, imgY, nPts);
		CBEDLock.lock(CBED);
		CBEDPix = (float*)CBEDLock.get();

		DigitalMicrograph::Result("Making it 0...\n");
		int ii;
		for (ii = 0; ii < (imgX*imgY*nPts); ii++)
		{
			CBEDPix[ii] = 0;
		}
		
		DigitalMicrograph::ImageDocumentShowAtPosition(DigitalMicrograph::ImageGetOrCreateImageDocument(CBED), 230, 30); //Returns an image document containing the image, creating one if necessary.
		DigitalMicrograph::SetWindowSize(CBED, 200, 200);// may need to edit values

		//Go to first point
		int iii, jjj;
		long pt = 0;
		GetCoordsFromNTilts(nTilts, pt, iii, jjj);
		//Tilts to be applied, in pixels
		pX = (float)iii*tInc;
		pY = (float)jjj*tInc;

		//Tilt to be applied, in DAC numbers
		tX = (float)TiltX0 + xTpX*pX + yTpX*pY;//final/desired DAC coordinates/position
		tY = (float)TiltY0 + xTpY*pY + yTpY*pY;
		//linear interpolation for beam shift correction
		nX = floor(pX / xCal);//xCal is distance between calibration points in pixles
		nY = floor(pY / yCal);
		nnX = nX + nCals;//pixel locations in calibration image
		nnY = nY + nCals;

		float corX, corY;

		DigitalMicrograph::Result("Interpolating...\n");
		//The 4 surrounding shift calibrations
		corX = (float)ExtraGatan::altInterpolate(&Xsh, xCal, yCal, pX, nX, pY, nY, nnX, nnY);
		corY = (float)ExtraGatan::altInterpolate(&Ysh, xCal, yCal, pX, nX, pY, nY, nnX, nnY);

		//Shift correction vector
		sX = (float)ShiftX0 + corX*xShpX + corY*yShpX; 
		sY = (float)ShiftY0 + corX*xShpY + corY*yShpY;
		DigitalMicrograph::Result("Potential failure here with shift values sent to microscope...\n");
		//Set tilt and shift
		ExtraGatan::ABSShift((long)ExtraGatan::round(sX), (long)ExtraGatan::round(sY));
		ExtraGatan::ABSTilt((long)ExtraGatan::round(tX), (long)ExtraGatan::round(tY));
		DigitalMicrograph::Sleep(sleeptime);

		//Start acquisition
		//NB define variables outside try/catch
		int iiii, jjjj;
		DigitalMicrograph::Result("Starting acquisition...\n");
	
	try
	{
		//may need to move display img1 to here
		DigitalMicrograph::UpdateImage(img1);
		while(pt<nPts)
		{

			DigitalMicrograph::OpenAndSetProgressWindow("Acquiring Images for CBED stack",(boost::lexical_cast<std::string>(ExtraGatan::round(((float)pt/nPts)*100))+"%").c_str(),"");
			DigitalMicrograph::UpdateImage(img1);
			GetCoordsFromNTilts(nTilts, pt, iiii, jjjj);
			//tX and tY as a fraction of image width
			pX = iiii*tInc;//in pixels
			pY = jjjj*tInc;

			tX = (float)TiltX0 + xTpX*pX + yTpX*pY;
			tY = (float)TiltY0 + xTpY*pX + yTpY*pY;
			//linear interpolation for beam shift correction
			nX = floor(pX/xCal);//xcal is distance between calibration points in pixels
			nY = floor(pY/yCal);
			nnX = nX+nCals;//pixel locations in calibration image
			nnY = nY+nCals;//nCals is half tiltcal img width (8)
			//The 4 surrounding shift calibrations
			corX = ExtraGatan::altInterpolate(&Xsh,xCal,yCal,pX,nX,pY,nY,nnX,nnY);
			corY = ExtraGatan::altInterpolate(&Ysh,xCal,yCal,pX,nX,pY,nY,nnX,nnY);

			sX = ShiftX0 + corX*xShpX + corY*yShpX;//may not need
			sY = ShiftY0 + corX*xShpY + corY*yShpY;//may not need
			//set tilt and shift

			ExtraGatan::ABSShift((long)ExtraGatan::round(sX),(long)ExtraGatan::round(sY));
			ExtraGatan::ABSTilt((long)ExtraGatan::round(tX),(long)ExtraGatan::round(tY));
			DigitalMicrograph::Sleep(sleeptime);

			img1 = ExtraGatan::Acquire(binning,&quit,&success,expo);
			img1.DataChanged();
			DigitalMicrograph::UpdateImage(img1);

			img1Lock.lock(img1);
			img1Pix = (float*) img1Lock.get();

			for(ii=0;ii<imgX*imgY;ii++)
			{
				CBEDPix[pt*imgX*imgY + ii] = img1Pix[ii];//copying current image into CBED stack
			}

			//std::copy(img1Pix, img1Pix + (imgX*imgY), CBEDPix + (pt*imgX*imgY));


			CBED.DataChanged(); //may not work
			DigitalMicrograph::UpdateImage(CBED); //may not work
			

			pt++;
		}//end of while
	}//end of try
	catch(...)
	{
		DigitalMicrograph::Result("Error occurred while acquiring CBED stack images\n");
		return;
	}
	DigitalMicrograph::OpenAndSetProgressWindow("Acquiring Images for CBED stack", "100%", "CBED Stack complete");

	//Put tags in stack
	CBEDGroup = DigitalMicrograph::ImageGetTagGroup(CBED);

	DigitalMicrograph::TagGroupSetTagAsString(CBEDGroup,"Info:Path",pathname);
	DigitalMicrograph::TagGroupSetTagAsFloat(CBEDGroup,"Info:Camera Length",CamL);
	DigitalMicrograph::TagGroupSetTagAsFloat(CBEDGroup,"Info:Magnification",mag);
	DigitalMicrograph::TagGroupSetTagAsFloat(CBEDGroup,"Info:Alpha",alpha);
	DigitalMicrograph::TagGroupSetTagAsFloat(CBEDGroup,"Info:Spot size",spot);
	DigitalMicrograph::TagGroupSetTagAsFloat(CBEDGroup,"Info:Disc Radius",Rr);
	
	DigitalMicrograph::TagGroupSetTagAsFloat(CBEDGroup,"Tilts:xTpX",xTpX);
	DigitalMicrograph::TagGroupSetTagAsFloat(CBEDGroup,"Tilts:xTpY",xTpY);
	DigitalMicrograph::TagGroupSetTagAsFloat(CBEDGroup,"Tilts:yTpX",yTpX);
	DigitalMicrograph::TagGroupSetTagAsFloat(CBEDGroup,"Tilts:yTpY",yTpY);
	
	DigitalMicrograph::TagGroupSetTagAsFloat(CBEDGroup,"Shifts:xShpX",xShpX);
	DigitalMicrograph::TagGroupSetTagAsFloat(CBEDGroup,"Shifts:xShpY",xShpY);
	DigitalMicrograph::TagGroupSetTagAsFloat(CBEDGroup,"Shifts:yShpX",yShpX);
	DigitalMicrograph::TagGroupSetTagAsFloat(CBEDGroup,"Shifts:yShpY",yShpY);

	//Gain and Dark ref correction
	std::string DMpathname = DigitalMicrograph::PathConcatenate(DigitalMicrograph::GetApplicationDirectory("common_app_data",false),"Reference Images\\"); //was true?
	pathname = "C:\\ProgramData\\Gatan\\Reference Images\\";
	std::string refpath = DMpathname+"D-ED_DarkReference_Binning"+boost::lexical_cast<std::string>(binning)+".dm4";
	DigitalMicrograph::Result("Loading reference images...\n");
	try
	{
		if(DigitalMicrograph::DoesFileExist(refpath))
			{
			//	Dark.operator=(DigitalMicrograph::NewImageFromFile(refpath));
			Dark = DigitalMicrograph::NewImageFromFile(refpath);
			}
		if(!DigitalMicrograph::DoesFileExist(refpath))
			{
				DigitalMicrograph::OkDialog("No dark reference image found, program will now stop");
				return;
			}
	}
	catch(...)
	{
		DigitalMicrograph::Result("Failed to load reference images\n");
	}

	try
	{
		refpath=DMpathname+"D-ED_GainReference_Binning"+boost::lexical_cast<std::string>(binning)+".dm4";
		if(DigitalMicrograph::DoesFileExist(refpath))
			{
				Gain = DigitalMicrograph::NewImageFromFile(refpath);
			}
		if(!DigitalMicrograph::DoesFileExist(refpath))
			{
				DigitalMicrograph::OkDialog("No gain reference image found, program will now stop");
				return;
			}
	}
	catch(...)
	{

	}
	

	DigitalMicrograph::ImageDocumentShowAtPosition(DigitalMicrograph::ImageGetOrCreateImageDocument(Dark), 230, 30); 

	DigitalMicrograph::ImageDocumentShowAtPosition(DigitalMicrograph::ImageGetOrCreateImageDocument(Gain), 230, 30);


	pt=0;
	DigitalMicrograph::Result("Correcting images using reference images");
	Gatan::PlugIn::ImageDataLocker DarkLock(Dark);
	float* DarkPix = (float*) DarkLock.get();
	Gatan::PlugIn::ImageDataLocker GainLock(Gain);
	float* GainPix = (float*) GainLock.get();
//	for (pt = 0; pt < nPts; pt++) // going through all stack images
	while (pt<nPts)
	{
		DigitalMicrograph::OpenAndSetProgressWindow("Correecting CBED stack images using reference images", (boost::lexical_cast<std::string>(ExtraGatan::round(((float)pt / nPts) * 100)) + "%").c_str(), "");
		for (ii=0; ii<imgX*imgY; ii++)
			{
				CBEDPix[pt*imgX*imgY + ii]-=DarkPix[ii];//correcting each pixel
				CBEDPix[pt*imgX*imgY + ii]/=GainPix[ii];
			}
		pt++;
	}
	float Min, Max;
	DigitalMicrograph::ImageCalculateMinMax(CBED,1,0,&Min,&Max);
	DigitalMicrograph::SetLimits(CBED,Min,Max); // may not work

	DigitalMicrograph::Result("Acquisition complete: ding, dong\n\n");

	if(DigitalMicrograph::OkCancelDialog("Would you like to save the CBED stack image?"))
	{
		if(DigitalMicrograph::DoesFileExist(DMpathname+"CBED_Stack.dm4"))
		{
			DigitalMicrograph::DeleteFileA(DMpathname+"CBED_Stack.dm4");
		}
		DigitalMicrograph::SaveImage(CBED,(DMpathname+"CBED_Stack.dm4"));
		DigitalMicrograph::Result("CBED stack save file path: " + DMpathname + "\n");
	}
	//reset tilts to original values
	ExtraGatan::ResetBeam();
	DigitalMicrograph::Result("Beam reset to inital position");
	}
	catch (...)
	{
		DigitalMicrograph::Result("Error has occurred\n");
	}
	//End of main program
	return;
}
