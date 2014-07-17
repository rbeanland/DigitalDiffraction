#include "stdafx.h"
#include "ExtraGatan.h"
#include "DMPluginCamera.h"
#include <string>
#include <omp.h>

//#include "MyDlg1.h"

class ROICheck;

ExtraGatan::ExtraGatan() 
	
	{
		
	}

void ExtraGatan::UpdateCamera(DigitalMicrograph::Image *img_disp, double expo, int binning)
{
	bool quit, success;
	*img_disp = Acquire(*img_disp,binning,&quit,&success,expo);
	(*img_disp).DataChanged();
		//Acquire(binning,&quit,&success,expo);
}


DigitalMicrograph::Image ExtraGatan::RemoveOutliers(DigitalMicrograph::Image Img, long *t, long *l, long *b, long *r, float thr) //removes outliers caused by x-ray interference
	{
		DigitalMicrograph::Image medImg;
		DigitalMicrograph::GetSelection(Img,t,l,b,r);
		medImg = DigitalMicrograph::RealImage("Median Image",4,*r,*b);
		medImg = DigitalMicrograph::MedianFilter(Img,3,1);
		Img = ExtraGatan::tert(Img, thr);
		DigitalMicrograph::DeleteImage(medImg);
		return(Img);
	} //end of RemoveOutliers




DigitalMicrograph::Image ExtraGatan::tert(DigitalMicrograph::Image INPUT, float thr) //NOT SURE IT'S WORKING NEEDS SOME MAJOR TESTING
{
	
	Gatan::PlugIn::ImageDataLocker  inLock(INPUT);
	long IN_x, IN_y;
	DigitalMicrograph::Get2DSize(INPUT,&IN_x,&IN_y);
	float* InPix = (float*) inLock.get();
	DigitalMicrograph::Image MED = DigitalMicrograph::RealImage("Median Image",4,IN_x,IN_y);
	MED = DigitalMicrograph::MedianFilter(INPUT,3,1);
	Gatan::PlugIn::ImageDataLocker  medLock(MED); //makes an array of floats based on the pixel intensities of the two input images
	long MED_x, MED_y;
	DigitalMicrograph::Get2DSize(MED ,&MED_x,&MED_y);
	float* MedPix = (float*) medLock.get(); //points to the first member of the array to initialise it

	std::string INSTRING = "IN" + boost::lexical_cast<std::string>(IN_x) + ", " + boost::lexical_cast<std::string>(IN_y) + "\n";
	std::string MEDSTRING = "MED" + boost::lexical_cast<std::string>(MED_x) + ", " + boost::lexical_cast<std::string>(MED_y) + "\n";

	if (IN_x !=	MED_x || IN_y != MED_y) //checks the images are of the same size
	{
		DigitalMicrograph::OkDialog("The image files provided are of different sizes\n the outlier removal program cannot continue");
		DigitalMicrograph::Result("Outlier removal stopped, the data will not be processed for x-ray outliers\n");
		return(INPUT); //escapes the program if they're not
		//not sure if this is overkill, but I can't see how images of different sizes would get passed unless a user was delibrately breaking my code
		//having said that I spent 2 hours trying to find where the bug that was causing this to flag was
	}
	int pixnum = IN_y*IN_x; //determines total size of array
	int i;
	DigitalMicrograph::OpenAndSetProgressWindow("Running image reference","Removing outliers","Please wait");
	for(i=0; i<pixnum; i++)
	{
		if(abs(InPix[i]-MedPix[i]) > thr) //compares the values with the threshold value (DEFINED IN THE CLASS CONSTRUCTOR)
		{ InPix[i]=MedPix[i]; } //clears outliers
	}
	medLock.~ImageDataLocker(); //deconstruction
	inLock.~ImageDataLocker();
	return(INPUT);
} //end of tert function

DigitalMicrograph::Image ExtraGatan::Normalise(DigitalMicrograph::Image IMG1, float max)
{
	Gatan::PlugIn::ImageDataLocker  IMG1Lock(IMG1); //makes an array of floats based on the pixel intensities of the two input images
	long IMG1_x, IMG1_y;
	DigitalMicrograph::Get2DSize(IMG1,&IMG1_x,&IMG1_y);
	float* IMG1Pix = (float*) IMG1Lock.get(); //points to the first member of the array to initialise it
	int pixnum = IMG1_y*IMG1_x; //determines total size of array
	int i;
	for(i=0; i<pixnum; i++)
	{
		IMG1Pix[i] = abs(IMG1Pix[i]/=max);
	}
	IMG1Lock.~ImageDataLocker(); //deconstruction
	return(IMG1);
} //end of normalise

float ExtraGatan::Max(DigitalMicrograph::Image IMG1)
{
	Gatan::PlugIn::ImageDataLocker  IMG1Lock(IMG1); //makes an array of floats based on the pixel intensities of the two input images
	long IMG1_x, IMG1_y;
	DigitalMicrograph::Get2DSize(IMG1,&IMG1_x,&IMG1_y);
	float* IMG1Pix = (float*) IMG1Lock.get(); //points to the first member of the array to initialise it
	int pixnum = IMG1_y*IMG1_x; //determines total size of array
	int i;
	float max = FLT_MIN;
	for(i=0; i<pixnum; i++)
	{
		if(abs(IMG1Pix[i])>max)
		{ max = abs(IMG1Pix[i]);}
	}
	IMG1Lock.~ImageDataLocker(); //deconstruction
	return(max);
} //end of Max function

float ExtraGatan::Max(DigitalMicrograph::Image IMG1, long* x, long* y)
{
	Gatan::PlugIn::ImageDataLocker  IMG1Lock(IMG1); //makes an array of floats based on the pixel intensities of the two input images
	long IMG1_x, IMG1_y;
	DigitalMicrograph::Get2DSize(IMG1,&IMG1_x,&IMG1_y);
	float* IMG1Pix = (float*) IMG1Lock.get(); //points to the first member of the array to initialise it
	int pixnum = IMG1_y*IMG1_x; //determines total size of array
	int i;
	float max = abs(IMG1Pix[1]);
	int max_position;
	for(i=0; i<pixnum; i++)
	{
		if(abs(IMG1Pix[i])>max)
		{
			max = abs(IMG1Pix[i]);
			max_position = i;
		}
	}
	*y = (long) floor((float) (max_position/IMG1_x));
	*x = pixnum - (*y*IMG1_x);
	IMG1Lock.~ImageDataLocker(); //deconstruction
	return(max);
} //end of Max function



void ExtraGatan::PixelPos(DigitalMicrograph::Image IMG1, float min, long* imgX, long* imgY, bool avg)
{
	Gatan::PlugIn::ImageDataLocker  IMG1Lock(IMG1); //makes an array of floats based on the pixel intensities of the two input images
	long IMG1_x, IMG1_y;
	DigitalMicrograph::Get2DSize(IMG1,&IMG1_x,&IMG1_y);
	float* IMG1Pix = (float*) IMG1Lock.get(); //points to the first member of the array to initialise it
	int pixnum = IMG1_y*IMG1_x; //determines total size of array
	int i, j, counter;
	counter = 0;
	for(j=0; j<IMG1_y; j++)
	{
		for(i=0; i<IMG1_x; i++)
		{
			if((IMG1Pix[(IMG1_x*j) + i])==min)
			{

				if(avg == false || counter == 0)
				{
					*imgX = i; *imgY = j; counter++;
				}

				if(avg == true && counter > 0)
				{
					*imgX = ((i+*imgX)/2);
					*imgY = ((j+*imgY)/2);
					counter++;
				}

			}
		}
	}
	if(counter > 1 && avg == false)
	{
		//DigitalMicrograph::Result("More than one pixel found with the given value, this may cause the program to fail!\n");
	}
	IMG1Lock.~ImageDataLocker(); //deconstruction
}//end of Max function

std::string ExtraGatan::EMGetImagingOpticsMode() //some other guys code, I don't fully understand it, but it works
        {
               static DigitalMicrograph::Function __sFunction = (DM_FunctionToken) NULL;
               static const char *__sSignature = "string EMGetImagingOpticsMode(  )";
               Gatan::PlugIn::DM_Variant params[1];
               GatanPlugIn::gDigitalMicrographInterface.CallFunction( __sFunction.get_ptr(), 1, params, __sSignature );
               
               std::string mode = (const char *) params[0].v_c_string;
               return mode;
        } //end of ENGet...






void ExtraGatan::EMGetBeamShift(long* shiftx, long* shifty) 
        {
               static DigitalMicrograph::Function __sFunction = (DM_FunctionToken) NULL;
               static const char *__sSignature = "void EMGetBeamShift(long* , long*)";
               Gatan::PlugIn::DM_Variant params[2];
			   params[0].v_uint64 = (Gatan::uint64) shiftx;
			   params[1].v_uint64 = (Gatan::uint64) shifty;
               GatanPlugIn::gDigitalMicrographInterface.CallFunction( __sFunction.get_ptr(), 2, params, __sSignature );
			   //DigitalMicrograph::Result("PARAMS " + boost::lexical_cast<std::string>(params[0].v_uint32) + "\n");


        } //end of ENGet...

void ExtraGatan::EMGetBeamTilt(long* tiltx, long* tilty) 
        {
               static DigitalMicrograph::Function __sFunction = (DM_FunctionToken) NULL;
               static const char *__sSignature = "void EMGetBeamTilt(long*, long*)";
               Gatan::PlugIn::DM_Variant params[2];
			   params[0].v_uint64 = (Gatan::uint64) tiltx;
			   params[1].v_uint64 = (Gatan::uint64) tilty;
               GatanPlugIn::gDigitalMicrographInterface.CallFunction( __sFunction.get_ptr(), 2, params, __sSignature );



               //tiltx = params[0].v_float32_ref; //might need to be 32
			   //tilty = params[1].v_float32_ref;
        } //end of ENGet...

float ExtraGatan::EMGetSpotSize() //some other guys code, I don't fully understand it, but it works
        {
			   float size;
               static DigitalMicrograph::Function __sFunction = (DM_FunctionToken) NULL;
			  
               static const char *__sSignature = "float EMGetSpotSize()";
			   
               Gatan::PlugIn::DM_Variant params[1];
			   
               GatanPlugIn::gDigitalMicrographInterface.CallFunction( __sFunction.get_ptr(), 1, params, __sSignature );
			   
			   
               size = params[0].v_float32;
			   
			   DigitalMicrograph::Result("Returned Spot Size: " + boost::lexical_cast<std::string>(size) + "\n");
			   return(size);
        } //end of ENGet...

float ExtraGatan::EMGetMagnification()
{
			   float mag;
               static DigitalMicrograph::Function __sFunction = (DM_FunctionToken) NULL;
               static const char *__sSignature = "float EMGetMagnification()";
               Gatan::PlugIn::DM_Variant params[1];
               GatanPlugIn::gDigitalMicrographInterface.CallFunction( __sFunction.get_ptr(), 1, params, __sSignature );
			   
               mag = params[0].v_float32;
			   return(mag);
}

void ExtraGatan::GetDateAndTime(std::string* dateout, std::string* timeout)
{
	char date[11], time[9];
	int i;

	DigitalMicrograph::GetDate(0,date,11);
	DigitalMicrograph::GetTime(true,time,9);
	for(i=0; i<11; i++)
	{
		*dateout += date[i];
	}
		for(i=0; i<9; i++)
	{
		*timeout += time[i];
	}
	//DigitalMicrograph::Result((dateout+"\n"));
	//DigitalMicrograph::Result((timeout+"\n"));
}

DigitalMicrograph::Image ExtraGatan::RandMap(long inX, long inY)
{
			DigitalMicrograph::Image output;
			output = DigitalMicrograph::RealImage("RandomMap",4,inX,inY);
			Gatan::PlugIn::ImageDataLocker randmaker(output);
			float* outpix = (float*) randmaker.get();
			int i; float throwaway;
			for(i=0; i<1000; i++) {throwaway = (float)(rand()%100);}
			
			for(i=0; i<inX*inY; i++)
			{
				outpix[i] =  ((float) (rand()%2000) - 1000 );
			}
			randmaker.~ImageDataLocker();
			output.DataChanged();
			return(output);
}



DigitalMicrograph::Image ExtraGatan::RandMap(DigitalMicrograph::Image output)
{
			long inX, inY;
			DigitalMicrograph::Get2DSize(output, &inX, &inY);
			Gatan::PlugIn::ImageDataLocker randmaker(output);
			float* outpix = (float*) randmaker.get();
			int i; float throwaway;
			for(i=0; i<1000; i++) {throwaway =(float)  (rand()%100);}
			
			for(i=0; i<inX*inY; i++)
			{
				outpix[i] =  ((float) (rand()%2000) - 1000 );
			}
			randmaker.~ImageDataLocker();
			output.DataChanged();
			return(output);
}

