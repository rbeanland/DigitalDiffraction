#pragma once

#include "stdafx.h"
#include <math.h>
#include "boost/lexical_cast.hpp"
#include <boost\range\algorithm\sort.hpp>
#include <sstream>
#include "standardfunctions.h"
#include "utility.h"


//** Some functions to fill the gaps in the Digital Micrograph SDK    **\\
//**	  Richard Beanland (scripts)	  r.beanland@warwick.ac.uk    **\\ 
//**	  Adam Dyson (camera functions)   m.a.dyson@warwick.ac.uk	  **\\
//**	  Joel Forster (C++ conversion)	  j.c.forster@warwick.ac.uk   **\\
//**	  James Gott (C++ conversion continued) j.a.gott@warwick.ac.uk**\\


class ExtraGatan
{
public:
	ExtraGatan();
	static void UpdateCamera(DigitalMicrograph::Image *img_disp, double expo, int binning);
	static DigitalMicrograph::Image tert(DigitalMicrograph::Image INPUT, float thr); //replaces the tert() function from DM
	static DigitalMicrograph::Image Normalise(DigitalMicrograph::Image IMG1, float max); //divides every pixel in an image by the specificed max value
	static float Max(DigitalMicrograph::Image IMG1); //finds the brightest pixel in an image
	static float Min(DigitalMicrograph::Image IMG1); //finds the dimmest pixel in an image JG
	static long Sum(float*,int); //finds the sum of pixel values in a 1D image JG
	static float Max(DigitalMicrograph::Image IMG1, long* x, long* y); //finds the brightest pixel in an image
	static void PixelPos(DigitalMicrograph::Image IMG1, float min, long* imgX, long* imgY, bool avg); //finds the x and y of a given pixel value, if avg is switched on, it gets the average location between equivalent values
	static std::string EMGetImagingOpticsMode(); //gets the current microscope mode
	static DigitalMicrograph::ScriptObject GetFrameSetInfo (DigitalMicrograph::ScriptObject &Acquis); //used in image aquisition
	static DigitalMicrograph::ScriptObject GetFrameSetInfoPtr (DigitalMicrograph::ScriptObject &Acquis);
	static bool EMChangeMode(std::string mode_want); //asks the user to change to the mode required by a function, returns if the user clicked cancel
	static void EMBeamCentre(std::string Tag_Path, DigitalMicrograph::Image IMG1); //centres the beam on a specified point JG
	static void EMGetBeamShift(long* shiftx, long* shifty); //gets the amount the beam has been shifted from it's original position
	static void EMGetBeamTilt(long* tiltx, long* tilty); //gets the amount the beam has been tilted
	static void GetDateAndTime(std::string* dateout, std::string* timeout);
	static float AverageTemp(DigitalMicrograph::Image IMG);
	static float DiscSize(DigitalMicrograph::Image IMG, long *x0, long *y0, int mode); //mode 1 is centred, mode 2 is positioned on the max pixel in the image, mode 3 scans entire image VERY SLOWLY
	static float DiscSize(DigitalMicrograph::Image IMG, long *x0, long *y0, long xloc, long yloc); //gets a disc size based on a specific user location. Will be a very powerful tool when 
	static float GaussSize(DigitalMicrograph::Image IMG, long *x0, long *y0,float* intensity); //finds a disc size, please use mode 2 if in doubt.
	static std::vector<float> LocalDiscSize(DigitalMicrograph::Image IMG, std::vector<long> *x0, std::vector<long> *y0,int num);
	static float round(float number);
	static float EMGetSpotSize(); //no clue
	static float EMGetMagnification(); //gets the current magnification level of the microscope
	static bool CheckCamera(); //checks the camera is inserted, flags if it isn't
	static DigitalMicrograph::Image  Acquire(int bin, bool* quit, bool* success, double expo); //aquires an image from the camera
	static DigitalMicrograph::Image  Acquire(DigitalMicrograph::Image AcquiredImage,int bin, bool* quit, bool* success, double expo);
	static DigitalMicrograph::Image RemoveOutliers(DigitalMicrograph::Image Img, long *t, long *l, long *b, long *r, float thr); //removes the outliers in an ROI
	//static long EMGetCameraLength();
	static double EMGetCameraLength();
	static long GetDataLockRange(long imgX, long x, long y);
	static long GetDataLockRange(long imgX, long t, long b, long r, long l, long* start, long* finish);
	static long GetDataLockRange(long t, long b, long r, long l, long imgX, long imgY, long stackNo, long* start, long* finish);
	static void GetPixelXandY(long element, long* pix_X, long*pix_Y, long imgX);
	static float Tiltsize(float dTilt, float *T1X, float *T1Y, float *T2X, float *T2Y, int binning, DigitalMicrograph::Image *img1, DigitalMicrograph::Image *imgCC, DigitalMicrograph::Image *img0, float expo, float sleeptime, long _b);
	static float Shiftsize(float dShift, float *Sh1X, float *Sh1Y,float *Sh2X,float *Sh2Y, int binning, float expo, float sleeptime, long _b);
	static void intrinsic(long imgX, long imgY, DigitalMicrograph::Image * icol, DigitalMicrograph::Image * irow); //returns an image of icol and iheight values for use in for loops etc
	static void ResetBeam(); //resets the beam to its original position
	static void ABSTilt(long tiltx, long tilty); //tilts the beam to an absolute number
	static void ABSShift(long shiftx, long shifty); //shifts the beam to an absolute number
	static void CircularMask2D(DigitalMicrograph::Image *input,long pix_X,long pix_y,float radius); //creates a circular mask
	static void InverseCircularMask2D(DigitalMicrograph::Image *input,long pix_X,long pix_y,float radius); //creates a circular mask
	static void CircularMask3D(DigitalMicrograph::Image *input,long pix_X,long pix_y,float radius); //creates a circular mask on every image in a stack
	static DigitalMicrograph::Image MakeDisc(long imgX, long imgY, float radius); //makes a central disc of value 1 in a surrounding field of 0s.
	static DigitalMicrograph::Image MakeDisc(long imgX, long imgY, float radius, long x0, long y0); //makes a disc with value 1 at position x0, y0 with a specified radius, and 0 elsewhere. For correlation purposes.
	static DigitalMicrograph::Image MakeDisc(long imgX, long imgY, float radius, long x0, long y0, float intensity);
	static DigitalMicrograph::Image MakeGradedDisc(long imgX, long imgY, float radius);
	static DigitalMicrograph::Image MakeGradedDisc(long imgX, long imgY, float radius, long x0, long y0);
	static DigitalMicrograph::Image MakeGradedDisc(long imgX, long imgY, float radius, long x0, long y0, float intensity);
	static DigitalMicrograph::Image Starfield(long imgX, long imgY);
	static DigitalMicrograph::Image GradedStarfield(long imgX, long imgY);
	static DigitalMicrograph::Image NoisyStarfield(long imgX, long imgY);
	static DigitalMicrograph::Image GradedNoisyStarfield(long imgX, long imgY);
	static DigitalMicrograph::Image GradedGaussStarfield(long imgX, long imgY,float intensity, float SD);
	static DigitalMicrograph::Image MakeGrid(long imgX, long imgY);
	static DigitalMicrograph::Image ImageMaskTert(bool high, DigitalMicrograph::Image img1, DigitalMicrograph::Image img2); //takes two images, in high mode if a pixel in img1 is brighter than img2, it replaces the pixel in img1 with the one in img2. In low mode, if it is darker it replaces it.
	static DigitalMicrograph::Image RandMap(long inX, long inY);
	static DigitalMicrograph::Image RandMap(DigitalMicrograph::Image output);
	static DigitalMicrograph::Image RandMap(DigitalMicrograph::Image output, long intensity);
	static DigitalMicrograph::Image GaussianNoise(long inX, long inY,float SD, float intensity);
	static std::vector<float> FindLocalMaxima(DigitalMicrograph::Image input, int sectors, bool avg, std::vector<long> *xpos,  std::vector<long> *ypos); //splits the image into a number of sectors and finds the max pixel in each sector, then gets their locations as global coordinates, returns a vector arranged thus: [x0,y0,x1,y1,x2...] up to 64 sectors
	static DigitalMicrograph::Image DiffImage(DigitalMicrograph::Image input, bool smooth); //make a 2d surface plot of an image, differentiate it, return the differentiated surface as an image
	static std::vector<long> FindPeaks(DigitalMicrograph::Image input);
	static void EMGetStageXY (double &StageX, double &StageY);
	static void EMSetStageXY (double StageX, double StageY);
	static void EMSetStageX (double StageX);
	static void EMSetStageY (double StageY);
	static double EMGetStageX();
	static double EMGetStageY();
	static double EMGetHighTensionOffset();
	static void EMSetHighTensionOffset(double offset);
	static void EMSetBrightness( double brightness );
	static double EMGetBrightness( );
	static float Interpolate(DigitalMicrograph::Image *input,long xCal,long yCal,long pX,long nX,long pY,long nY, long nnX, long nnY);
	static DigitalMicrograph::Image DrawPlot(DigitalMicrograph::Image* input, long x0, long y0, long x1, long y1);
	static void ClearGaussianNoise(DigitalMicrograph::Image *IMG);
	static long MultiPeak(DigitalMicrograph::Image IMG, std::vector<long> *xpos, std::vector<long> *ypos,  std::vector<float> *radius, float minrad, long maxrun);
	static long MultiLocalPeak(DigitalMicrograph::Image IMG, std::vector<long> *xpos, std::vector<long> *ypos,  std::vector<float> *radii, float minrad,int num);
	static void ComponentColour(DigitalMicrograph::Image *In, DigitalMicrograph::Component child,long r, long g, long b);
	static void DrawRedX(DigitalMicrograph::Image *input);
	static DigitalMicrograph::Image CrossCorr(DigitalMicrograph::Image Cc, DigitalMicrograph::Image img1, DigitalMicrograph::Image img2);
	static DigitalMicrograph::Image CrossCorr2(DigitalMicrograph::Image Cc, DigitalMicrograph::Image img1, DigitalMicrograph::Image img2);
	static void SafeSave(std::string path, DigitalMicrograph::Image input);

	static float maxoftwo(float, float);// returns the max from two inputs JG
	static float minoftwo(float, float);//returns the min of two inpits JG
	static void setCompCol(DigitalMicrograph::Component annot, float r, float g, float b);//change the colour of a component, requires the installation of setaCol() script
	static void ExtraGatan::InvCircularMask2D(DigitalMicrograph::Image *input, long pix_X, long pix_Y, float radius);//JG
	static double ExtraGatan::altInterpolate(DigitalMicrograph::Image* input, long xCal, long yCal, long pX, long nX, long pY, long nY, long nnX, long nnY);//JG
	static void  ExtraGatan::sAcquire(DigitalMicrograph::Image* Acquired, int bin, bool* quit, bool* success, double expo);//No longer useful, replaced by acquisition class
	static float ExtraGatan::Min3D(DigitalMicrograph::Image IMG1);//Returns the max pixel value form a 3D image JG
	static float ExtraGatan::Max3D(DigitalMicrograph::Image IMG1);// Returns the min pixel value from a 3D image JG
};