DigitalMicrograph::Image ExtraGatan::RandMap(DigitalMicrograph::Image output, long intensity)
{
			long inX, inY;
			DigitalMicrograph::Get2DSize(output, &inX, &inY);
			Gatan::PlugIn::ImageDataLocker randmaker(output);
			float* outpix = (float*) randmaker.get();
			int i; float throwaway;
			for(i=0; i<1000; i++) {throwaway =(float)  (rand()%100);}
			
			for(i=0; i<inX*inY; i++)
			{
				outpix[i] =  ((float) (rand()%(2*intensity)) - intensity );
			}
			randmaker.~ImageDataLocker();
			output.DataChanged();
			return(output);
}


DigitalMicrograph::Image ExtraGatan::GaussianNoise(long inX, long inY,float SD, float intensity)
{
	DigitalMicrograph::Image Gauss = DigitalMicrograph::RealImage("Gaussian",4,inX,inY);
	Gatan::PlugIn::ImageDataLocker GaussLock(Gauss);
	float* GaussPix = (float*) GaussLock.get();
	int i,xpos,ypos;
	double exponent;
	for(i=0; i<inX*inY; i++)
	{
		ypos = (int) floor((float)i/inX);
		xpos = i - ypos*inX;
		exponent = (  pow((float)(xpos -(inX/2)),2)/(2*SD*SD) + pow((float)(ypos -(inY/2)),2)/(2*SD*SD) );
		GaussPix[i] = (float) intensity*(exp(-exponent));

	}


	return(Gauss);
}

DigitalMicrograph::ScriptObject ExtraGatan::GetFrameSetInfo (DigitalMicrograph::ScriptObject &Acquis)

{
    static DigitalMicrograph::Function __sFunction = (DM_FunctionToken) NULL;
    static const char *__sSignature = "ScriptObject GetFrameSetInfo( ScriptObject )";
    Gatan::PlugIn::DM_Variant params[2];
    params[1].v_object = (DM_ObjectToken) Acquis.get();
	GatanPlugIn::gDigitalMicrographInterface.CallFunction( __sFunction.get_ptr(), 2, params, __sSignature );
	return (DM_ScriptObjectToken_1Ref) params[0].v_object;
};

DigitalMicrograph::ScriptObject ExtraGatan::GetFrameSetInfoPtr (DigitalMicrograph::ScriptObject &Acquis)

{

       static DigitalMicrograph::Function __sFunction = (DM_FunctionToken) NULL;

       static const char *__sSignature = "ScriptObject GetFrameSetInfoPtr( ScriptObject * )";

 

       Gatan::PlugIn::DM_Variant params[2];

 

       params[1].v_object_ref = (DM_ObjectToken*) Acquis.get_ptr();

       GatanPlugIn::gDigitalMicrographInterface.CallFunction( __sFunction.get_ptr(), 2, params, __sSignature );

 

       return (DM_ScriptObjectToken_1Ref) params[0].v_object;

};

bool ExtraGatan::EMChangeMode(std::string mode_want)
{
	std::string mode_is = EMGetImagingOpticsMode();
	
	if(mode_is == "")
	{

	}
	bool clickedOK = true;
	bool quit = false;
	std::string Splashstring = "Please put microscope in " + mode_want + " mode";
	const char * finalSplash = finalSplash = Splashstring.c_str();
	
	while(mode_want != mode_is)
	{
		try
		{
			
			clickedOK = DigitalMicrograph::ContinueCancelDialog(finalSplash);
		}
		catch(...)
		{
			DigitalMicrograph::OkDialog("The plugin has encountered an unexpected error");
			clickedOK = false;
			quit = true;
			return(quit);
		}
		if(clickedOK==false)
		{
			DigitalMicrograph::OkDialog("Plugin stopped as user request");
			quit = true;
			return(quit);
		}
		mode_is = EMGetImagingOpticsMode();
	}
	return(quit);
}



void ExtraGatan::EMBeamCentre(std::string Tag_Path, DigitalMicrograph::Image IMG1)
{
	long ShiftX0, ShiftY0;
	DigitalMicrograph::String xShpY, yShpX, yShpY, xShpX;
	EMGetBeamShift(&ShiftX0, &ShiftY0);
	DigitalMicrograph::TagGroup PersistentTags = DigitalMicrograph::GetPersistentTagGroup();
	std::string xstringx = Tag_Path + "xShpX"; //creating string
	const char* xtagx = xstringx.c_str(); //converting to c style string, type needed by DM tag functions
	std::string xstringy = Tag_Path + "xShpY";
	const char* xtagy = xstringy.c_str();
	std::string ystringx = Tag_Path + "yShpX";
	const char* ytagx = ystringx.c_str();
	std::string ystringy = Tag_Path + "yShpY";
	const char* ytagy = ystringy.c_str();
	DigitalMicrograph::TagGroupGetTagAsString(PersistentTags,xtagx, xShpX);
	DigitalMicrograph::TagGroupGetTagAsString(PersistentTags,xtagy, xShpY);
	DigitalMicrograph::TagGroupGetTagAsString(PersistentTags,ytagx, yShpX);
	DigitalMicrograph::TagGroupGetTagAsString(PersistentTags,ytagy, yShpY);
	std::string xShpXstr = xShpX.operator std::string();//casting to std::string, converting from DM string to std string
	int xShpXint = boost::lexical_cast<int>(xShpXstr);//casting the std string to int
	std::string xShpYstr = xShpY.operator std::string();
	int xShpYint = boost::lexical_cast<int>(xShpYstr);
	std::string yShpXstr = yShpX.operator std::string();
	int yShpXint = boost::lexical_cast<int>(yShpXstr);
	std::string yShpYstr = yShpY.operator std::string();
	int yShpYint = boost::lexical_cast<int>(yShpYstr);
	long x0, y0; // coords of untilted beam
	float maxval = Max(IMG1, &x0, &y0);
	long imgx, imgy;
	DigitalMicrograph::Get2DSize(IMG1, &imgx, &imgy);
	//long x1 = imgx - x0;//x1,y1 are the no. of pixels to move [x,y]
	//long y1 = imgy - y0;
	long x1 = imgx/2 - x0;//x1,y1 are the no. of pixels to move [x,y]
	long y1 = imgy/2 - y0;
	long xCentre = (ShiftX0 + x1*xShpXint + y1*yShpXint);
	long yCentre = (ShiftY0 + x1*xShpYint + y1*yShpYint);
	//long xCentre = (x1*xShpXint + y1*yShpXint);
	//long yCentre = (x1*xShpYint + y1*yShpYint);
	DigitalMicrograph::EMBeamShift(xCentre, yCentre);
	//remember to call a sleep function in the main after calling this function
}

float ExtraGatan::round(float number)
{
	float rounded;
	if(floor(number) == floor(number+0.5))
	{ rounded = floor(number); }
	if(floor(number) == floor(number+0.5) - 1)
	{ rounded = ceil(number); }
	return(rounded);
}

DigitalMicrograph::Image ExtraGatan::CrossCorr(DigitalMicrograph::Image Cc, DigitalMicrograph::Image img1, DigitalMicrograph::Image img2)// cross correlates img1 and img2 and puts resulting image into imgcc
{
	DigitalMicrograph::Image CcTemp = DigitalMicrograph::CrossCorrelation(img1,img2);
	Gatan::PlugIn::ImageDataLocker CcLock(Cc); //creating locker for image cc
	int i;
	float CCmin, CCmax; // max and min intensities found using minmax
	float* CcPix = (float*) CcLock.get(); /// pointing to first pixel of image cc, image which is return of thos function
	Gatan::PlugIn::ImageDataLocker CcTempLock(CcTemp);// new locker for image tempcc, crosscorrelation of img1 and img2
	float* CcTempPix = (float*) CcTempLock.get();//pointing to first pixel of new temp image
	long img1X, img1Y, img2X, img2Y; //size variables of the two images
	DigitalMicrograph::Get2DSize(img1,&img1X, &img1Y);
	DigitalMicrograph::Get2DSize(img2,&img2X, &img2Y);
	for(i=0; i<img1X*img1Y; i++)
	{
		//img1X*img1Y-
		CcPix[i] = CcTempPix[img1X*img1Y-i]; // changing the pixel values in cc to the values of crosscorrelation of img1 and 2 using temp image
	}
	CcLock.~ImageDataLocker(); // deconstructing array
	CcTempLock.~ImageDataLocker(); //deconstructing array
	DigitalMicrograph::ImageCalculateMinMax(Cc,0,0,&CCmin,&CCmax); //Calculating minmax intensities of cc image
	DigitalMicrograph::SetLimits(Cc,CCmin,CCmax);// everything below CCmin will be black, above CCmax white
	return(Cc);
	
}

DigitalMicrograph::Image ExtraGatan::CrossCorr2(DigitalMicrograph::Image Cc, DigitalMicrograph::Image img1, DigitalMicrograph::Image img2)
{
	DigitalMicrograph::Image CcTemp = DigitalMicrograph::CrossCorrelation(img1,img2);
	Gatan::PlugIn::ImageDataLocker CcLock(Cc);
	int i;
	float CCmin, CCmax;
	float* CcPix = (float*) CcLock.get();
	Gatan::PlugIn::ImageDataLocker CcTempLock(CcTemp);
	float* CcTempPix = (float*) CcTempLock.get();
	long img1X, img1Y, img2X, img2Y;
	DigitalMicrograph::Get2DSize(img1,&img1X, &img1Y);
	DigitalMicrograph::Get2DSize(img2,&img2X, &img2Y);
	for(i=0; i<img1X*img1Y; i++)
	{
		//img1X*img1Y-
		CcPix[i] = CcTempPix[i];
	}
	CcLock.~ImageDataLocker();
	CcTempLock.~ImageDataLocker();
	DigitalMicrograph::ImageCalculateMinMax(Cc,0,0,&CCmin,&CCmax);
	DigitalMicrograph::SetLimits(Cc,CCmin,CCmax);
	return(Cc);
	
}

float ExtraGatan::AverageTemp(DigitalMicrograph::Image IMG)
{
	Gatan::PlugIn::ImageDataLocker AvgLock(IMG);
	float* AvgPix = (float*) AvgLock.get();
	float running = 0;
	float average;
	long imgX, imgY;
	int i;
	DigitalMicrograph::Get2DSize(IMG,&imgX,&imgY);
	for(i=0; i<imgX*imgY; i++)
	{
		running += AvgPix[i];
	}
	average = running/(imgX*imgY);
	return(average);
}

float ExtraGatan::DiscSize(DigitalMicrograph::Image IMG, long *x0, long *y0, int mode) //finds a disc size, please use mode 2 if in doubt.
{
	 
	 long imgX,imgY;
	 DigitalMicrograph::Get2DSize(IMG, &imgX,&imgY);
	 DigitalMicrograph::Image Disc = DigitalMicrograph::ImageClone(IMG);
	 float  max;
	 max = Max(IMG); //Max is in ExtraGatan, finds max pixel intesity of input image.
	 	 long Rmax;
	 //here
	 if(imgX>imgY) // finding max radius based on x,y image size
	 {
		Rmax=(long) (round(imgY/6));
	 }
	 else
	 {
		Rmax=(long) (round(imgX/6)); //ln235
	 }

	 Gatan::PlugIn::ImageDataLocker DiscLock(Disc); //Creating array of floats for image Disc based on pixel intensity
	 float* discpix = (float*) DiscLock.get(); // points to first member of the array to initialise it
	 long _x0,_y0,discX,discY,MaxDiscX, MaxDiscY;
	 float Rr=0;
	 int i;
	 float BestCc=0;
	 float CCmax;
	float actrad;

	 DigitalMicrograph::Image Cc, iDisc,Cctrans;
	 Cc = DigitalMicrograph::RealImage("Cross Correlation", 4,imgX,imgY);//making 'blank' image
	 /*DigitalMicrograph::DisplayAt(Cc,660,30);
	 DigitalMicrograph::SetWindowSize(Cc,200,200);
	 DigitalMicrograph::SetColorMode(Cc,4);*/
	 iDisc = DigitalMicrograph::ImageClone(Disc);
	 MaxDiscX = imgX/100; //think max disc size, smaller than image
	 MaxDiscY = imgY/100;
	 int threadnum;//not used????
	 std::vector<float> CCmaxs;//creating a vector of floats for CCmax??
	 std::vector<long> xpos, ypos;//creating vectors of longs????
	PixelPos(IMG,max,&discX,&discY,false); // finding pixel position of disc based on max, the highest pixel intensity of the image
	 if(mode == 1)
	 {
	  _x0 = imgX/2;
	  _y0 = imgY/2;
	 }
	 if(mode == 2)
	 {
	  PixelPos(IMG,max,&_x0, &_y0,false); // as in script, if(cc>bestcc) ln244
	 }
	 if(mode==1 || mode ==2)
	 {
		#pragma omp parallel
		 {
		 #pragma omp for private(CCmax,_x0,Cc,iDisc,actrad,_y0)
		 for (i=0; i<Rmax*2; i+=1)
						 {
							actrad = ((float)i)/2;
							iDisc = MakeDisc(imgX,imgY,actrad);
							  //DigitalMicrograph::OpenAndSetProgressWindow("Progress", (boost::lexical_cast<std::string>(i) + ", ").c_str(), "");
							//maximum of cross correlation
							Cc = CrossCorrelate(iDisc,IMG);
							//Cc.DataChanged();
							CCmax = Max(Cc);
							PixelPos(Cc,CCmax,&_x0,&_y0,false);
							if (CCmax>BestCc)
							{
					
								Rr=actrad;
								BestCc=CCmax;
								*x0=imgX/2 + (imgX/2 - _x0); //as in script ln243, sending position of _x0 to input parameter
								*y0=imgY/2 + (imgY/2 - _y0); //
								 //DigitalMicrograph::DisplayAt(iDisc,100,200);
							}
							//result(i+": "+Cc+"\n")

						}
		 }
		 DigitalMicrograph::Result("Disc Found\n");

	 }
	 //DigitalMicrograph::Result("for loop finished\n");
	 //DigitalMicrograph::Result("Disc radius="+boost::lexical_cast<std::string>(Rr)+" pixels\n");
	 DigitalMicrograph::ImageDocumentClose(Cc.GetOrCreateImageDocument(),false);
	 
	 return(Rr);
}//End of DiscSize

float ExtraGatan::GaussSize(DigitalMicrograph::Image IMG, long *x0, long *y0,float* intensity) //finds a disc size, please use mode 2 if in doubt.
{
	 
	 long imgX,imgY;
	 DigitalMicrograph::Get2DSize(IMG, &imgX,&imgY);
	 DigitalMicrograph::Image Disc = DigitalMicrograph::ImageClone(IMG);
	 float max;
	 max = (float) DigitalMicrograph::GetPixel(IMG,imgX/2,imgY/2);
	 	 float SigMax;
	 //here
	 SigMax = ((float)(imgX))/sqrt((float)2);
	 Gatan::PlugIn::ImageDataLocker DiscLock(Disc);
	 float* discpix = (float*) DiscLock.get();
	 long _x0,_y0,discX,discY,MaxDiscX, MaxDiscY;
	 float Rr=0,i;
	 float BestCc=0;
	 float CCmax;
	 DigitalMicrograph::Image Cc, iDisc,Cctrans;
	 Cc = DigitalMicrograph::RealImage("Cross Correlation", 4,imgY,imgX);
	 DigitalMicrograph::DisplayAt(Cc,600,50);
	 DigitalMicrograph::SetWindowSize(Cc,200,200);
	 DigitalMicrograph::SetColorMode(Cc,4);
	 iDisc = DigitalMicrograph::ImageClone(Disc);
	 MaxDiscX = imgX/100;
	 MaxDiscY = imgY/100;
	PixelPos(IMG,max,&discX,&discY,false);

	  _x0 = imgX/2;
	  _y0 = imgY/2;


			for (i=0; i<SigMax; i+=5)
			{
					iDisc = GaussianNoise(imgX,imgY,i,max);
					//DigitalMicrograph::OpenAndSetProgressWindow("Progress", (boost::lexical_cast<std::string>(i) + ", ").c_str(), "");
					//maximum of cross correlation
					Cc = CrossCorr(Cc,iDisc,IMG);
					Cc.DataChanged();
					CCmax = Max(Cc);
					PixelPos(Cc,CCmax,&_x0,&_y0,false);
					//result(i+": "+Cc+"\n")
					if (CCmax>BestCc)
					{
					
						Rr=i;
						BestCc=CCmax;
						*x0=_x0;
						*y0=_y0;
								 //DigitalMicrograph::DisplayAt(iDisc,100,200);
					}
			}
	 //DigitalMicrograph::Result("for loop finished\n");
	 //DigitalMicrograph::Result("Disc radius="+boost::lexical_cast<std::string>(Rr)+" pixels\n");
	 DigitalMicrograph::ImageDocumentClose(Cc.GetOrCreateImageDocument(),false);
	 *intensity = max;
	 return(Rr);
}//End of DiscSize



std::vector<float> ExtraGatan::LocalDiscSize(DigitalMicrograph::Image IMG, std::vector<long> *x0, std::vector<long> *y0,int num) //finds a disc size, please use mode 2 if in doubt.
{
	 
	 long imgX,imgY;
	 DigitalMicrograph::Get2DSize(IMG, &imgX,&imgY);
	 DigitalMicrograph::Image Disc = DigitalMicrograph::ImageClone(IMG);
	 float max;
	 max = Max(IMG);
	 long Rmax;
	 //here
	 if(imgX>imgY)
	 {
		Rmax=(long) (round(imgY/6));
	 }
	 else
	 {
		Rmax=(long) (round(imgX/6));
	 }
	 Gatan::PlugIn::ImageDataLocker DiscLock(Disc);
	 float* discpix = (float*) DiscLock.get();
	 long _x0,_y0,discX,discY,MaxDiscX, MaxDiscY;
	 float Rr=0,i;
	 float BestCc=0;
	 float CCmax, average;
	 std::vector<float> radii,truradii, maxima, bestmax;
	 std::vector<long> tempX, tempY;
	 bestmax.resize(num);
	 maxima.resize(num);
	 radii.resize(num);
	 (*x0).resize(num);
	 (*y0).resize(num);
	 truradii.resize(num);
	 tempX.resize(num);
	 tempY.resize(num);
	 std::vector<long> tempX0, tempY0;
	 int m;
	 DigitalMicrograph::Image Cc, iDisc,Cctrans;
	 Cc = DigitalMicrograph::RealImage("Cross Correlation", 4,imgY,imgX);
	 DigitalMicrograph::DisplayAt(Cc,600,50);
	 DigitalMicrograph::SetWindowSize(Cc,200,200);
	 DigitalMicrograph::SetColorMode(Cc,4);
	 iDisc = DigitalMicrograph::ImageClone(Disc);
	 MaxDiscX = imgX/100;
	 MaxDiscY = imgY/100;

	PixelPos(IMG,max,&discX,&discY,false);
	PixelPos(IMG,max,&_x0, &_y0,false);
	average = AverageTemp(IMG);


					 for (i=0; i<Rmax; i+=0.5)
						 {
							iDisc = MakeDisc(imgX,imgY,i);
							DigitalMicrograph::OpenAndSetProgressWindow("Progress", (boost::lexical_cast<std::string>(i) + ", ").c_str(), "");
							//maximum of cross correlation
							Cc = CrossCorr(Cc,iDisc,IMG);
							Cc.DataChanged();
							CCmax = Max(Cc);
							maxima = FindLocalMaxima(Cc,num,false,&tempX0, &tempY0);
							//DigitalMicrograph::Result("hi");
							//result(i+": "+Cc+"\n")
							for(m=0;m<num;m++)
							{
								if (maxima[m]>bestmax[m])
								{
									//DigitalMicrograph::Result(boost::lexical_cast<std::string>(m));
									radii[m]=i;
									bestmax[m]=maxima[m];
									(tempX)[m]=tempX0[m];
									(tempY)[m]=tempY0[m];
									 //DigitalMicrograph::DisplayAt(iDisc,100,200);
								}
							}

						}

					for(m=0;m<num;m++)
					{
						if(radii[m] < Rmax-0.5 && DigitalMicrograph::GetPixel(IMG,tempX[m],tempY[m]) > average*2 ) 
						{
							truradii[m] = radii[m];
							(*x0)[m] = tempX[m];
							(*y0)[m] = tempY[m];
						}
						else
						{
							truradii[m] = 0;
							(*x0)[m] = 0;
							(*y0)[m] = 0;
						}


					}
	 //DigitalMicrograph::Result("for loop finished\n");
	 //DigitalMicrograph::Result("Disc radius="+boost::lexical_cast<std::string>(Rr)+" pixels\n");
	 DigitalMicrograph::ImageDocumentClose(Cc.GetOrCreateImageDocument(),false);


	 return(truradii);
}//End of DiscSize



DigitalMicrograph::Image  ExtraGatan::Acquire(int bin, bool* quit, bool* success, double expo)
{
	DigitalMicrograph::Image AcquiredImage;

       // Get current DM camera
       Gatan::Camera::Camera camera;
	   Gatan::uint32 xpixels; //Get from CCD anyway
		Gatan::uint32 ypixels; //Get from CCD anyway
       try

       {
              camera = Gatan::Camera::GetCurrentCamera();
              Gatan::Camera::CCD_GetSize(camera,&xpixels,&ypixels);
       }
       catch(...)
       {
              short error;
              long context;
              DigitalMicrograph::GetException(&error,&context);
              DigitalMicrograph::ErrorDialog(error);
              DigitalMicrograph::OpenAndSetProgressWindow("No Camera Detected","","");
			  *quit = true;
              return(AcquiredImage);
       }

       bool inserted = false;
       try
       {
              inserted = Gatan::Camera::GetCameraInserted(camera);
       }
       catch(...)

       {
              short error;
              long context;
              DigitalMicrograph::GetException(&error,&context);
              DigitalMicrograph::ErrorDialog(error);
              DigitalMicrograph::OpenAndSetProgressWindow("Couldn't check camera","status","");
			  *quit = true;
              return(AcquiredImage);
       }

       if (inserted != true)

       {
              DigitalMicrograph::OpenAndSetProgressWindow("Camera not inserted","","");
			  *quit = true;
              return(AcquiredImage);
       }

       // Want gain normalized imaging unless doing post processing yourself
       Gatan::Camera::AcquisitionProcessing processing = Gatan::Camera::kGainNormalized; 
       Gatan::Camera::AcquisitionParameters acqparams;

       try
       {
              acqparams = Gatan::Camera::CreateAcquisitionParameters(camera,processing,expo,bin,bin,0,0,ypixels,xpixels);
              Gatan::CM::SetDoContinuousReadout(acqparams,true);
              Gatan::CM::SetQualityLevel(acqparams,0); // Can't remember if fast or slow :D
              Gatan::Camera::Validate_AcquisitionParameters(camera,acqparams);
       }

       catch(...)
       {
              short error;
              long context;
              DigitalMicrograph::GetException(&error,&context);
              DigitalMicrograph::ErrorDialog(error);
              DigitalMicrograph::OpenAndSetProgressWindow("Problem with acquisition","parameters","");
			  *quit = true;
              return(AcquiredImage);
       }

       // NEW BIT FOR ALTERNATE ACQUISITION

       Gatan::CM::AcquisitionPtr acq = CreateAcquisition( camera, acqparams );

       // Turn into script object for my dms function

       DigitalMicrograph::ScriptObject acqtok = DigitalMicrograph::ScriptObjectProxy<Gatan::Camera::AcquisitionImp,DigitalMicrograph::DMObject>::to_object_token(acq.get());

       Gatan::CM::FrameSetInfoPtr fsi = ExtraGatan::GetFrameSetInfoPtr(acqtok);

       Gatan::Camera::AcquisitionImageSourcePtr acqsource = Gatan::Camera::AcquisitionImageSource::New(acq,fsi,0);

       // Start Acquisition

       acqsource->BeginAcquisition();
	   bool acqprmchanged = false;
		AcquiredImage = Gatan::Camera::CreateImageForAcquire(acq,"Acquired");

              if(!acqsource->AcquireTo(AcquiredImage,true,0.5f,acqprmchanged))

    {      // Now wait for it to finish again but dont restart if it finishes durign call....
		             while(!acqsource->AcquireTo(AcquiredImage,false,0.5f,acqprmchanged))

                     {
						// Waiting for read to finish
                     }
	}
       // Probably important...
	*success = true;

	acqsource->FinishAcquisition();
	return(AcquiredImage);
}

DigitalMicrograph::Image  ExtraGatan::Acquire(DigitalMicrograph::Image AcquiredImage, int bin, bool* quit, bool* success, double expo)
{
	

       // Get current DM camera
       Gatan::Camera::Camera camera;
	   DigitalMicrograph::Image JunkImg;
	   Gatan::uint32 xpixels; //Get from CCD anyway
		Gatan::uint32 ypixels; //Get from CCD anyway
       try

       {
              camera = Gatan::Camera::GetCurrentCamera();
              Gatan::Camera::CCD_GetSize(camera,&xpixels,&ypixels);
       }
       catch(...)
       {
              short error;
              long context;
              DigitalMicrograph::GetException(&error,&context);
              DigitalMicrograph::ErrorDialog(error);
              DigitalMicrograph::OpenAndSetProgressWindow("No Camera Detected","","");
			  *quit = true;
              return(AcquiredImage);
       }

       bool inserted = false;
       try
       {
              inserted = Gatan::Camera::GetCameraInserted(camera);
       }
       catch(...)

       {
              short error;
              long context;
              DigitalMicrograph::GetException(&error,&context);
              DigitalMicrograph::ErrorDialog(error);
              DigitalMicrograph::OpenAndSetProgressWindow("Couldn't check camera","status","");
			  *quit = true;
              return(AcquiredImage);
       }

       if (inserted != true)

       {
              DigitalMicrograph::OpenAndSetProgressWindow("Camera not inserted","","");
			  *quit = true;
              return(AcquiredImage);
       }

       // Want gain normalized imaging unless doing post processing yourself
       Gatan::Camera::AcquisitionProcessing processing = Gatan::Camera::kGainNormalized; 
       Gatan::Camera::AcquisitionParameters acqparams;

       try
       {
              acqparams = Gatan::Camera::CreateAcquisitionParameters(camera,processing,expo,bin,bin,0,0,ypixels,xpixels);
              Gatan::CM::SetDoContinuousReadout(acqparams,true);
              Gatan::CM::SetQualityLevel(acqparams,0); // Can't remember if fast or slow :D
              Gatan::Camera::Validate_AcquisitionParameters(camera,acqparams);
       }

       catch(...)
       {
              short error;
              long context;
              DigitalMicrograph::GetException(&error,&context);
              DigitalMicrograph::ErrorDialog(error);
              DigitalMicrograph::OpenAndSetProgressWindow("Problem with acquisition","parameters","");
			  *quit = true;
              return(AcquiredImage);
       }

       // NEW BIT FOR ALTERNATE ACQUISITION

       Gatan::CM::AcquisitionPtr acq = CreateAcquisition( camera, acqparams );

       // Turn into script object for my dms function

       DigitalMicrograph::ScriptObject acqtok = DigitalMicrograph::ScriptObjectProxy<Gatan::Camera::AcquisitionImp,DigitalMicrograph::DMObject>::to_object_token(acq.get());

       Gatan::CM::FrameSetInfoPtr fsi = ExtraGatan::GetFrameSetInfoPtr(acqtok);

       Gatan::Camera::AcquisitionImageSourcePtr acqsource = Gatan::Camera::AcquisitionImageSource::New(acq,fsi,0);

       // Start Acquisition

       acqsource->BeginAcquisition();
	   bool acqprmchanged = false;
		JunkImg = Gatan::Camera::CreateImageForAcquire(acq,"Acquired");

              if(!acqsource->AcquireTo(JunkImg,true,0.5f,acqprmchanged))

    {      // Now wait for it to finish again but dont restart if it finishes durign call....
		             while(!acqsource->AcquireTo(JunkImg,false,0.5f,acqprmchanged))

                     {
						// Waiting for read to finish
                     }
	}
       // Probably important...
	*success = true;
	Gatan::PlugIn::ImageDataLocker JunkLock(JunkImg);
	float* JunkPix = (float*) JunkLock.get();
	Gatan::PlugIn::ImageDataLocker AquLock(AcquiredImage);
	float* AquPix = (float*) AquLock.get();
	int i;
	long jX, jY;
	DigitalMicrograph::Get2DSize(JunkImg,&jX,&jY);
	for(i=0; i<jX*jY; i++)
	{
		AquPix[i] = JunkPix[i];
	}
	AquLock.~ImageDataLocker();
	JunkLock.~ImageDataLocker();
	acqsource->FinishAcquisition();
	AcquiredImage.DataChanged();
	return(AcquiredImage);
}

bool ExtraGatan::CheckCamera()
{
Gatan::Camera::Camera camera;
	   Gatan::uint32 xpixels; //Get from CCD anyway
		Gatan::uint32 ypixels; //Get from CCD anyway
       try

       {
              camera = Gatan::Camera::GetCurrentCamera();
              Gatan::Camera::CCD_GetSize(camera,&xpixels,&ypixels);
       }
       catch(...)
       {
              short error;
              long context;
              DigitalMicrograph::GetException(&error,&context);
              DigitalMicrograph::ErrorDialog(error);
              DigitalMicrograph::OpenAndSetProgressWindow("No Camera Detected","","");
              return(false);
       }

       bool inserted = false;
       try
       {
              inserted = Gatan::Camera::GetCameraInserted(camera);
       }
       catch(...)

       {
              short error;
              long context;
              DigitalMicrograph::GetException(&error,&context);
              DigitalMicrograph::ErrorDialog(error);
              DigitalMicrograph::OpenAndSetProgressWindow("Couldn't check camera","status","");
              return(false);
       }

       if (inserted != true)

       {
              DigitalMicrograph::OpenAndSetProgressWindow("Camera not inserted","","");
              return(false);
       }

	return(true);
}

//long ExtraGatan::EMGetCameraLength()
double ExtraGatan::EMGetCameraLength()
{
		   	   //long CamL;
			   double CamL;
               static DigitalMicrograph::Function __sFunction = (DM_FunctionToken) NULL;
               static const char *__sSignature = "long EMGetCameraLength()";
               Gatan::PlugIn::DM_Variant params[1];
               GatanPlugIn::gDigitalMicrographInterface.CallFunction( __sFunction.get_ptr(), 1, params, __sSignature );
			   
               CamL = params[0].v_uint32; //?
			   return(CamL);

}
long ExtraGatan::GetDataLockRange(long imgX, long x, long y)
{
	long number = x + y*imgX;
	return(number);
}


long ExtraGatan::GetDataLockRange(long imgX, long t, long b, long r, long l, long* start, long* finish)
{
	*start = t*imgX + l;
	*finish = b*imgX + r;
	return((long) (finish-start));
}

long ExtraGatan::GetDataLockRange(long t, long b, long r, long l, long imgX, long imgY, long stackNo, long* start, long* finish)
{
	*start = stackNo*imgX*imgY*t*l;
	*finish = stackNo*imgX*imgY*b*r+1;
	return((long) (finish-start));
}

void ExtraGatan::GetPixelXandY(long element, long* pix_X, long*pix_Y, long imgX)
{
	*pix_Y = (long) floor(((float) element)/ ((float) imgX));
	*pix_X = element - (*pix_Y*imgX);
}

float ExtraGatan::Tiltsize(float dTilt, float *T1X, float *T1Y, float *T2X, float *T2Y, int binning, DigitalMicrograph::Image *img1, DigitalMicrograph::Image *imgCC, DigitalMicrograph::Image *img0, float expo, float sleeptime, long _b)
{
	//magnitude of dTilt
	 DigitalMicrograph::Result("Tiltsize function started\n");
	 long TiltX0,TiltY0;
	 bool quit, success;
	 quit = false; success = false;
	 EMGetBeamTilt(&TiltX0,&TiltY0);


	 DigitalMicrograph::Sleep(1);
	 *img1 = Acquire(*img1, binning, &quit, &success, expo); // ln139? 
	 (*img1).DataChanged();
	  *imgCC = CrossCorr(*imgCC,*img1,*img0); // ln142
	 (*imgCC).DataChanged();

	 DigitalMicrograph::UpdateImage(*imgCC); //ln143
	 long x0,y0;//coords of untilted beam

	 float maxval;

	 maxval = Max(*imgCC);

	 PixelPos(*imgCC,maxval,&x0,&y0,false); //finding pixel positioni of max intensity

	 long tX=TiltX0+ (long)dTilt; //ln 146, added cast of long

	 ExtraGatan::ABSTilt(tX,TiltY0); // changing tilt to [tX,TiltY0] (EMSetBeamTilt) ln147

	 DigitalMicrograph::Sleep(sleeptime);//have to give the microscope time to respond


	 *img1 =  Acquire(*img1, binning, &quit, &success, expo); //ln149 getting tilted image
	(*img1).DataChanged();

	  *imgCC = CrossCorr(*imgCC,*img0,*img1);
	   (*imgCC).DataChanged();

	 DigitalMicrograph::UpdateImage(*imgCC);

	 long x,y;//coords of tilted beam, ln 152

	 maxval = Max(*imgCC);

	 PixelPos(*imgCC,maxval,&x,&y,false); // getting pixel position of max intensity pixel

	 float tPix=sqrt((float)((x-x0)*(x-x0)+(y-y0)*(y-y0)));//magnitude of the difference, incorrect y value <---------

	 dTilt= (float) (dTilt*0.25*_b/tPix); // from earlier _b=Camb/binning

	 tX=TiltX0+(long)dTilt; // added cast to long


	 //now measure tilt(pixels) per DAC
	 //finding change in position(pixels) for x-direction
	 ExtraGatan::ABSTilt(TiltX0+dTilt,TiltY0); //setting tilt to 0 poisition + dtilt in x

	 DigitalMicrograph::Sleep(sleeptime);
	 *img1 =  Acquire(*img1, binning, &quit, &success, expo);//acquiring tilted image
	(*img1).DataChanged();
	 *imgCC = CrossCorr(*imgCC,*img0,*img1);
	  (*imgCC).DataChanged();
	 DigitalMicrograph::UpdateImage(*imgCC);

	 maxval = Max(*imgCC); // finding max pixel intensity
	 PixelPos(*imgCC,maxval,&x,&y,false);// finding position of max

	 *T1X=(x-x0)/dTilt; //setting conversion factors
	 *T1Y=(y-y0)/dTilt;

	 //finding change in position(pixcels) for y-direction
	 ExtraGatan::ABSTilt(TiltX0,TiltY0+dTilt);
	 DigitalMicrograph::Sleep(sleeptime);

	 *img1 =  Acquire(*img1, binning, &quit, &success, expo);
	(*img1).DataChanged();
	 *imgCC = CrossCorr(*imgCC,*img0,*img1);
	 DigitalMicrograph::UpdateImage(*imgCC);
	 (*imgCC).DataChanged();
	 maxval = Max(*imgCC);
	 PixelPos(*imgCC,maxval,&x,&y,false);
	 *T2X=(x-x0)/dTilt;
	 *T2Y=(y-y0)/dTilt;
	 // setting to 0 position (resetting beam position)
	 ExtraGatan::ABSTilt(TiltX0,TiltY0);
	 DigitalMicrograph::Sleep(sleeptime);
	 *img1 =  Acquire(*img1, binning, &quit, &success, expo);
	(*img1).DataChanged();
	 DigitalMicrograph::Result(boost::lexical_cast<std::string>(dTilt) + "\n");

	 DigitalMicrograph::Result("Tiltsize function complete\n");
	 return (dTilt);
}

float ExtraGatan::Shiftsize(float dShift, float *Sh1X, float *Sh1Y,float *Sh2X,float *Sh2Y, int binning, float expo, float sleeptime, long _b)
{
	DigitalMicrograph::Result("Shiftsize function started\n");
	//magnitude of dShift
	long ShiftX0,ShiftY0; 
	EMGetBeamShift(&ShiftX0,&ShiftY0);
	bool quit, success;
	quit = false; success = true;
	DigitalMicrograph::Image s_img1 = Acquire(binning,&quit,&success,expo); // zero shift image
	s_img1.GetOrCreateImageDocument().ShowAtPosition(15,245);
	DigitalMicrograph::SetWindowSize(s_img1,200,200);
	DigitalMicrograph::Image s_img0 = DigitalMicrograph::ImageClone(s_img1);//put into img0
	s_img0.GetOrCreateImageDocument().ShowAtPosition(230,245);
	DigitalMicrograph::SetWindowSize(s_img0,200,200);
	DigitalMicrograph::Image s_imgCC=DigitalMicrograph::CrossCorrelate(s_img1,s_img0);
	s_imgCC.GetOrCreateImageDocument().ShowAtPosition(445,245);
	DigitalMicrograph::SetWindowSize(s_imgCC,200,200);
	long x0,y0;//coords of unshifted beam
	 

	float maxval/*, minval*/; //min not used??
	maxval = Max(s_imgCC);
	PixelPos(s_imgCC,maxval,&x0,&y0, false); //finding pixel position of max intensity of unshifted s_cc image
	long sX = (long) (ShiftX0 + dShift); //used to shift beam x direction
	long sY = ShiftY0; 

	//ABSShift(ShiftX0 + dShift,ShiftY0);//shifting the beam
	ABSShift(sX,ShiftY0);
	 DigitalMicrograph::Sleep(sleeptime);//have to give the microscope time to respond
	 s_img1 = Acquire(s_img1,binning,&quit,&success,expo);//getting shifted image
	 (s_img1).DataChanged();
	 s_imgCC=CrossCorr(s_imgCC,s_img1,s_img0);
	 (s_imgCC).DataChanged();
	 DigitalMicrograph::UpdateImage(s_imgCC);
	 long x,y;//coords of shifted beam
	 maxval = Max(s_imgCC);
	 PixelPos(s_imgCC,maxval,&x,&y, false);
	 float tPix=sqrt((float)((x-x0)*(x-x0)+(y-y0)*(y-y0)));//magnitude of the difference
	 ABSShift(ShiftX0,ShiftY0);//setting shift back to 0 position
	 DigitalMicrograph::Sleep(sleeptime);
	 dShift=(float) (round(dShift*0.25*((float)_b)/tPix)); // added cast to float in parameters
	 sX = ShiftX0 + (long)dShift;// added cast to long, updating to new dshift value
	 sY = ShiftY0;
	 //now measure shift(pixels) per DAC
	 ABSShift(sX,sY);// shifting beam in x direction using new dshift value
	 DigitalMicrograph::Sleep(sleeptime);//have to give the microscope time to respond
	 s_img1 = Acquire(s_img1, binning,&quit,&success,expo); //shifted image
	 (s_img1).DataChanged();
	 s_imgCC=CrossCorr(s_imgCC,s_img0,s_img1);
	 DigitalMicrograph::UpdateImage(s_imgCC);
	 s_imgCC.DataChanged();
	 maxval = Max(s_imgCC);
	 PixelPos(s_imgCC,maxval,&x,&y, false);
	 *Sh1X=((float)(x-x0))/dShift;//added cast to float
	 *Sh1Y=((float)(y-y0))/dShift;//aded cast to float
	 ABSShift(ShiftX0,ShiftY0);//resetting beam shift
	 DigitalMicrograph::Sleep(sleeptime);
	 sX = ShiftX0;
	 sY = ShiftY0+ (long)dShift;//added cast to long
	 ABSShift(sX,sY);// shifting beam in y direction using new dshift
	 DigitalMicrograph::Sleep(sleeptime);//have to give the microscope time to respond
	 s_img1 = Acquire(s_img1, binning,&quit,&success,expo);//shifted image
	 DigitalMicrograph::UpdateImage(s_img1);
	 s_imgCC=CrossCorr(s_imgCC,s_img0,s_img1);
	 DigitalMicrograph::UpdateImage(s_imgCC);
	 maxval = Max(s_imgCC);
	 PixelPos(s_imgCC,maxval,&x,&y, false);
	 *Sh2X=((float)(x-x0))/dShift;// added cast to float
	 *Sh2Y=((float)(y-y0))/dShift; // added cast to float
	 ABSShift(ShiftX0,ShiftY0); // resetting shift poisition
	 DigitalMicrograph::Sleep(sleeptime);//have to give the microscope time to respond
	 DigitalMicrograph::ImageDocumentClose(s_img0.GetOrCreateImageDocument(),false);//closing image windows when finished
	 DigitalMicrograph::ImageDocumentClose(s_img1.GetOrCreateImageDocument(),false);
	 DigitalMicrograph::ImageDocumentClose(s_imgCC.GetOrCreateImageDocument(),false);
	 DigitalMicrograph::Result("Shiftsize function complete\n");
	 return(dShift);
}

void ExtraGatan::intrinsic(long imgX, long imgY, DigitalMicrograph::Image * icol, DigitalMicrograph::Image * irow)
{
	Gatan::PlugIn::ImageDataLocker ColLock, RowLow;
}

void ExtraGatan::ResetBeam()
{
	float TiltX0, TiltY0, ShiftX0, ShiftY0;
	long ShiftX1, ShiftY1, TiltX1, TiltY1;
	long ShiftByX, ShiftByY, TiltByX, TiltByY;
	long ShiftXf, ShiftYf, TiltXf, TiltYf;
	DigitalMicrograph::TagGroup Persistent = DigitalMicrograph::GetPersistentTagGroup();
	DigitalMicrograph::TagGroupGetTagAsFloat(Persistent, "Info:TiltX0",&TiltX0);
	DigitalMicrograph::TagGroupGetTagAsFloat(Persistent, "Info:TiltY0",&TiltY0);
	DigitalMicrograph::TagGroupGetTagAsFloat(Persistent, "Info:ShiftX0",&ShiftX0);
	DigitalMicrograph::TagGroupGetTagAsFloat(Persistent, "Info:ShiftY0",&ShiftY0);
	EMGetBeamShift(&ShiftX1,&ShiftY1);
	EMGetBeamTilt(&TiltX1, &TiltY1);
	ShiftByX = (long) (ShiftX0 - ShiftX1);
	ShiftByY = (long) (ShiftY0 - ShiftY1);
	TiltByX = (long) (TiltX0 - TiltX1);
	TiltByY = (long) (TiltY0 - TiltY1);
	DigitalMicrograph::EMChangeTilt(TiltByX, TiltByY);
	DigitalMicrograph::EMBeamShift(ShiftByX, ShiftByY);
	EMGetBeamShift(&ShiftXf,&ShiftYf);
	EMGetBeamTilt(&TiltXf, &TiltYf);
	DigitalMicrograph::Result("Beam reset to original values\n");
	DigitalMicrograph::Result("TiltX = " + boost::lexical_cast<std::string>(TiltXf) + "\nTiltY = " + boost::lexical_cast<std::string>(TiltYf) + "\n");
	DigitalMicrograph::Result("ShiftX = " + boost::lexical_cast<std::string>(ShiftXf) + "\nShiftY = " + boost::lexical_cast<std::string>(ShiftYf) + "\n");
}

void ExtraGatan::ABSTilt(long tiltx, long tilty) //changes tilt from current position by tiltx and tilty
{
	long currentX, currentY;
	EMGetBeamTilt(&currentX, &currentY);
	DigitalMicrograph::EMChangeTilt(tiltx-currentX,tilty-currentY);
	EMGetBeamTilt(&currentX, &currentY);
	DigitalMicrograph::Result("TiltX = " + boost::lexical_cast<std::string>(currentX) + " , TiltY = " + boost::lexical_cast<std::string>(currentY) + "\n");
} 


void ExtraGatan::ABSShift(long shiftx, long shifty) //changes shift from current position by shiftx and shifty
{
	long currentX, currentY;
	EMGetBeamShift(&currentX, &currentY);
	DigitalMicrograph::EMBeamShift(shiftx-currentX,shifty-currentY);
	EMGetBeamShift(&currentX, &currentY);
	DigitalMicrograph::Result("ShiftX = " + boost::lexical_cast<std::string>(currentX) + " , ShiftY = " + boost::lexical_cast<std::string>(currentY) + "\n");
}

void ExtraGatan::CircularMask2D(DigitalMicrograph::Image *input,long pix_X,long pix_Y,float radius)
{
	long imgX, imgY;
	
	Gatan::PlugIn::ImageDataLocker inputlock(*input);
	float* inPix = (float*) inputlock.get();
	DigitalMicrograph::Get2DSize(*input, &imgX, &imgY);
	long centre = GetDataLockRange(imgX, pix_X, pix_Y);
	
	int i;
	long currentX, currentY;
	float xdist, ydist, dist;
	
	for(i=0; i<imgX*imgY; i++)
	{
		GetPixelXandY(i,&currentX,&currentY,imgX);
		xdist = (float) (abs(currentX-pix_X));
		ydist = (float) (abs(currentY-pix_Y));
		dist =(pow(xdist,2) + pow(ydist,2));
		
		if(dist>(pow(radius,2)+radius-1)) //
		{
			inPix[i] = 0;
		}
		
	}
}

void ExtraGatan::InverseCircularMask2D(DigitalMicrograph::Image *input,long pix_X,long pix_Y,float radius)
{
	long imgX, imgY;
	
	Gatan::PlugIn::ImageDataLocker inputlock(*input);
	float* inPix = (float*) inputlock.get();
	DigitalMicrograph::Get2DSize(*input, &imgX, &imgY);
	long centre = GetDataLockRange(imgX, pix_X, pix_Y);
	
	int i;
	long currentX, currentY;
	float xdist, ydist, dist;
	
	for(i=0; i<imgX*imgY; i++)
	{
		GetPixelXandY(i,&currentX,&currentY,imgX);
		xdist = (float) (abs(currentX-pix_X));
		ydist = (float) (abs(currentY-pix_Y));
		dist =(pow(xdist,2) + pow(ydist,2));
		
		if(dist<(pow(radius,2)+radius-1)) //
		{
			inPix[i] = 0;
		}
		
	}
}

void ExtraGatan::CircularMask3D(DigitalMicrograph::Image *input,long pix_X,long pix_Y, float radius)
{
	long imgX, imgY,imgZ;
	Gatan::PlugIn::ImageDataLocker inputlock(*input);
	float* inPix = (float*) inputlock.get();
	DigitalMicrograph::Get3DSize(*input, &imgX, &imgY,&imgZ);
	long centre = GetDataLockRange(imgX,pix_X, pix_Y);
	int i, zpt;
	long currentX, currentY;
	float xdist, ydist, dist;
	for(zpt=0; zpt<imgZ; zpt++)
	{
		for(i=0; i<imgX*imgY; i++)
		{
			GetPixelXandY((i-zpt*imgZ),&currentX,&currentY,imgX);
			xdist = (float) abs(currentX-pix_X);
			ydist = (float) abs(currentY-pix_Y);
			dist = (pow(xdist-1,2) + pow(ydist-1,2));
			if(dist>(pow(radius,2)+radius-1)) //
			{
				inPix[zpt*imgX*imgY*imgZ + i] = 0;
			}
		}
	}
}

DigitalMicrograph::Image ExtraGatan::ImageMaskTert(bool high, DigitalMicrograph::Image img1, DigitalMicrograph::Image img2)
{
	Gatan::PlugIn::ImageDataLocker img1Lock(img1), img2Lock(img2);
	float* img1Pix = (float*) img1Lock.get();
	float* img2Pix = (float*) img2Lock.get();
	int i;
	long img1X, img1Y, img2X, img2Y;
	DigitalMicrograph::Get2DSize(img1, &img1X, &img1Y);
	DigitalMicrograph::Get2DSize(img2, &img2X, &img2Y);
	if(img1X*img1Y != img2X*img2Y)
	{DigitalMicrograph::OkDialog("Images input to masking function are of different sizes"); return(img1);}
	DigitalMicrograph::Image imgMasked;
	for(i=0; i<img1X*img1Y; i++)
	{
		if(high==true)
		{
			if(img1Pix[i]>img2Pix[i]) {img1Pix[i] = img2Pix[i];}
		}
		if(high==false)
		{
			if(img1Pix[i]<img2Pix[i]) {img1Pix[i] = img2Pix[i];}
		}
	}
}

DigitalMicrograph::Image ExtraGatan::MakeDisc(long imgX, long imgY, float radius)
{
	DigitalMicrograph::Image fieldofones; //behold the many 1s
	fieldofones = DigitalMicrograph::RealImage("Disc Radius = " + boost::lexical_cast<std::string>(radius), 4,imgX,imgY);
	Gatan::PlugIn::ImageDataLocker oneLock(fieldofones); //oneLock is all we need
	float* onePix = (float*) oneLock.get();
	int i;
	for(i=0; i<imgX*imgY; i++)
	{
		onePix[i] = 1;
	}
	/*	float  max, min;
	DigitalMicrograph::ImageCalculateMinMax(fieldofones,0,0,&min,&max);
	onePix[imgY/2*imgX + imgX/2] = max*1.25;*/
	ExtraGatan::CircularMask2D(&fieldofones,ceil((float)(imgX/2)),ceil((float)(imgY/2)),radius);
	//DigitalMicrograph::SetLimits(fieldofones,0,1);
	return(fieldofones);
}

DigitalMicrograph::Image ExtraGatan::MakeDisc(long imgX, long imgY, float radius, long x0, long y0)
{
	DigitalMicrograph::Image fieldofones; //behold the many 1s
	fieldofones = DigitalMicrograph::RealImage("Disc Radius = " + boost::lexical_cast<std::string>(radius), 4,imgX,imgY);
	Gatan::PlugIn::ImageDataLocker oneLock(fieldofones); //oneLock is all we need
	float* onePix = (float*) oneLock.get();
	int i;
	for(i=0; i<imgX*imgY; i++)
	{
		onePix[i] = 1;
	}
	/*	float  max, min;
	DigitalMicrograph::ImageCalculateMinMax(fieldofones,0,0,&min,&max);
	onePix[y0*imgX + x0] = max*1.25;
	ExtraGatan::CircularMask2D(&fieldofones,x0,y0,radius);*/
	//DigitalMicrograph::SetLimits(fieldofones,0,1);
	return(fieldofones);
}


DigitalMicrograph::Image ExtraGatan::MakeDisc(long imgX, long imgY, float radius, long x0, long y0, float intensity)
{
	DigitalMicrograph::Image fieldofones; //behold the many 1s
	fieldofones = DigitalMicrograph::RealImage("Disc Radius = " + boost::lexical_cast<std::string>(radius), 4,imgX,imgY);
	Gatan::PlugIn::ImageDataLocker oneLock(fieldofones); //oneLock is all we need
	float* onePix = (float*) oneLock.get();
	int i;
	for(i=0; i<imgX*imgY; i++)
	{
		onePix[i] = intensity;
	}
	/*	float max, min;
	DigitalMicrograph::ImageCalculateMinMax(fieldofones,0,0,&min,&max);
	onePix[y0*imgX + x0] = max*1.25;
	ExtraGatan::CircularMask2D(&fieldofones,x0,y0,radius);*/
	//DigitalMicrograph::SetLimits(fieldofones,0,1);
	return(fieldofones);
}

DigitalMicrograph::Image ExtraGatan::MakeGradedDisc(long imgX, long imgY, float radius)
{
	DigitalMicrograph::Image fieldofones; //behold the many 1s
	fieldofones = DigitalMicrograph::RealImage("Disc Radius = " + boost::lexical_cast<std::string>(radius), 4,imgX,imgY);
	Gatan::PlugIn::ImageDataLocker oneLock(fieldofones); //oneLock is all we need
	float* onePix = (float*) oneLock.get();
	DigitalMicrograph::Image fieldlevel = DigitalMicrograph::RealImage("Disc Radius = " + boost::lexical_cast<std::string>(radius), 4,imgX,imgY);
	Gatan::PlugIn::ImageDataLocker levLock(fieldlevel); //oneLock is all we need
	float* levPix = (float*) levLock.get();
	int i,level;
	for(level=0; level<radius/2; level++)
	{
		for(i=0; i<imgX*imgY; i++)
		{
			onePix[i] = level/(radius);
		}
		ExtraGatan::CircularMask2D(&fieldofones,ceil((float)(imgX/2)),ceil((float)(imgY/2)),radius-level);
		for(i=0; i<imgX*imgY; i++)
		{
			levPix[i] += onePix[i];
		}
		
	}
	/*float max, min;
	DigitalMicrograph::ImageCalculateMinMax(fieldlevel,0,0,&min,&max);
	levPix[imgY/2*imgX + imgX/2] = max*1.25;*/
	//DigitalMicrograph::SetLimits(fieldofones,0,1);
	return(fieldlevel);

}

DigitalMicrograph::Image ExtraGatan::MakeGradedDisc(long imgX, long imgY, float radius, long x0, long y0)
{
	DigitalMicrograph::Image fieldofones; //behold the many 1s
	fieldofones = DigitalMicrograph::RealImage("Disc Radius = " + boost::lexical_cast<std::string>(radius), 4,imgX,imgY);
	Gatan::PlugIn::ImageDataLocker oneLock(fieldofones); //oneLock is all we need
	float* onePix = (float*) oneLock.get();
	DigitalMicrograph::Image fieldlevel = DigitalMicrograph::RealImage("Disc Radius = " + boost::lexical_cast<std::string>(radius), 4,imgX,imgY);
	Gatan::PlugIn::ImageDataLocker levLock(fieldlevel); //oneLock is all we need
	float* levPix = (float*) levLock.get();
	int i,level;
	for(level=0; level<radius/2; level++)
	{
		for(i=0; i<imgX*imgY; i++)
		{
			onePix[i] = level/(radius);
		}
		ExtraGatan::CircularMask2D(&fieldofones,x0,y0,radius-level);
		for(i=0; i<imgX*imgY; i++)
		{
			levPix[i] += onePix[i];
		}
		
	}
	/*float max, min;
	DigitalMicrograph::ImageCalculateMinMax(fieldlevel,0,0,&min,&max);
	levPix[y0*imgX + x0] = max*1.25;*/
	//DigitalMicrograph::SetLimits(fieldofones,0,1);
	return(fieldlevel);
}


DigitalMicrograph::Image ExtraGatan::MakeGradedDisc(long imgX, long imgY, float radius, long x0, long y0, float intensity)
{
		DigitalMicrograph::Image fieldofones; //behold the many 1s
	fieldofones = DigitalMicrograph::RealImage("Disc Radius = " + boost::lexical_cast<std::string>(radius), 4,imgX,imgY);
	Gatan::PlugIn::ImageDataLocker oneLock(fieldofones); //oneLock is all we need
	float* onePix = (float*) oneLock.get();
	DigitalMicrograph::Image fieldlevel = DigitalMicrograph::RealImage("Disc Radius = " + boost::lexical_cast<std::string>(radius), 4,imgX,imgY);
	Gatan::PlugIn::ImageDataLocker levLock(fieldlevel); //oneLock is all we need
	float* levPix = (float*) levLock.get();
	int i,level;
	for(level=0; level<radius/2; level++)
	{
		for(i=0; i<imgX*imgY; i++)
		{
			onePix[i] = level/(radius)*intensity;
		}
		ExtraGatan::CircularMask2D(&fieldofones,x0,y0,radius-level);
		for(i=0; i<imgX*imgY; i++)
		{
			levPix[i] += onePix[i];
		}
	}
	/*	float max, min;
	DigitalMicrograph::ImageCalculateMinMax(fieldlevel,0,0,&min,&max);
	levPix[y0*imgX + x0] = max*1.25;*/
	//DigitalMicrograph::SetLimits(fieldofones,0,1);
	return(fieldlevel);

}



DigitalMicrograph::Image ExtraGatan::Starfield(long imgX, long imgY)
{
	DigitalMicrograph::Image Starscape = DigitalMicrograph::RealImage("Starfield",4,imgX,imgY);
	DigitalMicrograph::Image Star;
	Gatan::PlugIn::ImageDataLocker Starlock(Starscape);
	float* StarPix = (float*) Starlock.get();
	float* sunPix;
	Gatan::PlugIn::ImageDataLocker sunlock;
	srand(time(NULL));
	int num = rand()%20 + 3;
	long xpos, ypos;
	float radius,intensity,throwaway;
	int i,j;
	for (int i = 0; i < 100; i++)
	{
		throwaway = rand();
	}
	for (int i = 0; i < num; i++)
	{
		 xpos = rand()%imgX;
		 ypos = rand()%imgY;
		 radius = rand()%30 + 5;
		 intensity = rand()%10000 + 1000;
		 Star = MakeDisc(imgX, imgY, radius, xpos, ypos, intensity);
		 //DigitalMicrograph::DisplayAt(Star,30,30);
		 sunlock.lock(Star);
		 sunPix = (float*) sunlock.get();
		 for(j=0; j<imgX*imgY; j++)
		 {
			 StarPix[j]+=sunPix[j];
		 }
	}
	return(Starscape);
}

DigitalMicrograph::Image ExtraGatan::GradedStarfield(long imgX, long imgY)
{
	DigitalMicrograph::Image Starscape = DigitalMicrograph::RealImage("Starfield",4,imgX,imgY);
	DigitalMicrograph::Image Star;
	Gatan::PlugIn::ImageDataLocker Starlock(Starscape);
	float* StarPix = (float*) Starlock.get();
	float* sunPix;
	Gatan::PlugIn::ImageDataLocker sunlock;
	srand(time(NULL));
	int num = rand()%20 + 3;
	long xpos, ypos;
	float radius,intensity,throwaway;
	int i,j;
	for (int i = 0; i < 100; i++)
	{
		throwaway = rand();
	}
	for (int i = 0; i < num; i++)
	{
		 xpos = rand()%imgX;
		 ypos = rand()%imgY;
		 radius = rand()%30 + 5;
		 intensity = rand()%10000 + 1000;
		 Star = MakeGradedDisc(imgX, imgY, radius, xpos, ypos, intensity);
		 //DigitalMicrograph::DisplayAt(Star,30,30);
		 sunlock.lock(Star);
		 sunPix = (float*) sunlock.get();
		 for(j=0; j<imgX*imgY; j++)
		 {
			 StarPix[j]+=sunPix[j];
		 }
	}
	return(Starscape);
}

DigitalMicrograph::Image ExtraGatan::NoisyStarfield(long imgX, long imgY)
{
	DigitalMicrograph::Image Output = Starfield(imgX, imgY);
	Gatan::PlugIn::ImageDataLocker OutLock(Output);

	float* OutPix = (float*) OutLock.get();
	DigitalMicrograph::Image Noise = RandMap(imgX, imgY);
	Gatan::PlugIn::ImageDataLocker NoiseLock(Noise);
	float* NoisePix = (float*) NoiseLock.get();
	int i;
	for(i=0; i<imgX*imgY; i++)
	{
		OutPix[i] += NoisePix[i];
	}
	return(Output);
}

DigitalMicrograph::Image ExtraGatan::GradedNoisyStarfield(long imgX, long imgY)
{
	DigitalMicrograph::Image Output = GradedStarfield(imgX, imgY);
	Gatan::PlugIn::ImageDataLocker OutLock(Output);

	float* OutPix = (float*) OutLock.get();
	DigitalMicrograph::Image Noise = RandMap(imgX, imgY);
	Gatan::PlugIn::ImageDataLocker NoiseLock(Noise);
	float* NoisePix = (float*) NoiseLock.get();
	int i;
	for(i=0; i<imgX*imgY; i++)
	{
		OutPix[i] += NoisePix[i];
	}
	return(Output);
}

DigitalMicrograph::Image ExtraGatan::GradedGaussStarfield(long imgX, long imgY,float intensity, float SD)
{
	DigitalMicrograph::Image Output = GradedStarfield(imgX, imgY);
	Gatan::PlugIn::ImageDataLocker OutLock(Output);

	float* OutPix = (float*) OutLock.get();
	DigitalMicrograph::Image Noise = GaussianNoise(imgX, imgY,SD,intensity);
	Gatan::PlugIn::ImageDataLocker NoiseLock(Noise);
	float* NoisePix = (float*) NoiseLock.get();
	int i;
	for(i=0; i<imgX*imgY; i++)
	{
		OutPix[i] += NoisePix[i];
	}
	return(Output);
}

DigitalMicrograph::Image ExtraGatan::MakeGrid(long imgX, long imgY)
{
	DigitalMicrograph::Image Gridscape = DigitalMicrograph::RealImage("Simulated Diffraction",4,imgX,imgY);
	DigitalMicrograph::Image Star;
	Gatan::PlugIn::ImageDataLocker Gridlock(Gridscape);
	float* GridPix = (float*) Gridlock.get();
	float* sunPix;
	Gatan::PlugIn::ImageDataLocker sunlock;
	srand(time(NULL));
	int spacing = rand()%50 + 50;
	float radius = rand()%30 + 15;
	float intensity = rand()%10000 + 1000;
	long numberx = floor((float)(imgX)/(spacing+2*radius));
	long numbery = floor((float)(imgY)/(spacing+2*radius));
	long xpos, ypos;
	float throwaway;
	int i,j,k;
	for (int i = 0; i < 100; i++)
	{
		throwaway = rand();
	}
	for (int i = -(numberx/2); i <= numberx/2; i++)
	{
		for(j = -(numbery/2); j <= numbery/2; j++)
		{
		 xpos = imgX/2 + i*(spacing+2*radius);
		 ypos = imgY/2 + j*(spacing+2*radius);
		
		 Star = MakeGradedDisc(imgX, imgY, radius, xpos, ypos, intensity);
		 //DigitalMicrograph::DisplayAt(Star,30,30);
		 sunlock.lock(Star);
		 sunPix = (float*) sunlock.get();
		 for(k=0; k<imgX*imgY; k++)
		 {
			 GridPix[k]+=sunPix[k];
		 }
		}
	}
	return(Gridscape);

}

std::vector<float> ExtraGatan::FindLocalMaxima(DigitalMicrograph::Image input, int sectors, bool avg, std::vector<long> *xpos,  std::vector<long> *ypos) //splits an image up into a number of segments, finds the maximum in each segment.
{
	float min, max;
	long xloc, yloc, inX, inY;
	int counter = 0;
	int sidesect, i,j,k,l,m, element, column, row, unit, segment, rowspersect, colspersect;
	std::vector<long> locations;
	std::vector<float> maxs;
	std::vector<float> mins;
	DigitalMicrograph::Get2DSize(input, &inX, &inY);
	//DigitalMicrograph::Result(boost::lexical_cast<std::string>(inX) + "\n" + boost::lexical_cast<std::string>(inY) + "\n");
	/*if(sectors!=2 && sectors!=4 && sectors!=9 && sectors!=16 && sectors!=25 && sectors!=36 && sectors!=49 && sectors!=64 && sectors != 81 
		&& sectors!=100)*/
	locations.resize(sectors*2);
	(*xpos).resize(sectors);
	(*ypos).resize(sectors);
	if((int) sqrt((float) sectors) != sqrt((float) sectors))
	{
		for(j=0; j<sectors*2;j++)
		{
			locations[j] = 0;
		}
		DigitalMicrograph::OkDialog("Critical Failure");
		DigitalMicrograph::Result("Program quit because number of sectors was not a square number!\n");
		return(maxs);
	}
	locations.resize(sectors*2);
	maxs.resize(sectors);
	mins.resize(sectors);
	Gatan::PlugIn::ImageDataLocker inlock(input);
	float* inPix = (float*) inlock.get();

	sidesect = (int) sqrt((float)sectors);
	std::vector<DigitalMicrograph::Image> section;
	std::vector<Gatan::PlugIn::ImageDataLocker> sectlock;
	std::vector<float*> sectPix;
	std::vector<int> counters;
	section.resize(sectors);
	sectlock.resize(sectors);
	sectPix.resize(sectors);
	counters.resize(sectors);
	for(i=0; i<sectors; i++)
	{
	section[i] = DigitalMicrograph::RealImage("section number = " + boost::lexical_cast<std::string>(i), 4, inX/sidesect, inY/sidesect);
	sectlock[i].lock(section[i]);
	sectPix[i] = (float*) sectlock[i].get();
	counters[i] = 0;
	}
		rowspersect = inY/sidesect;
		colspersect = inX/sidesect;
		int progress;
		std::string loading;
	for(i=0; i<inX*inY; i++)
	{
		row = floor(((float) i)/inX);

		column = i - row*inX;

		segment = floor((((float)column)/colspersect)) + sidesect*floor((((float)row)/rowspersect));

		sectPix[segment][counters[segment]] = inPix[i];
		//DigitalMicrograph::Result(boost::lexical_cast<std::string>(i) + "\n");
		counters[segment]++;
		//DigitalMicrograph::Result(boost::lexical_cast<std::string>(i) + "\n");
	}
	long locallocx, locallocy, unitx, unity, secrow, seccol;
	for(i=0; i<sectors; i++)
	{
		j = 2*i;
		//DigitalMicrograph::DisplayAt(section[i],i*20,200);
		maxs[i] = Max(section[i]);
		PixelPos(section[i],maxs[i],&locallocx,&locallocy, avg);
		secrow = floor(((float) i )/sidesect);
		seccol = i - sidesect*secrow;
		unitx = locallocx + seccol*colspersect;
		unity = locallocy + secrow*rowspersect;
		locations[j] = unitx;
		locations[j+1] = unity;
		(*xpos)[i] = unitx;
		(*ypos)[i] = unity;
		//DigitalMicrograph::Result(boost::lexical_cast<std::string>(maxs[i]) + " - " + "[" + boost::lexical_cast<std::string>(unitx) + "," + boost::lexical_cast<std::string>(unity)+"]" + "\n");
	}
	return(maxs);
}




DigitalMicrograph::Image ExtraGatan::DiffImage(DigitalMicrograph::Image input, bool smooth)
{
	//** It is vital to using this function that the way that data is stored is correctly understood **\\
	//			 diffmapx[i] corresponds to the boundary between input[i] and input[i+1]			   \\
	//			 diffmapy[i] corresponds to the boundary between input[i] and input[i+inX]             \\
	//			 As such, there is no entry for input[n*inX] in the diffmap, as there is no				\\
	//					neighbouring pixel to the right. Same for input[n*inY].							\\
	//			 The diffmap is simply the two differentials added together. This is useful for         \\
	//			 Finding peaks, checking for slopes etc, but lacks real mathematical meaning			\\
	//					TODO - Write a function which outputs diffx or diffy or both.					\\


	long inX, inY;
	DigitalMicrograph::Get2DSize(input, &inX, &inY);
	Gatan::PlugIn::ImageDataLocker inlock(input);
	float* inPix = (float*) inlock.get();
	std::vector<float> xdiff, ydiff;
	xdiff.resize((inX)*(inY));
	ydiff.resize((inX)*(inY));
	DigitalMicrograph::Image diffmapx = DigitalMicrograph::RealImage("x_Differential", 4, inX, inY);
	Gatan::PlugIn::ImageDataLocker diffxlock(diffmapx);
	float* diffxPix = (float*) diffxlock.get();
	DigitalMicrograph::Image diffmapy = DigitalMicrograph::RealImage("y_Differential", 4, inX, inY);
	Gatan::PlugIn::ImageDataLocker diffylock(diffmapy);
	float* diffyPix = (float*) diffylock.get();
	DigitalMicrograph::Image diffmap = DigitalMicrograph::RealImage("Net Differential", 4, inX, inY);
	Gatan::PlugIn::ImageDataLocker difflock(diffmap);
	float* diffPix = (float*) difflock.get();

	int i,row,column,xrow,xcol,yrow,ycol,unit;
	unit = 0;

	for(i=0; i<inX*inY; i++)
	{
		row = floor(((float) i)/inX);
		column = i - row*inX;
		if(column!=inX) {xdiff[i] = (inPix[i+1]-inPix[i])/2;}
		if(row!=inY) {ydiff[i] = (inPix[i+inX]-inPix[i])/2;}
		if(column==inX) {xdiff[i] = 0;}
		if(row==inY) {ydiff[i] = 0;}
	}

	for(i=0; i<(inX)*(inY);i++)
	{

		diffxPix[i] = xdiff[i];
	}

	for(i=0; i<(inX)*(inY);i++)
	{
		diffyPix[i] = ydiff[i];
	}

	for(i=0; i<(inX)*(inY);i++)
	{
		row = floor(((float) i)/inX);
		column = i - row*inX;
		diffPix[i] = diffyPix[i] + diffxPix[i];
	}

	return(diffmap);
}

std::vector<long> ExtraGatan::FindPeaks(DigitalMicrograph::Image input)
{
	long inX, inY,n, xpos, ypos, num;
	float max, runningmax,avgmax, maxmax, minmax,min;
	runningmax = 0;
	n=2500;
	num = 0;
	DigitalMicrograph::ImageCalculateMinMax(input,0,0,&min,&maxmax);
	minmax = maxmax;
	DigitalMicrograph::Get2DSize(input, &inX, &inY);
	std::vector<long> peakpositions;// = FindLocalMaxima(input,n,true);
	std::vector<long> actualpos;
	Gatan::PlugIn::ImageDataLocker inlock(input);
	float* inPix = (float*) inlock.get();
	long diffX, diffY;
	DigitalMicrograph::Image diff = DiffImage(input, false);
	DigitalMicrograph::Get2DSize(diff, &diffX, &diffY);
	Gatan::PlugIn::ImageDataLocker difflock(diff);
	float* diffPix = (float*) difflock.get();
	std::vector<bool> isitapeak(n);
	int i;
	float runningdiff,avgdiff;
	runningdiff = 0;
	for(i=0; i<diffX*diffY; i++)
	{
		runningdiff+=abs(diffPix[i]);
	}
	avgdiff = runningdiff/diffX*diffY;

	avgmax = runningmax/n;
	DigitalMicrograph::Result("avg = " + boost::lexical_cast<std::string>(avgmax) + "\n");
	for(i=0; i<n; i++)
	{
		xpos = peakpositions[2*i];
		ypos = peakpositions[2*i+1];
		max = inPix[ypos*inX + xpos];
		if(max<avgmax) {isitapeak[i] = false;}
	}






	for(i=0;i<n;i++)
	{
		if(isitapeak[i] == true) {num++;}
	}

	DigitalMicrograph::Result("hi\n");
	DigitalMicrograph::Result("The program found " + boost::lexical_cast<std::string>(num) + " peaks\n");
	actualpos.resize(num*2);
	
	int unit = 0;
	for(i=0;i<n;i++)
	{
		if(isitapeak[i] == true)
		{
			xpos = peakpositions[2*i];
			ypos = peakpositions[2*i+1];
			max = inPix[ypos*inX + xpos];
			actualpos[unit] = peakpositions[2*i];
			actualpos[unit+1] = peakpositions[2*i+1];
			DigitalMicrograph::Result(boost::lexical_cast<std::string>(max) + " - " + "[" + boost::lexical_cast<std::string>(xpos) + "," + boost::lexical_cast<std::string>(ypos)+"]" + "\n");
			unit+=2;
			
		}	
	}
	return(actualpos);
} 

void ExtraGatan::EMGetStageXY (double &StageX, double &StageY)
{
       static DigitalMicrograph::Function __sFunction = (DM_FunctionToken) NULL;
       static const char *__sSignature = "void EMGetStageXY( double *, double* )";
       Gatan::PlugIn::DM_Variant params[2];
       params[0].v_float64_ref =  &StageX;
       params[1].v_float64_ref =  &StageY;
       GatanPlugIn::gDigitalMicrographInterface.CallFunction( __sFunction.get_ptr(), 2, params, __sSignature );
}

void ExtraGatan::EMSetStageXY (double StageX, double StageY)
{
       static DigitalMicrograph::Function __sFunction = (DM_FunctionToken) NULL;
       static const char *__sSignature = "void EMSetStageXY( double , double )";
       Gatan::PlugIn::DM_Variant params[2];
       params[0].v_float64 =  StageX;
       params[1].v_float64 =  StageY;
       GatanPlugIn::gDigitalMicrographInterface.CallFunction( __sFunction.get_ptr(), 2, params, __sSignature );
}

void ExtraGatan::EMSetStageX (double StageX)
{
       static DigitalMicrograph::Function __sFunction = (DM_FunctionToken) NULL;
       static const char *__sSignature = "void EMSetStageX( double )";
       Gatan::PlugIn::DM_Variant params[2];
       params[0].v_float64 =  StageX;
       GatanPlugIn::gDigitalMicrographInterface.CallFunction( __sFunction.get_ptr(), 1, params, __sSignature );
}

void ExtraGatan::EMSetStageY (double StageY)

{
       static DigitalMicrograph::Function __sFunction = (DM_FunctionToken) NULL;
       static const char *__sSignature = "void EMSetStageY( double )";
       Gatan::PlugIn::DM_Variant params[2];
       params[0].v_float64 =  StageY;
       GatanPlugIn::gDigitalMicrographInterface.CallFunction( __sFunction.get_ptr(), 1, params, __sSignature );
}

double ExtraGatan::EMGetStageX ()

{
       static DigitalMicrograph::Function __sFunction = (DM_FunctionToken) NULL;
       static const char *__sSignature = "double EMGetStageX( )";
       Gatan::PlugIn::DM_Variant params[1];
       GatanPlugIn::gDigitalMicrographInterface.CallFunction( __sFunction.get_ptr(), 1, params, __sSignature );
       return params[0].v_float64;
}

double ExtraGatan::EMGetStageY ()

{
       static DigitalMicrograph::Function __sFunction = (DM_FunctionToken) NULL;
       static const char *__sSignature = "double EMGetStageY( )";
       Gatan::PlugIn::DM_Variant params[1];
       GatanPlugIn::gDigitalMicrographInterface.CallFunction( __sFunction.get_ptr(), 1, params, __sSignature );
       return(params[0].v_float64);
}

void ExtraGatan::EMSetHighTensionOffset( double offset )

{
    static Gatan::DM::Function __sFunction = (DM_FunctionToken) NULL;
    static const char *__sSignature = "void EMSetHighTensionOffset( double )";
    Gatan::PlugIn::DM_Variant params[1];
    params[0].v_float64 = offset;
    GatanPlugIn::gDigitalMicrographInterface.CallFunction( __sFunction.get_ptr(), 1, params, __sSignature );
}

 

double ExtraGatan::EMGetHighTensionOffset( )

{
    static Gatan::DM::Function __sFunction = (DM_FunctionToken) NULL;
    static const char *__sSignature = "double EMGetHighTensionOffset(  )";
    Gatan::PlugIn::DM_Variant params[1];
    GatanPlugIn::gDigitalMicrographInterface.CallFunction( __sFunction.get_ptr(), 1, params, __sSignature );
    return params[0].v_float64;
}

void ExtraGatan::EMSetBrightness( double brightness )

{
    static Gatan::DM::Function __sFunction = (DM_FunctionToken) NULL;
    static const char *__sSignature = "void EMSetBrightness( double )";
    Gatan::PlugIn::DM_Variant params[1];
    params[0].v_float64 = brightness;
    GatanPlugIn::gDigitalMicrographInterface.CallFunction( __sFunction.get_ptr(), 1, params, __sSignature );
}

double ExtraGatan::EMGetBrightness( )

{
    static Gatan::DM::Function __sFunction = (DM_FunctionToken) NULL;
    static const char *__sSignature = "double EMGetBrightness(  )";
    Gatan::PlugIn::DM_Variant params[1];
    GatanPlugIn::gDigitalMicrographInterface.CallFunction( __sFunction.get_ptr(), 1, params, __sSignature );
    return params[0].v_float64;
}

float ExtraGatan::Interpolate(DigitalMicrograph::Image* input,long xCal,long yCal,long pX,long nX,long pY,long nY, long nnX, long nnY)
{
	long Cal_X, Cal_Y;
	/*DigitalMicrograph::Result("xCal: " + boost::lexical_cast<std::string>(xCal) + "\n");
	DigitalMicrograph::Result("yCal: " + boost::lexical_cast<std::string>(yCal) + "\n");
	DigitalMicrograph::Result("pX: " + boost::lexical_cast<std::string>(pX) + "\n");
	DigitalMicrograph::Result("nX: " + boost::lexical_cast<std::string>(nX) + "\n");
	DigitalMicrograph::Result("pY: " + boost::lexical_cast<std::string>(pY) + "\n");
	DigitalMicrograph::Result("nY: " + boost::lexical_cast<std::string>(nY) + "\n");
	DigitalMicrograph::Result("nnX: " + boost::lexical_cast<std::string>(nnX) + "\n");
	DigitalMicrograph::Result("nnY: " + boost::lexical_cast<std::string>(nnY) + "\n");*/

	DigitalMicrograph::Get2DSize(*input,&Cal_X,&Cal_Y);
	Gatan::PlugIn::ImageDataLocker InLock(*input);
	float* InPix = (float*) InLock.get();
	float tl = InPix[GetDataLockRange(Cal_X, nnX,nnY)];
	float tr =InPix[GetDataLockRange(Cal_X, nnX+1,nnY)];
	float bl =InPix[GetDataLockRange(Cal_X, nnX,nnY+1)];
	float br=InPix[GetDataLockRange(Cal_X, nnX+1,nnY+1)];
	float Cor;
	float a=tl;
	float b=(tr-tl)/xCal;
	float c=(bl-tl)/yCal;
	float d=(tl-bl-tr+br)/(xCal*yCal);
	//result("ax,bx,cx,dx "+_ax+","+_bx+","+_cx+","+_dx+"\n");
	Cor=a+b*(pX-nX*xCal)+c*(pY-nY*yCal)+d*(pX-nX*xCal)*(pY-nY*yCal);
	/*DigitalMicrograph::Result("a: " + boost::lexical_cast<std::string>(a) + "\n");
	DigitalMicrograph::Result("b: " + boost::lexical_cast<std::string>(b) + "\n");
	DigitalMicrograph::Result("c: " + boost::lexical_cast<std::string>(c) + "\n");
	DigitalMicrograph::Result("d: " + boost::lexical_cast<std::string>(d) + "\n");
	DigitalMicrograph::Result("Cor: " + boost::lexical_cast<std::string>(Cor) + "\n");*/
	return(Cor);
}

DigitalMicrograph::Image ExtraGatan::DrawPlot(DigitalMicrograph::Image *input, long x0, long y0, long x1, long y1)
{
	long length = (long) round(sqrt((float) ((x1-x0)*(x1-x0) + (y1-y0)*(y1-y0))));
	/*if((x1-x0)>(y1-y0)) {length = x1-x0;}
	if((x1-x0)<(y1-y0)) {length = y1-y0;}*/
	float tangent = (float) (y1-y0)/(x1-x0);
	DigitalMicrograph::Result("tangent = " + boost::lexical_cast<std::string>(tangent) + "\n");
	float angle = atan(tangent);
	DigitalMicrograph::Result("angle = " + boost::lexical_cast<std::string>(angle) + "\n");
	DigitalMicrograph::Image plot = DigitalMicrograph::RealImage("Plot", 4, length);
	Gatan::PlugIn::ImageDataLocker InLock(*input);
	float* InPix = (float*) InLock.get();
	Gatan::PlugIn::ImageDataLocker PlotLock(plot);
	float* plotPix = (float*) PlotLock.get();
	long inX, inY;
	DigitalMicrograph::Get2DSize(*input,&inX, &inY);
	int i, j;
	long xpos, ypos, element,segment, prev_x, prev_y;
	float ideal_x, ideal_y;
	//plotPix[0] = InPix[y0*inX + x0];
	//plotPix[1] = InPix[
	prev_x = 0;
	prev_y = 0;
	for(segment=0; segment<length; segment++)
	{
			ideal_y = segment*sqrt((float)2)*sin(angle);
			//DigitalMicrograph::Result("ideal_y = " + boost::lexical_cast<std::string>(ideal_y) + "\n");
			ideal_x = segment*sqrt((float)2)*cos(angle);
			//DigitalMicrograph::Result("ideal_x = " + boost::lexical_cast<std::string>(ideal_x) + "\n");
			xpos = round(ideal_x);
			ypos = round(ideal_y);
			if(xpos > prev_x+1)
			{
				xpos = floor(ideal_x);
			}
			if(xpos > prev_x+1)
			{
				xpos --;
			}
			if(ypos > prev_y+1)
			{
				ypos = floor(ideal_y);
			}
			if(ypos > prev_y+1)
			{
				ypos --;
			}
			if(xpos == prev_x && ypos == prev_y)
			{
				if(ideal_x-xpos<ideal_y-ypos)
				{
					xpos++;
				}
				else if(ideal_x-xpos>ideal_y-ypos)
				{
					ypos++;
				}
			}

			//DigitalMicrograph::Result("[" + boost::lexical_cast<std::string>(xpos) + "," + boost::lexical_cast<std::string>(ypos) + "]" + "\n");
			plotPix[segment] = InPix[y0*inX + x0 + ypos*inX + xpos];
			prev_x = xpos;
			prev_y = ypos;
	}

	DigitalMicrograph::ImageDisplay disp;

	PlotLock.~ImageDataLocker();
	InLock.~ImageDataLocker();

	disp = (*input).GetImageDisplay(0);
	DigitalMicrograph::Component line = DigitalMicrograph::NewLineAnnotation(y0,x0,y1,x1);
	disp.AddChildAtEnd(line);

	//(*input).DataChanged();
	//DigitalMicrograph::ImageDisplay plotdisp = plot.GetOrCreateImageDocument.GetImageDisplay();
	return(plot);

}

void ExtraGatan::ClearGaussianNoise(DigitalMicrograph::Image *IMG)
{
	long ixpos,iypos, imgX, imgY;
	float sig,max;
	DigitalMicrograph::Get2DSize(*IMG,&imgX,&imgY);
	sig = GaussSize(*IMG,&ixpos,&iypos,&max);
	DigitalMicrograph::Result("Hi\n");
	(*IMG)-=GaussianNoise(imgX,imgY,sig,max);
	DigitalMicrograph::Result("Hi\n");
}

long ExtraGatan::MultiPeak(DigitalMicrograph::Image IMG, std::vector<long> *xpos, std::vector<long> *ypos,  std::vector<float> *radii, float minrad, long maxrun)
{
	float iradius, noiselevel, averagerad, runningrad;
	averagerad = 0;
	runningrad = 0;
	long ixpos,iypos, imgX, imgY;
	DigitalMicrograph::Get2DSize(IMG,&imgX,&imgY);
	iradius = FLT_MAX;
	DigitalMicrograph::Image Clone = DigitalMicrograph::ImageClone(IMG);
	Gatan::PlugIn::ImageDataLocker CloneLock(Clone);
	float* ClonePix = (float*) CloneLock.get();
	DigitalMicrograph::Image NoiseNegate;
	DigitalMicrograph::Component oval;
	std::vector<long> unit, unitref,xposref, yposref;
	std::vector<float> radiiref;
	int i=0;
	int j,k,s;
	s=0;
	float maxrunfl;
	DigitalMicrograph::GetNumber("Approximate number of discs",maxrun,&maxrunfl);
	maxrun = (long) maxrunfl;
	while(iradius>minrad && i<maxrun)
	{
		DigitalMicrograph::OpenAndSetProgressWindow("Finding Bright Discs",("Disc Number: " + boost::lexical_cast<std::string>(i+1)).c_str(),"Please wait...");
		(*xpos).resize(s + 1);
		(*ypos).resize(s + 1);
		(*radii).resize(s + 1);
		iradius = DiscSize(Clone,&ixpos,&iypos,2);
		if(iradius < imgX/6 -0.5 || (averagerad != 0 && iradius < averagerad*2))
		{
		(*xpos)[s] = ixpos;
		(*ypos)[s] = iypos+1;
		(*radii)[s] = iradius;
		averagerad = (averagerad*(s-1) + iradius)/s;
		s++;
		}
		i++;
		InverseCircularMask2D(&Clone,ixpos,iypos,iradius+10);
	}
	i--;
	s--;
	unit.resize(s);
	unitref.resize(s);
	xposref = (*xpos);
	yposref = (*ypos);
	radiiref = (*radii);
		for(j=0; j<s; j++)
	{
		unit[j] = (*ypos)[j]*imgX + (*xpos)[j];
		unitref[j] = (*ypos)[j]*imgX + (*xpos)[j];
	}
	boost::sort(unit);
	for(j=0; j<s; j++)
	{
		for(k=0; k<s; k++)
		{
			if(unit[j] == unitref[k])
			{
				(*xpos)[j] = xposref[k];
				(*ypos)[j] = yposref[k];
				(*radii)[j] = radiiref[k];
			}
		}

	}

	for(j=0; j<s; j++)
	{
		oval = DigitalMicrograph::NewOvalAnnotation((*ypos)[j]-(*radii)[j],(*xpos)[j]-(*radii)[j],(*ypos)[j] + (*radii)[j], (*xpos)[j]+(*radii)[j]);
		DigitalMicrograph::ComponentAddChildAtEnd((IMG).GetImageDisplay(0),oval);
		ComponentColour(&IMG,oval,0,0,1);
		DigitalMicrograph::Result("[" + boost::lexical_cast<std::string>((*xpos)[j]) + "," + boost::lexical_cast<std::string>((*ypos)[j]) + "]" + ";  radius = " + boost::lexical_cast<std::string>((*radii)[j]) + "\n");
	}
	
	return(i);

}

void ExtraGatan::DrawRedX(DigitalMicrograph::Image *input)
{
	long imgX, imgY;
	DigitalMicrograph::ImageDisplay img_disp;
	DigitalMicrograph::Get2DSize(*input,&imgX,&imgY);
	DigitalMicrograph::Component left = DigitalMicrograph::NewLineAnnotation(0,0,imgX,imgY);
	DigitalMicrograph::Component right = DigitalMicrograph::NewLineAnnotation(imgY,0,0,imgX);
	try
		{
		img_disp = DigitalMicrograph::ImageGetImageDisplay(*input,0);
		}
	catch(...)
		{
		(*input).GetOrCreateImageDocument().ShowAtPosition(200,200);
		DigitalMicrograph::SetWindowSize(*input,200,200);
		img_disp = DigitalMicrograph::ImageGetImageDisplay(*input,0);
		}
	DigitalMicrograph::ComponentAddChildAtEnd(img_disp,left);
	DigitalMicrograph::ComponentAddChildAtEnd(img_disp,right);
	ComponentColour(input,left,1,0,0);
	ComponentColour(input,right,1,0,0);
}

long ExtraGatan::MultiLocalPeak(DigitalMicrograph::Image IMG, std::vector<long> *xpos, std::vector<long> *ypos,  std::vector<float> *radii, float minrad,int num)
	//this routine could be amazing if I had the time to work out why it's so broken.
{
	float iradius;
	long ixpos,iypos, imgX, imgY;
	DigitalMicrograph::Get2DSize(IMG,&imgX,&imgY);
	iradius = FLT_MAX;
	DigitalMicrograph::Image Clone = DigitalMicrograph::ImageClone(IMG);
	DigitalMicrograph::Component oval;
	std::vector<long> unit, unitref,xposref, yposref;
	std::vector<float> radiiref;
	int i=0;
	int j,k;
	int size = num;
	int w = 0;
	*radii = LocalDiscSize(IMG, xpos,ypos,num);
	unit.resize(num);
	unitref.resize(num);
	xposref = (*xpos);
	yposref = (*ypos);
	radiiref = (*radii);
	for(j=0; j<num; j++)
	{
		if(radiiref[j] >= imgX/6-0.5)
		{
			unit[w] = (*ypos)[j]*imgX + (*xpos)[j];
			unitref[w] = (*ypos)[j]*imgX + (*xpos)[j];
			w++;
		}
		else
		{
			size--;	
		}
	}
	unit.resize(size);
	(*xpos).resize(size);
	(*ypos).resize(size);
	(*radii).resize(size);
	boost::sort(unit);
	for(j=0; j<size; j++)
	{
		for(k=0; k<i; k++)
		{
			if(unit[j] == unitref[k])
			{
				
				(*xpos)[j] = xposref[k];
				(*ypos)[j] = yposref[k];
				(*radii)[j] = radiiref[k];
				
				
			}
		}

	}

	for(j=0; j<num; j++)
	{
		if((*radii)[j] != 0)
		{
		oval = DigitalMicrograph::NewOvalAnnotation((*ypos)[j]-(*radii)[j],(*xpos)[j]-(*radii)[j],(*ypos)[j] + (*radii)[j], (*xpos)[j]+(*radii)[j]);
		DigitalMicrograph::ComponentAddChildAtEnd((IMG).GetImageDisplay(0),oval);
		DigitalMicrograph::Result("[" + boost::lexical_cast<std::string>((*xpos)[j]) + "," + boost::lexical_cast<std::string>((*ypos)[j]) + "]" + ";  radius = " + boost::lexical_cast<std::string>((*radii)[j]) + "\n");
		}
	}
	
	return(i);

}

void ExtraGatan::ComponentColour(DigitalMicrograph::Image *In, DigitalMicrograph::Component child, long r, long g, long b) //broken to all hell
{
    static DigitalMicrograph::Function __sFunction = (DM_FunctionToken) NULL;
	long imgID = ((*In).GetID());
	long compID = child.GetID();
    static const char *__sSignature = "void ExternalColourChange(long, long , long, long, long)";
    Gatan::PlugIn::DM_Variant params[5];
	params[0].v_uint32 = imgID;
	params[1].v_uint32 = compID;
	params[2].v_uint32 = r;
	params[3].v_uint32 = g;
	params[4].v_uint32 = b;
	GatanPlugIn::gDigitalMicrographInterface.CallFunction( __sFunction.get_ptr(), 5, params, __sSignature );
}

void ExtraGatan::SafeSave(std::string path, DigitalMicrograph::Image input)
{
		if(DigitalMicrograph::DoesFileExist(path))
		{DigitalMicrograph::DeleteFileA(path);}
		DigitalMicrograph::SaveImage(input,(path));
}