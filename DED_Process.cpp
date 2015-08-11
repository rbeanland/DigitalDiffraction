#include "stdafx.h"
#include "DED_Process.h"

void Process::ROIpos(DigitalMicrograph::Image Img,std::string prompt,double &t, double &l, double &b, double &r)
{
	long imgX,imgY;// size of i nput image
	bool ClickedOK;
	DigitalMicrograph::Get2DSize(Img,&imgX,&imgY);
	DigitalMicrograph::ImageDisplay img_disp;
	img_disp = DigitalMicrograph::ImageGetImageDisplay(Img,0);
	DigitalMicrograph::ROI theROI=DigitalMicrograph::NewROI();
	DigitalMicrograph::ROISetRectangle(theROI,t,l,b,r);
	DigitalMicrograph::ImageDisplayAddROI(img_disp,theROI);// add ROI square to the image
	ClickedOK=true;
	//Pause the program and allow user to move ROI until OK is pressed
	long sem;
	sem = DigitalMicrograph::NewSemaphore();
	try
	{
		DigitalMicrograph::ModelessDialog(prompt.c_str(),"OK",sem);
		DigitalMicrograph::GrabSemaphore(sem);
		DigitalMicrograph::ReleaseSemaphore(sem);
		DigitalMicrograph::FreeSemaphore(sem);
	}
	catch(...)
	{
		DigitalMicrograph::FreeSemaphore(sem);
		ClickedOK=false;
		//return;
	}

	DigitalMicrograph::ImageDisplayDeleteROI(img_disp,theROI);
	float _t,_l,_b,_r;
	_t=(float)t;
	_l=(float)l;
	_b=(float)b;
	_r=(float)r;

	if(ClickedOK)
	{
		DigitalMicrograph::ROIGetRectangle(theROI,&_t,&_l,&_b,&_r);//Get the coordinates of the ROI position
		t = _t;
		l = _l;
		b = _b;
		r = _r;
	}
	else
	{
		DigitalMicrograph::Result("Cancelled setting ROI\n");
		return;
	}
	//End of function ROIpos
}

void Process::GetCoordsFromNTilts(long nTilts,long currentPoint, int &i, int &j)
{
	int side = 2*nTilts+1;
	try
	{
		j = floor(((double)currentPoint) / ((double)side)) - nTilts;
		i = ((currentPoint % side) - nTilts)*pow((float)(-1), (int)(j % 2));//NB % means modulo, flips sign every row
	}
	catch (...)
	{
		DigitalMicrograph::Result("Failed in GetCoords\n");
		return;
	}

}

void Process::AddBlueCircle(DigitalMicrograph::Image *img,float t, float l, float b, float r)//Add a circle with coordinates t,l,b,r
{
	DigitalMicrograph::ImageDisplay imgdisp=DigitalMicrograph::ImageGetImageDisplay(*img,0);
	DigitalMicrograph::Component ov;
	ov=DigitalMicrograph::NewOvalAnnotation(t,l,b,r);
	DigitalMicrograph::ComponentAddChildAtEnd(imgdisp, ov);	//Add the circle to the image

	bool enable_colour=false;
	std::string Tag_path;
	Tag_path = "DigitalDiffraction:Settings:";
	try
	{
		DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (Tag_path + "Enable Colour Change").c_str(), &enable_colour);
	}
	catch (...)	{	}
	if (enable_colour == true)
	{
		ExtraGatan::setCompCol(ov, 0, 0, 1);//Make it blue
	}
	//End of AddBlueCircle
}

void Process::AddYellowArrow(DigitalMicrograph::Image *img, float x0, float y0, float x1, float y1) //Add an arrow with coordinates t,l,b,r
{
	DigitalMicrograph::ImageDisplay imgdisp = DigitalMicrograph::ImageGetImageDisplay(*img, 0);
	DigitalMicrograph::Component arrowannot;
	arrowannot = DigitalMicrograph::NewArrowAnnotation(y0, x0, y1, x1);
	DigitalMicrograph::ComponentAddChildAtEnd(imgdisp, arrowannot);// add arrow to image

	bool enable_colour=false;
	std::string Tag_path;
	Tag_path = "DigitalDiffraction:Settings:";
	try
	{
		DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (Tag_path + "Enable Colour Change").c_str(), &enable_colour);
	}
	catch (...)
	{
		DigitalMicrograph::Result("No setting saved for enable colour change\n");
	}
	if (enable_colour == true)
	{
		ExtraGatan::setCompCol(arrowannot, 1, 1, 0);//make it yellow
	}
	//end of AddYellowArrow
}

void Process::UserG(DigitalMicrograph::Image Avg, float Rr, float &g1X, float &g1Y, float &g2X, float &g2Y, float &pX, float &pY)
{
	double t, l, b, r;
	long imgX, imgY;
	std::string prompt;
	DigitalMicrograph::Image AvgTemp;

	DigitalMicrograph::Get2DSize(Avg, &imgX, &imgY);
	//Create new average image and get rid of the old one
	AvgTemp = DigitalMicrograph::ImageClone(Avg);
	DigitalMicrograph::DeleteImage(Avg);
	
	Avg = DigitalMicrograph::ImageClone(AvgTemp);
	Avg.GetOrCreateImageDocument().ShowAtPosition(50, 30);
	DigitalMicrograph::SetWindowSize(Avg, 900, 900);
	DigitalMicrograph::SetName(Avg, "Average CBED image");
	
	DigitalMicrograph::DeleteImage(AvgTemp);
	//Get g-vectors manually
	prompt = "Position ROI on central beam and hit OK";
	DigitalMicrograph::Result(prompt+"...");
	
	float i=0, j=0;
	t = (double)(pY + g1Y*i + g2Y*j - (float)Rr);
	l = (double)(pX + g1X*i + g2X*j - (float)Rr);
	b = (double)(pY + g1Y*i + g2Y*j + (float)Rr);
	r = (double)(pX + g1X*i + g2X*j + (float)Rr);
	double t1 = t;

	ROIpos(Avg, prompt, t, l, b, r);
	DigitalMicrograph::Result("Done\n");

	pX = (l + r) / 2;
	pY = (t + b) / 2;
	//first diffraction vector
	prompt = "Position ROI on 1st g and hit OK";
	DigitalMicrograph::Result(prompt + "...");

	i = 2;
	j = 0;
	t = (double)(pY + g1Y*i + g2Y*j - (float)Rr);
	l = (double)(pX + g1X*i + g2X*j - (float)Rr);
	b = (double)(pY + g1Y*i + g2Y*j + (float)Rr);
	r = (double)(pX + g1X*i + g2X*j + (float)Rr);
	ROIpos(Avg, prompt, t, l, b, r);

	float order = 2;
	if (!DigitalMicrograph::GetNumber("Diffraction order?", order, &order))
	{	return;	}
	DigitalMicrograph::Result(" Done\n");
	g1X = (((l + r) / 2) - pX) / order;
	g1Y = (((t + b) / 2) - pY) / order;
	//second diffraction vector
	prompt = "Position ROI on 2nd g and hit OK";
	DigitalMicrograph::Result(prompt + "...");
	i = 0;
	j = 2;
	t = (double)(pY + g1Y*i + g2Y*j - (float)Rr);
	l = (double)(pX + g1X*i + g2X*j - (float)Rr);
	b = (double)(pY + g1Y*i + g2Y*j + (float)Rr);
	r = (double)(pX + g1X*i + g2X*j + (float)Rr);

	t1 = t;
	ROIpos(Avg, prompt, t, l, b, r);
	
	if (t == t1)
	{
		if (!DigitalMicrograph::OkCancelDialog("ROI has not been moved, continue?"))
		{
			return;
		}
	}
	
	order = 2;
	if (!DigitalMicrograph::GetNumber("Diffraction order?", order, &order))
	{	return;	}
	DigitalMicrograph::Result(" Done\n");
	g2X = (((l + r) / 2) - pX) / order;
	g2Y = (((t + b) / 2) - pY) / order;
	//show discs and g-vectors on average image
	int ii, jj;
	for (ii = -2; ii < 3; ii++)
	{
		for (jj = -2; jj < 3; jj++)
		{
			AddBlueCircle(&Avg, (pY + g1Y*(float)ii + g2Y*(float)jj - (float)Rr), (pX + g1X*(float)ii + g2X*(float)jj - (float)Rr), (pY + g1Y*(float)ii + g2Y*(float)jj + (float)Rr), (pX + g1X*(float)ii + g2X*(float)jj + (float)Rr));
		}
	}
	AddYellowArrow(&Avg, pX, pY, (pX + g1X), (pY + g1Y));
	AddYellowArrow(&Avg, pX, pY, (pX + g2X), (pY + g2Y));
	//End of function UserG
}

void Process::GetMeanG(DigitalMicrograph::Image *MagTheta,DigitalMicrograph::Image &Cluster ,float* Clusterpix,double &MeanMag,double &MeanTheta, double &MeanVx, double &MeanVy, int &nMeas)
{
	
	//incoming Mean values are a single vector
	//outgoing mean values are an average of the vectors deemed to be in the cluster
	//incoming mag/theta is the same for +/- vectors [since tan(q+pi)=tan(q)]
	//so flip the vectors if the x-component has an opposite sign
	double tolMag = 5;//tolerance in pixels to say it's in a cluster
	double tolAng = 5;//tolerance in degrees to say it's in a cluster
	long n, nVecs;
	int i;
	double dTheta, dMag, ThetaSum, VxSum, VySum, x, y, signX;
	VxSum = 0;
	VySum = 0;
	ThetaSum = 0;
	DigitalMicrograph::Get2DSize(Cluster, &n, &nVecs);
	nMeas = 0;
	double MagSum = 0;

	double currmagthetapix;
	double col2currentmagthetapix;
	for (i = 0; i < nVecs; i++)
	{
		currmagthetapix = DigitalMicrograph::GetPixel(*MagTheta, 0, i);
		dMag = abs(MeanMag - currmagthetapix);
		col2currentmagthetapix = DigitalMicrograph::GetPixel(*MagTheta, 1, i);
		dTheta = abs(MeanTheta - col2currentmagthetapix);

		if ((dTheta < tolAng) && (dMag < tolMag))
		{
			nMeas++;
			MagSum += currmagthetapix;
			ThetaSum += col2currentmagthetapix;

			x = DigitalMicrograph::GetPixel(*MagTheta, 2, i);
			y = DigitalMicrograph::GetPixel(*MagTheta, 3, i);
			signX = (abs(x)) / x;

			VxSum += x*((abs(MeanVx)) / MeanVx)*signX;//second part here reverses
			VySum += y*((abs(MeanVx)) / MeanVx)*signX;//sign if x is opposite sign

			Clusterpix[i] = 1;//it is in the cluster

			MeanMag = MagSum / (double)nMeas;
			MeanTheta = ThetaSum / (double)nMeas;
			MeanVx = VxSum / (double)nMeas;
			MeanVy = VySum / (double)nMeas;

		} //end of if(dtheta)
	}//end of for(i)	
}//End of GetMeanG

void Process::GetG_Vectors(DigitalMicrograph::Image Avg, float Rr, float &g1X, float &g1Y, float &g2X, float &g2Y, long &pX, long &pY)
{
	long nPeaks = 25;//maximum number of peaks to measure in the cross correlation
	long imgX, imgY;
	DigitalMicrograph::Get2DSize(Avg, &imgX, &imgY);

	//start by getting a cross-correlation image
	//image of a blank disk, radius Rr
	DigitalMicrograph::Image Disc = ExtraGatan::MakeDisc(imgX, imgY, Rr);
	DigitalMicrograph::Image AvCC = DigitalMicrograph::CrossCorrelate(Avg, Disc);
	Gatan::PlugIn::ImageDataLocker avcclock(AvCC);
	float* avccpix = (float*)avcclock.get();

	long avccX, avccY;
	DigitalMicrograph::Get2DSize(AvCC, &avccX, &avccY);
	double maxval;
	long xp, yp;
	maxval = ExtraGatan::Max(AvCC);
	ExtraGatan::PixelPos(AvCC, maxval, &xp, &yp, false);
	pX = xp;
	pY = yp;

	std::string Tag_path;
	Tag_path = "DigitalDiffraction:Settings:";

	float dSize = 3.5;//a multiplying factor for the disc size when deleting peaks which have already been maeasured. Ideally dSize*Rr should be half of the smallest g-vector.
	float DdSize = ExtraGatan::round(dSize*Rr);
	DigitalMicrograph::DeleteImage(Disc);//tidy up
	//delete all info below 2% of best correlation
	float _top = maxval*0.02;
	DigitalMicrograph::Result("about to tert...\n");
	for (int i = 0; i < (avccX*avccY); i++)
	{
		if (avccpix[i]>_top)
		{avccpix[i] = avccpix[i];}
		else{avccpix[i] = 0.001;}
	}
	DigitalMicrograph::ImageDataChanged(AvCC);

	DigitalMicrograph::Image X, Y, TempImg;
	//x- and y-coords as column vectors
	X = DigitalMicrograph::RealImage("X-coords", 4, 1, nPeaks);
	Y = DigitalMicrograph::RealImage("Y-coords", 4, 1, nPeaks);
	TempImg = DigitalMicrograph::RealImage("Deleting disc", 4, (2 * DdSize), (2 * DdSize));
	Gatan::PlugIn::ImageDataLocker templock(TempImg);
	float* temppix = (float*)templock.get();

	for (int i = 0; i < ((2 * DdSize)*(2 * DdSize)); i++)
	{
		temppix[i] = 1;
	}
	ExtraGatan::InvCircularMask2D(&TempImg, DdSize, DdSize, DdSize);//Masking it to be a circle

	int i,k,n;
	int Nmax = nPeaks;
	bool flag = false;
	
	for (i = 0; i < nPeaks; i++)
	{//get peak position, in descending order of correlation/intensity
		maxval = ExtraGatan::Max(AvCC);
		if (maxval>_top)
		{//only keep going if there are peaks to be found
			ExtraGatan::PixelPos(AvCC, maxval, &xp, &yp, false);
			DigitalMicrograph::SetPixel(X, 0, i, xp);
			DigitalMicrograph::SetPixel(Y, 0, i, yp);
			AddBlueCircle(&Avg, yp - Rr, xp - Rr, yp + Rr, xp + Rr);
			DigitalMicrograph::SetSelection(AvCC, (yp - DdSize), (xp - DdSize), (yp + DdSize), (xp + DdSize));
			for (k = 0; k < 2 * DdSize; k++)
			{
				for (n = 0; n < 2 * DdSize; n++)
				{
					avccpix[(n+yp-(long)DdSize)*imgX+(k+xp-(long)DdSize)] *= temppix[(2 * (long)DdSize)*n + k];
				}
			}
			
			DigitalMicrograph::ImageDataChanged(AvCC);
			DigitalMicrograph::ClearSelection(AvCC);
		}
		else
		{ //No peaks left
			if (flag == false)
			{
				Nmax = i;//reduce number of peaks
				flag = true;
			}
		}
	}//end of for i < nPeaks

	nPeaks = Nmax;
	if (nPeaks < 3)
	{
		return;
	}
	//Find difference vectors Vx, Vy, by replicationg X and Y into square matrices Xx, Yy, and subtracting the transpose

	DigitalMicrograph::Image Xx, Xxtrans, Vx, Yy, Yytrans, Vy;

	bool display_xx = false;
	bool display_yy = false;
	try
	{
		DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (Tag_path + "Display Y Yy Vy").c_str(), &display_yy);
		DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (Tag_path + "Display X Xx Vx").c_str(), &display_xx);
	}
	catch (...)	{	}

	Xx = DigitalMicrograph::RealImage("Xx", 4, nPeaks, nPeaks);
	Xxtrans = DigitalMicrograph::RealImage("XxTransposed", 4, nPeaks, nPeaks);
	Vx = DigitalMicrograph::RealImage("Vx", 4, nPeaks, nPeaks);

	Yy = DigitalMicrograph::RealImage("Yy", 4, nPeaks, nPeaks);
	Yytrans = DigitalMicrograph::RealImage("YyTransposed", 4, nPeaks, nPeaks);
	Vy = DigitalMicrograph::RealImage("Vy", 4, nPeaks, nPeaks);

	Gatan::PlugIn::ImageDataLocker xlock(X);
	float* xpix = (float*)xlock.get();

	Gatan::PlugIn::ImageDataLocker ylock(Y);
	float* ypix = (float*)ylock.get();

	Gatan::PlugIn::ImageDataLocker xxlock(Xx);
	float* xxpix = (float*)xxlock.get();

	Gatan::PlugIn::ImageDataLocker xxtranslock(Xxtrans);
	float* xxtranspix = (float*)xxtranslock.get();

	Gatan::PlugIn::ImageDataLocker vxlock(Vx);
	float* vxpix = (float*)vxlock.get();

	Gatan::PlugIn::ImageDataLocker yylock(Yy);
	float* yypix = (float*)yylock.get();

	Gatan::PlugIn::ImageDataLocker yytranslock(Yytrans);
	float* yytranspix = (float*)yytranslock.get();

	Gatan::PlugIn::ImageDataLocker vylock(Vy);
	float* vypix = (float*)vylock.get();

	int ii,jj;
	float complete = 1000000;//an arbitrary number larger than anything else

	//create Xx matrix, copies a vertical single pixel width X image into all npeak columns
	for (ii = 0; ii < nPeaks; ii++)
	{
		for (jj = 0; jj < nPeaks; jj++)
		{
			xxpix[nPeaks*jj + ii] = xpix[jj];
		}
	}
	//create transpose of matrix Xx
	for (ii = 0; ii < nPeaks; ii++)
	{
		for (jj = 0; jj < nPeaks; jj++)
		{
			xxtranspix[nPeaks*jj+ii] = xxpix[nPeaks*ii+jj];
		}
	}
	//Vx=xxtrans-xx
	for (ii = 0; ii < nPeaks; ii++)
	{
		for (jj = 0; jj < nPeaks; jj++)
		{
			if ((xxtranspix[nPeaks*jj + ii] - xxpix[nPeaks*jj + ii]) == 0)//get rid of divide by zero error
			{
				vxpix[nPeaks*jj + ii] = complete;
			}
			else
			{
				vxpix[nPeaks*jj + ii] = xxtranspix[nPeaks*jj + ii] - xxpix[nPeaks*jj + ii];
			}
		}
	}
	//create Yy matrix, copies a vertical single pixel width Y image into all npeak columns
	for (ii = 0; ii < nPeaks; ii++)
	{
		for (jj = 0; jj < nPeaks; jj++)
		{
			yypix[nPeaks*jj + ii] = ypix[jj];
		}
	}
	//create transpose of matrix Yy
	for (ii = 0; ii < nPeaks; ii++)
	{
		for (jj = 0; jj < nPeaks; jj++)
		{
			yytranspix[nPeaks*jj + ii] = yypix[nPeaks*ii + jj];
		}
	}
	//Vy=yytrans-yy
	for (ii = 0; ii < nPeaks; ii++)
	{
		for (jj = 0; jj < nPeaks; jj++)
		{
			vypix[nPeaks*jj + ii] = yytranspix[nPeaks*jj + ii] - yypix[nPeaks*jj + ii];
		}
	}

	if (display_xx == true)
	{
		DigitalMicrograph::ImageDocumentShowAtPosition(DigitalMicrograph::ImageGetOrCreateImageDocument(Xx), 15, 30);
		DigitalMicrograph::ImageDocumentShowAtPosition(DigitalMicrograph::ImageGetOrCreateImageDocument(Xxtrans), 25, 30);
		DigitalMicrograph::ImageDocumentShowAtPosition(DigitalMicrograph::ImageGetOrCreateImageDocument(Vx), 35, 30);
	}

	if (display_yy == true)
	{
		DigitalMicrograph::ImageDocumentShowAtPosition(DigitalMicrograph::ImageGetOrCreateImageDocument(Yy), 15, 30);
		DigitalMicrograph::ImageDocumentShowAtPosition(DigitalMicrograph::ImageGetOrCreateImageDocument(Yytrans), 25, 30);
		DigitalMicrograph::ImageDocumentShowAtPosition(DigitalMicrograph::ImageGetOrCreateImageDocument(Vy), 35, 30);
	}
	
	//Polar coordinates, Vmag and Vtheta
	DigitalMicrograph::Image Vmag, Vtheta;
	Vmag = DigitalMicrograph::RealImage("Vmag", 4, nPeaks, nPeaks);
	Gatan::PlugIn::ImageDataLocker VmagLock(Vmag);
	float* vmagpix = (float*)VmagLock.get();
	
	bool display_mag_theta = false;
	try
	{
		DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (Tag_path + "Display Mag Theta").c_str(), &display_mag_theta);
	}
	catch (...)	{	}
	
	for (ii = 0; ii < nPeaks; ii++)
	{
		for (jj = 0; jj < nPeaks; jj++)
		{
			if (jj >= ii)//gives bottom left diagonal half
			{
				vmagpix[nPeaks*jj + ii] = sqrt((vxpix[nPeaks*jj + ii] * vxpix[nPeaks*jj + ii]) + (vypix[nPeaks*jj + ii] * vypix[nPeaks*jj + ii]));
				if (vmagpix[nPeaks*jj + ii] == 0)
				{
					vmagpix[nPeaks*jj + ii] = complete;
				}
			}
			else
			{
				vmagpix[nPeaks*jj + ii] = complete;
			}
		}
	}

	if (display_mag_theta == true)
	{
		DigitalMicrograph::ImageDocumentShowAtPosition(DigitalMicrograph::ImageGetOrCreateImageDocument(Vmag), 15, 30);
	}

	Vtheta = DigitalMicrograph::RealImage("Vtheta", 4, nPeaks, nPeaks);
	Gatan::PlugIn::ImageDataLocker VthetaLock(Vtheta);
	float* vthetapix = (float*)VthetaLock.get();

	for (ii = 0; ii < nPeaks; ii++)
	{
		for (jj = 0; jj < nPeaks; jj++)
		{
			if (jj >= ii)//gives bottom left diagonal half
			{
				vthetapix[nPeaks*jj + ii] = atan(vypix[nPeaks*jj + ii] / vxpix[nPeaks*jj + ii]);
			}
			else
			{
				vthetapix[nPeaks*jj + ii] = 0;
			}
		}
	}
		
	double pi = 3.1415926535897932384626433832795;

	for (ii = 0; ii < nPeaks; ii++)
	{
		for (jj = 0; jj < nPeaks; jj++)
		{
			vthetapix[nPeaks*jj + ii] = vthetapix[nPeaks*jj + ii] * (180 / pi);
		}
	}

	if (display_mag_theta == true)
	{
		DigitalMicrograph::ImageDocumentShowAtPosition(DigitalMicrograph::ImageGetOrCreateImageDocument(Vtheta), 15, 30);
	}
	
	//sort magnitude ascending into new column vector MagTheta
	long nVecs = (nPeaks*nPeaks - nPeaks) / 2;//number of different vectors
	DigitalMicrograph::Image MagTheta;
	MagTheta = DigitalMicrograph::RealImage("Mag-Theta-X-Y", 4, 4, nVecs);
	Gatan::PlugIn::ImageDataLocker magthetaLock(MagTheta);
	float* magthetapix = (float*)magthetaLock.get();

	float Mag = ExtraGatan::Min(Vmag);//lowest magnitude in the list
	ExtraGatan::PixelPos(Vmag, Mag, &xp, &yp, false);

	int iii=0;
	while (Mag < complete)
	{
		magthetapix[iii * 4] = vmagpix[nPeaks*yp + xp];
		magthetapix[iii * 4 + 1] = vthetapix[nPeaks*yp + xp];
		magthetapix[iii * 4 + 2] = vxpix[nPeaks*yp + xp];
		magthetapix[iii * 4 + 3] = vypix[nPeaks*yp + xp];
		vmagpix[nPeaks*yp + xp] = complete;
		
		iii++;
		Mag = ExtraGatan::Min(Vmag);
		ExtraGatan::PixelPos(Vmag, Mag, &xp, &yp, false);
	}

	if (display_mag_theta == true)
	{
		DigitalMicrograph::ImageDocumentShowAtPosition(DigitalMicrograph::ImageGetOrCreateImageDocument(MagTheta), 15, 30);
	}

	//Find clusters - similar g-vectors in mag-theta space
	DigitalMicrograph::Image Cluster, gVectors;
	Cluster = DigitalMicrograph::RealImage("Cluster", 4, 1, nVecs);
	Gatan::PlugIn::ImageDataLocker clusterlock(Cluster);
	float* clusterpix = (float*)clusterlock.get();
	for (iii = 0; iii < nVecs; iii++)
	{
		clusterpix[iii] = 0;
	}
	
	gVectors = DigitalMicrograph::RealImage("g's", 4, 5, nVecs);
	Gatan::PlugIn::ImageDataLocker gveclock(gVectors);
	float* gvecpix = (float*)gveclock.get();
		
	bool display_cluster_gvec = false;
	try
	{
		DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (Tag_path + "Display Cluster and gVectors").c_str(), &display_cluster_gvec);
	}
	catch (...)	{	}

	double MeanMag, MeanTheta, MeanVx, MeanVy;
	MeanMag = DigitalMicrograph::GetPixel(MagTheta, 0, 0);
	MeanTheta = DigitalMicrograph::GetPixel(MagTheta, 1, 0);
	MeanVx = DigitalMicrograph::GetPixel(MagTheta, 2, 0);
	MeanVy = DigitalMicrograph::GetPixel(MagTheta, 3, 0);

	if (display_cluster_gvec == true)
	{
			DigitalMicrograph::ImageDocumentShowAtPosition(DigitalMicrograph::ImageGetOrCreateImageDocument(Cluster), 40, 30);
			DigitalMicrograph::ImageDocumentShowAtPosition(DigitalMicrograph::ImageGetOrCreateImageDocument(gVectors), 60, 30);
	}
	DigitalMicrograph::Result("MeanMag = " + boost::lexical_cast<std::string>(MeanMag)+", MeanTheta = "+boost::lexical_cast<std::string>(MeanTheta)+"\n");
	
	int jjj = 1, kkk = 0;
	int nMeas = 1;//number of measured points to give an average g-vector
	//Go through and get clusters
	while (ExtraGatan::Sum(clusterpix, nVecs) < nVecs)
	{
		GetMeanG(&MagTheta, Cluster, clusterpix, MeanMag, MeanTheta, MeanVx, MeanVy, nMeas);
		gvecpix[kkk * 5] = MeanMag;
		gvecpix[kkk * 5 + 1] = MeanTheta;
		gvecpix[kkk * 5 + 2] = MeanVx;
		gvecpix[kkk * 5 + 3] = MeanVy;
		gvecpix[kkk * 5 + 4] = nMeas;
		//Find next unmatched point
		DigitalMicrograph::ImageDataChanged(gVectors);
		iii = 0;
		while (jjj == 1)
		{
			iii++;
			jjj = DigitalMicrograph::GetPixel(Cluster, 0, iii);
		}
		jjj = 1;
		MeanMag = DigitalMicrograph::GetPixel(MagTheta, 0, iii);//next point
		MeanTheta = DigitalMicrograph::GetPixel(MagTheta, 1, iii);//next point
		clusterpix[iii] = 1;//next cluster
		kkk++;
	}
	g1X = DigitalMicrograph::GetPixel(gVectors, 2, 0);
	g1Y = DigitalMicrograph::GetPixel(gVectors, 3, 0);
	g2X = DigitalMicrograph::GetPixel(gVectors, 2, 1);
	g2Y = DigitalMicrograph::GetPixel(gVectors, 3, 1);
	
	bool print_g1x_g1y = false;
	try
	{
		DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (Tag_path + "Print g1X g1Y").c_str(), &print_g1x_g1y);
	}
	catch (...)	{	}

	if (print_g1x_g1y == true);
	{
		DigitalMicrograph::Result("g1X = " + boost::lexical_cast<std::string>(g1X)+", g1Y = " + boost::lexical_cast<std::string>(g1Y)+", g2X = " + boost::lexical_cast<std::string>(g2X)+", g2Y = " + boost::lexical_cast<std::string>(g2Y)+"\n");
	}

	DigitalMicrograph::Result("Found " + boost::lexical_cast<std::string>(nPeaks)+" different CBED disks, \n");
	DigitalMicrograph::Result("giving " + boost::lexical_cast<std::string>(kkk)+" different g-vectors\n");
	//show g-vectors on average image
	AddYellowArrow(&Avg, pX, pY, (pX + g1X), (pY + g1Y));
	AddYellowArrow(&Avg, pX, pY, (pX + g2X), (pY + g2Y));
	//End of GetG_Vectors
}

void Process::DoProcess(CProgressCtrl &progress_ctrl)
{
	long f = 0;
	char date[11], time[6];
	std::string datestring, timestring;
	DigitalMicrograph::GetDate(f, date, 11);
	DigitalMicrograph::GetTime(f, time, 6);
	int i;
	for (i = 0; i<10; i++)
	{
		datestring += date[i];
	}
	for (i = 0; i < 5; i++)
	{
		timestring += time[i];
	}
	std::string datetimestring = datestring + "_" + timestring;

	DigitalMicrograph::Result("\nStarting processing " + datetimestring + "\n");

	Persistent = DigitalMicrograph::GetPersistentTagGroup();
	std::string Tag_path;
	Tag_path = "DigitalDiffraction:Settings:";

	//Get 3D data stack
	DigitalMicrograph::Image CBED_stack;
	try
	{
		CBED_stack = DigitalMicrograph::GetFrontImage();
	}
	catch (...)
	{
		DigitalMicrograph::OkDialog("Failed to get front image, please make sure the CBED stack image is at the front");
		return;
	}
	Gatan::PlugIn::ImageDataLocker cbedlock(CBED_stack);
	float* cbedpix = (float*)cbedlock.get();
		
	long imgX, imgY, nPts;
	try
	{
		DigitalMicrograph::Get3DSize(CBED_stack, &imgX, &imgY, &nPts);
	}
	catch (...)
	{
		DigitalMicrograph::OkDialog("Failed to get cbed stack size");
		return;
	}
	long data_type = DigitalMicrograph::GetDataType(CBED_stack);

	//get image tags
	float Rr;
	DigitalMicrograph::TagGroup CBEDtags;
	try
	{
	CBEDtags = DigitalMicrograph::ImageGetTagGroup(CBED_stack);
	DigitalMicrograph::TagGroupGetTagAsFloat(CBEDtags, "Info:Disc Radius", &Rr);
	}
	catch (...)
	{
		DigitalMicrograph::Result("Failed to get radius value from front image\n");
		return;
	}
	float tInc = tInc_factor*Rr;	

	try// tInc must be the same as tInc from collection
	{
		DigitalMicrograph::TagGroupGetTagAsFloat(CBEDtags, "Info:tInc", &tInc);
		DigitalMicrograph::Result("tInc = " + boost::lexical_cast<std::string>(tInc)+"\n");
	}
	catch (...)
	{
		DigitalMicrograph::Result("Failed to get tInc from image tags, using user input instead\n");
		tInc = tInc_factor*Rr;
	}
	bool print_rr_tinc = true;
	try
	{
		DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (Tag_path + "Print Disc Radius Rr and tInc").c_str(), &print_rr_tinc);
	}
	catch (...)	{	}
	if (print_rr_tinc == true)
	{
		DigitalMicrograph::Result("tInc = " + boost::lexical_cast<std::string>(tInc)+"\n");
		DigitalMicrograph::Result("nPts = " + boost::lexical_cast<std::string>(nPts)+"\n");
	}
	//////////////////////////
	//Make sum of all individual images
	float nTilts;

	try
	{
		nTilts = (sqrt((float)nPts) - 1) / 2;
	}
	catch (...)
	{
		DigitalMicrograph::Result("Failed to get nTilts\n");
	}

	int _i, _j;
	int ii, jj;
	DigitalMicrograph::Image Avg;
	Avg = DigitalMicrograph::RealImage("Average CBED image", 4, imgX, imgY);
	Gatan::PlugIn::ImageDataLocker avglock(Avg);
	float* avgpix = (float*)avglock.get();

	DigitalMicrograph::ImageDocumentShowAtPosition(DigitalMicrograph::ImageGetOrCreateImageDocument(CBED_stack), 15, 30);
	DigitalMicrograph::SetWindowSize(CBED_stack, 800, 800);
	DigitalMicrograph::Result("about to loop through pts...\n");
	//run through CBED stack
	long disX, disY, pt;
	
	if (print_rr_tinc == true)
	{
		DigitalMicrograph::Result("Radius :" + boost::lexical_cast<std::string>(Rr)+"\n");
	}
	DigitalMicrograph::ImageDocument avgdisp;
	avgdisp = DigitalMicrograph::ImageGetOrCreateImageDocument(Avg);
	progress_ctrl.SetRange(0,(short)nPts);// setting up the plugin progress bar

	DigitalMicrograph::ImageDocumentShowAtPosition(avgdisp, 15, 30);
	try
	{
		for (pt = 0; pt < nPts; pt++)
		{
			progress_ctrl.SetPos(pt); // update plugin progress bar
			DigitalMicrograph::OpenAndSetProgressWindow("Making average image of CBED Stack", (boost::lexical_cast<std::string>(ExtraGatan::round(((float)pt / nPts) * 100)) + "%").c_str(), "");
			try
			{
				GetCoordsFromNTilts(nTilts, pt, _i, _j);
				disX = ExtraGatan::round(_i*tInc);
				disY = ExtraGatan::round(_j*tInc);
			}
			catch (...)
			{
				DigitalMicrograph::Result("GetCoords failed\n");
				return;
			}

			if ((disX < 0) && (disY < 0))
			{
				for (ii = 0; ii < (imgX-abs(disX)); ii++)
				{
					for (jj = 0; jj < (imgY-abs(disY)); jj++)
					{
						avgpix[((jj + abs(disY))*imgX) + (ii + abs(disX))] += cbedpix[pt*(imgX*imgY) + (jj)*imgX + (ii)];
					}
				}
			}
			else if ((disX < 0) && (disY >= 0))
			{
				for (ii = 0; ii < (imgX - abs(disX)); ii++)
				{
					for (jj = 0; jj < (imgY - abs(disY)); jj++)
					{
						avgpix[jj*imgX + (ii+abs(disX))] += cbedpix[pt*imgX*imgY + (jj + abs(disY))*imgX + ii];
					}
				}
			}
			else if ((disX >= 0) && (disY < 0))
			{
				for (ii = 0; ii < (imgX - abs(disX)); ii++)
				{
					for (jj = 0; jj < (imgY - abs(disY)); jj++)
					{
						avgpix[((jj + abs(disY))*imgX) + ii] += cbedpix[pt*imgX*imgY + (jj)*imgX + (ii + abs(disX))];
					}
				}
			}
			else if ((disX >= 0) && (disY >= 0))
			{		
				for (ii = 0; ii < (imgX - abs(disX)); ii++)
				{
					for (jj = 0; jj < (imgY - abs(disY)); jj++)
					{
						avgpix[jj*imgX + ii] += cbedpix[pt*imgX*imgY + (jj + abs(disY))*imgX + (ii + abs(disX))];
					}
				}
			}
			DigitalMicrograph::ImageDataChanged(Avg);
		}//end of for pt<nPts
		DigitalMicrograph::OpenAndSetProgressWindow("Making average image of CBED Stack", "100%", "");
	}
	catch (...)
	{
		DigitalMicrograph::OkDialog("Failed to run through stack image and create average image");
		return;
	}
	DigitalMicrograph::ImageDocumentShowAtPosition(DigitalMicrograph::ImageGetOrCreateImageDocument(Avg), 15, 30);

	////////////
	//Find g-vectors
	float g1X, g1Y, g1mag, g2X, g2Y, g2mag, ratio, angle;
	long pX, pY;
	DigitalMicrograph::Result("calling getgvectors...\n");

	DigitalMicrograph::Image Disc = ExtraGatan::MakeDisc(imgX, imgY, Rr);
	DigitalMicrograph::Image AvCC = DigitalMicrograph::CrossCorrelate(Avg, Disc);

	bool display_avg_cc = false;
	try
	{
		DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (Tag_path + "Display Avg Cross Correlation").c_str(), &display_avg_cc);
	}
	catch (...)	{	}
	if (display_avg_cc == true)
	{
		DigitalMicrograph::ImageDocumentShowAtPosition(DigitalMicrograph::ImageGetOrCreateImageDocument(AvCC), 15, 30);
	}
	GetG_Vectors(Avg, Rr, g1X, g1Y, g2X, g2Y, pX, pY);
	//Check for failure to get g-vectors
	if ((g1X*g1X) > ((imgX*imgX) / 4))
	{
		g1X = 0;
	}
	if ((g1Y*g1Y) > ((imgX*imgX) / 4))//should this be imgY??
	{
		g1Y = 0;
	}
		
	g1mag = sqrt((g1X*g1X) + (g1Y*g1Y));
	g2mag = sqrt((g2X*g2X) + (g2Y*g2Y));
	double pi = 3.1415926535897932384626433832795;

	if ((g1X + g1Y + g2X + g2Y) == 0)
	{
		DigitalMicrograph::Result("Cannot find g-vectors!");
		return;
	}
	else
	{//output g-vector statistics
		DigitalMicrograph::Result("g-vector 1: " + boost::lexical_cast<std::string>(g1X)+", " + boost::lexical_cast<std::string>(g1Y)+", magnitude" + boost::lexical_cast<std::string>(g1mag)+"\n");
		DigitalMicrograph::Result("g-vector 2: " + boost::lexical_cast<std::string>(g2X)+", " + boost::lexical_cast<std::string>(g2Y)+", magnitude" + boost::lexical_cast<std::string>(g2mag)+"\n");
		ratio = g1mag / g2mag;
		angle = 180 * acos(((g1X*g2X) + (g1Y*g2Y)) / (g1mag*g2mag)) / pi;
		DigitalMicrograph::Result("ratio= " + boost::lexical_cast<std::string>(ratio)+", angle= " + boost::lexical_cast<std::string>(angle)+"\n");
	}
	//////////////////////////////
	//check g-vectors are acceptable, if not do them manually
	//long t, l, b, r;
	std::string prompt;
	DigitalMicrograph::Image AvgTemp;
	float ppx, ppy;
	ppx = (float)pX;
	ppy = (float)pY;
	int nocount=0; // counts how many times measured g-vectors are 'not good'

	while (!DigitalMicrograph::TwoButtonDialog("Are the measured g-vectors good?", "Yes", "No"))
	{
		if (nocount > 3)
		{
			if (DigitalMicrograph::OkCancelDialog("g-vectors have been attempted 3 or more times, would you like to quit?"))// provides an option to quit if g-vectors are not being found, provides an escape from a potential infinite loop
			{
				DigitalMicrograph::Result("Quit at user request\n");
				return;
			}
		}
		UserG(Avg, Rr, g1X, g1Y, g2X, g2Y, ppx, ppy);
		nocount++;
		g1mag = sqrt((g1X*g1X) + (g1Y*g1Y));
		g2mag = sqrt((g2X*g2X) + (g2Y*g2Y));
		DigitalMicrograph::Result("g-vector 1: " + boost::lexical_cast<std::string>(g1X)+", " + boost::lexical_cast<std::string>(g1Y)+", magnitude" + boost::lexical_cast<std::string>(g1mag)+"\n");
		DigitalMicrograph::Result("g-vector 2: " + boost::lexical_cast<std::string>(g2X)+", " + boost::lexical_cast<std::string>(g2Y)+", magnitude" + boost::lexical_cast<std::string>(g2mag)+"\n");
		ratio = g1mag / g2mag;
		angle = 180 * acos(((g1X*g2X) + (g1Y*g2Y)) / (g1mag*g2mag)) / pi;
		DigitalMicrograph::Result("ratio= " + boost::lexical_cast<std::string>(ratio)+", angle= " + boost::lexical_cast<std::string>(angle)+"\n");
	}
	/////////////////////////////////
	//Create 3D stack for D-LACBED images
	//prompt input for number of orders for 1st g
	float nV1 = 3;
	if (!DigitalMicrograph::GetNumber("Number of spots for 1st g (+/-)?", nV1, &nV1))
	{	return;	}
	float nV2 = 3;
	if (!DigitalMicrograph::GetNumber("Number of spots for 2nd g (+/-)?", nV2, &nV2))
	{	return;	}

	//Diameter of circular selection
	float Wfrac=75.0;
	if (!DigitalMicrograph::GetNumber("Percentage of spot used, 67-100%?", Wfrac, &Wfrac))
	{	return;	}
	
	DigitalMicrograph::Result("Creating stack of D-LACBED images...\n");
	long stackz = (long)((2 * nV1 + 1)*(2 * nV2 + 1));
	DigitalMicrograph::Image DLACBEDimg;
	DLACBEDimg = DigitalMicrograph::RealImage("D_LACBED Stack", 4, imgX, imgY, stackz);
	Gatan::PlugIn::ImageDataLocker DLACBEDlock(DLACBEDimg);
	float* DLACBEDpix = (float*)DLACBEDlock.get();
	for (ii = 0; ii < (imgX*imgY*stackz);ii++)
	{
		DLACBEDpix[ii] = 0;
	}

	bool show_dlacbed=true;
	try
	{
		DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (Tag_path + "Show DLACBED stack").c_str(), &show_dlacbed);
	}
	catch (...)	{	}
	
	if (show_dlacbed == true)
	{
		DigitalMicrograph::ImageDocumentShowAtPosition(DigitalMicrograph::ImageGetOrCreateImageDocument(DLACBEDimg), 15, 465);
		DigitalMicrograph::SetWindowSize(DLACBEDimg, 200, 200);
	}

	//Create scratch image for calculation of average
	DigitalMicrograph::Image ScratImg;
	ScratImg = DigitalMicrograph::RealImage("Average", 4, imgX, imgY);
	Gatan::PlugIn::ImageDataLocker scratlock(ScratImg);
	float* scratpix = (float*)scratlock.get();

	long Rr2 = (float)ExtraGatan::round((Wfrac*Rr) / 100);

	//other images for cut and copy
	DigitalMicrograph::Image TempImg, vTempImg;

	TempImg = DigitalMicrograph::RealImage("Disk", 4, 2 * Rr2, 2 * Rr2);
	Gatan::PlugIn::ImageDataLocker tempimglock(TempImg);
	float* tempimgpix = (float*)tempimglock.get();

	vTempImg = ExtraGatan::MakeDisc(2 * Rr2, 2 * Rr2, Rr2);
	Gatan::PlugIn::ImageDataLocker vtempimglock(vTempImg);
	float* vtempimgpix = (float*)vtempimglock.get();

	//loop over DLACBED image
	long gNo = 0;
	long X, Y, m1, m2;
	bool inside;

	DigitalMicrograph::Result("about to loop\n");
	progress_ctrl.SetRange(0, (short)stackz);//setting up the plugin progress bar
	bool print_x_y = false;
	try
	{
		DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (Tag_path + "Print X,Y in DLACBED Stack"), &print_x_y);
	}
	catch (...)	{	}

	for (m1 = -nV1; m1 < nV1 + 1; m1++)
	{
		for (m2 = -nV2; m2 < nV2 + 1; m2++)
		{
			progress_ctrl.SetPos(gNo);//updating plugin progress bar
			DigitalMicrograph::OpenAndSetProgressWindow("Making DLACBED Stack", (boost::lexical_cast<std::string>(ExtraGatan::round(((float)gNo / (stackz) * 100))) + "%").c_str(), "");
			//loop over CBED stack
			for (pt = 0; pt < nPts; pt++)
			{
				GetCoordsFromNTilts(nTilts, pt, _i, _j);
				//appropriate vector for disk
				X = ExtraGatan::round(pX + ((float)_i)*tInc + (float)(m1*g1X) + (float)(m2*g2X));//pX is the centre beam position
				Y = ExtraGatan::round(pY + ((float)_j)*tInc + (float)(m1*g1Y) + (float)(m2*g2Y));

				if (print_x_y == true)
				{
					DigitalMicrograph::Result("X =" + boost::lexical_cast<std::string>(X)+", Y =" + boost::lexical_cast<std::string>(Y)+" about to decide if inside\n");
				}
				inside = !((X - Rr2<0) || (X + Rr2>imgX) || (Y - Rr2<0) || (Y + Rr2>imgY));				
				if (inside)
				{	
					for (jj = 0; jj < 2 * Rr2; jj++)
					{
						std::copy(cbedpix + pt*imgX*imgY + (jj + Y - Rr2)*imgX + (X - Rr2), cbedpix + pt*imgX*imgY + (jj + Y - Rr2)*imgX + (X + Rr2), tempimgpix +(2*Rr2*jj));
					}		
					//Cropped to be circular
					ExtraGatan::CircularMask2D(&TempImg, Rr2, Rr2, Rr2);							
					//Add it to the LACBED pattern
					for (ii = 0; ii < 2*Rr2; ii++)
					{
						for (jj = 0; jj < 2*Rr2; jj++)
						{
							DLACBEDpix[imgX*imgY*gNo + imgX*(jj+Y-Rr2) + (ii+X-Rr2)] += tempimgpix[jj*(2*Rr2) + ii];	
						}
					}
					DigitalMicrograph::ImageDataChanged(DLACBEDimg);
					//Update mask which keeps count of the number of images in one pixel
					for (ii = 0; ii < 2*Rr2; ii++)
					{
						for (jj = 0; jj < 2*Rr2; jj++)
						{
							scratpix[imgX*(jj+Y-Rr2) + (ii+X-Rr2)] += vtempimgpix[jj*(2*Rr2) + ii];
						}
					}

					for (ii = 0; ii < 2 * Rr2; ii++)
					{
						for (jj = 0; jj < 2 * Rr2; jj++)
						{
							if (vtempimgpix[(2 * Rr2*jj) + ii] > tempimgpix[(2 * Rr2*jj) + ii])
							{
								tempimgpix[(2 * Rr2*jj) + ii] = vtempimgpix[(2 * Rr2*jj) + ii];
							}
						}
					}
				}//end of if(inside)
			}//end of for(pt) loop

			//make pixels with zero values equal 1
			for (ii = 0; ii < imgX*imgY; ii++)
			{
				if (scratpix[ii] == 0)
				{
					scratpix[ii] += 1;
				}
			}
			//divide by mask
			for (ii = 0; ii < (imgX); ii++)
			{
				for (jj = 0; jj < (imgY); jj++)
				{
					DLACBEDpix[gNo*imgX*imgY + imgX*jj + ii] /= scratpix[imgX*jj + ii];
				}
			}				

			for (ii = 0; ii < (imgX*imgY); ii++)
			{
				scratpix[ii] = 0;
			}
			gNo++;
		}//end of for m2	
	}

	float dlacmin, dlacmax;
	dlacmin = ExtraGatan::Min3D(DLACBEDimg);
	dlacmax = ExtraGatan::Max3D(DLACBEDimg);
	DigitalMicrograph::SetLimits(DLACBEDimg, dlacmin, dlacmax);
	//Tidy up
	DigitalMicrograph::DeleteImage(TempImg);
	DigitalMicrograph::DeleteImage(vTempImg);
	DigitalMicrograph::Result("Done\n");
	///////////////////////////////////////////
	//Montage of D-LACBED images
	DigitalMicrograph::Result("Creating Montage of D-LACBED images...\n");

	//each D-LACBED image is (2*0.8*nTilts+2)*Rr wide
	float wid;
	wid = (tInc_factor*nTilts + 1)*Rr;
	//wid = (0.8*nTilts + 1)*Rr;
	//Scaling factors between CBED image and montage image
	//F is the relative size of D-LACBED vs original disc size
	//Uses the largest component of the smallest g-vector

	//DigitalMicrograph::Result("Wid = " + boost::lexical_cast<std::string>(wid)+" \n");

	bool print_f_fimgx=false;
	try
	{
		DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (Tag_path + "Print F and FimgX").c_str(), &print_f_fimgx);
	}
	catch (...)	{	}

	float F1, F2, FimgX;
	float maxofg1, maxofg2;
	maxofg1 = ExtraGatan::maxoftwo(g1X, g1Y);
	maxofg2 = ExtraGatan::maxoftwo(g2X, g2Y);

	F1 = (2 * wid) / (ExtraGatan::maxoftwo(abs(g1X), abs(g1Y)));
	F2 = (2 * wid) / (ExtraGatan::maxoftwo(abs(g2X), abs(g2Y)));
	if (print_f_fimgx == true)
	{
		DigitalMicrograph::Result("F = " + boost::lexical_cast<std::string>(F1)+" \n");
	}
	//FimgX is the size of the montage, uses the largest dimension
	//depending on number of reflections and magnitude // here be potential bug...
	if (ExtraGatan::maxoftwo(abs(g1X), abs(g1Y)) > ExtraGatan::maxoftwo(abs(g2X), abs(g2Y)))
	{
		FimgX = ExtraGatan::maxoftwo((4 * nV1*ExtraGatan::maxoftwo(abs(g1X), abs(g1Y))*F1), (4 * nV2 * 2 * wid));
		DigitalMicrograph::Result("Image Size " + boost::lexical_cast<std::string>(FimgX) + " \n");
	}
	else
	{
		FimgX = ExtraGatan::maxoftwo((4 * nV2*ExtraGatan::maxoftwo(abs(g1X), abs(g1Y))*F1), (4 * nV1 * 2 * wid));
		DigitalMicrograph::Result("Image Size " + boost::lexical_cast<std::string>(FimgX) + " \n");
	}

	if (print_f_fimgx == true)
	{
		DigitalMicrograph::Result("FimgX = " + boost::lexical_cast<std::string>(FimgX)+" \n");
	}
	float FimgY = FimgX;
	//The 000 image will be in the centre
	float Lx, Ly;
	Lx = ExtraGatan::round(FimgX / 2);
	Ly = ExtraGatan::round(FimgY / 2);

	DigitalMicrograph::Image Montage;
	Montage = DigitalMicrograph::RealImage("D-LACBED montage", 4, (long)FimgX, (long)FimgY);
	Gatan::PlugIn::ImageDataLocker montagelock(Montage);
	float* montagepix = (float*)montagelock.get();

	//copy image calibration information to CBED stack
	DigitalMicrograph::ImageCopyCalibrationFrom(Montage,CBED_stack);

	DigitalMicrograph::ImageDocument montagedisp;
	montagedisp = DigitalMicrograph::ImageGetOrCreateImageDocument(Montage);
	montagedisp.ShowAtPosition(440, 30);
	DigitalMicrograph::SetWindowSize(Montage, 0.75*imgX, 0.75*imgY);
	DigitalMicrograph::ImageDataChanged(Montage);

	bool create_ask_cropped=false;
	try
	{
		DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (Tag_path + "Create and ask to view cropped").c_str(), &create_ask_cropped);
	}
	catch (...)	{	}

	DigitalMicrograph::Image croppedDLACBED;
	croppedDLACBED = DigitalMicrograph::RealImage("Cropped DLACBED", 4, 2 * wid, 2 * wid, stackz);
	Gatan::PlugIn::ImageDataLocker cropDLACBEDlock(croppedDLACBED);
	float* croppedpix = (float*)cropDLACBEDlock.get();
	long croppedX, croppedY, croppedZ;
	DigitalMicrograph::Get3DSize(croppedDLACBED, &croppedX, &croppedY, &croppedZ);

	long t2, l2, b2, r2, t1, l1, b1, r1;
	float a2X, a2Y, a1X, a1Y;
	gNo = 0;

	long insidelong;
	insidelong = 2;

	DigitalMicrograph::Result("g1X = " + boost::lexical_cast<std::string>(g1X)+", g2X = " + boost::lexical_cast<std::string>(g2X)+"\n");
	DigitalMicrograph::Result("g1Y = " + boost::lexical_cast<std::string>(g1Y)+", g2Y = " + boost::lexical_cast<std::string>(g2Y)+"\n");
	int notinsidecount=0; // counts whether any of the DLACBED images are not included on the montage

	for (m1 = -nV1; m1 < nV1 + 1; m1++)
	{
		for (m2 = -nV2; m2 < nV2 + 1; m2++)
		{
			//a2 is the centre of the rectangle where the D_LACBED images comes from in the stack
			a2X = ExtraGatan::round((float)pX + (float)m1*g1X + (float)m2*g2X);
			a2Y = ExtraGatan::round((float)pY + (float)m1*g1Y + (float)m2*g2Y);
			//Bounding rectangle for each D-LACBED image
			t2 = (long)ExtraGatan::round((a2Y - wid)*(1 - ((a2Y - wid) < 0 ? 1 : 0)));
			l2 = (long)ExtraGatan::round((a2X - wid)*(1 - ((a2X - wid) < 0 ? 1 : 0)));
			b2 = (long)ExtraGatan::round((a2Y + wid)*(1 - ((a2Y + wid) > imgY ? 1 : 0)) + ((a2Y + wid) > imgY ? 1 : 0)*imgY);
			r2 = (long)ExtraGatan::round((a2X + wid)*(1 - ((a2X + wid) > imgX ? 1 : 0)) + ((a2X + wid) > imgX ? 1 : 0)*imgX);
			//a1 is the location of the rectangle where the D-LACBED image will go in the montage
			a1X = ExtraGatan::round(Lx + (m1*g1X*F1 + m2*g2X*F2));
			a1Y = ExtraGatan::round(Ly + (m1*g1Y*F1 + m2*g2Y*F2));

			t1 = (long)ExtraGatan::round(a1Y - a2Y + t2);
			l1 = (long)ExtraGatan::round(a1X - a2X + l2);
			b1 = (long)ExtraGatan::round(a1Y + b2 - a2Y);
			r1 = (long)ExtraGatan::round(a1X + r2 - a2X);
	
			inside = !((l1<0) || (r1>FimgX) || (t1<0) || (b1>FimgY));
			if (inside)
			{
				for (ii = 0; ii < abs(r1-l1); ii++)
				{
					for (jj = 0; jj < abs(b1 - t1); jj++)
					{
						if (DLACBEDpix[(imgX*imgY*gNo) + (jj + t2)*imgX + (ii + l2)] == 0)
						{
							montagepix[(t1 + jj)*(long)FimgX + (l1 + ii)] = montagepix[(t1 + jj)*(long)FimgX + (l1 + ii)];
						}
						else
						{
							montagepix[(t1 + jj)*(long)FimgX + (l1 + ii)] = DLACBEDpix[(imgX*imgY*gNo) + (jj + t2)*imgX + (ii + l2)];
						}
						if (create_ask_cropped == true)// make a cropped version if setting was selected
						{
							croppedpix[croppedX*croppedY*gNo + jj * croppedX + ii] = DLACBEDpix[(imgX*imgY*gNo) + (jj + t2)*imgX + (ii + l2)];
						}
					}
				}
				DigitalMicrograph::ImageDataChanged(Montage);//end of if(inside)
			}
			else
			{
				notinsidecount++;
			}
			gNo++;
		}
	}
	if (notinsidecount > 0)
	{
		DigitalMicrograph::OkDialog("One or more of the LACBED images were not included in the montage\nYou may wish to retry processing and ensure the correct g-vectors are found");
	}

	if (create_ask_cropped == true)
	{
		if (DigitalMicrograph::OkCancelDialog("Would you like a cropped version of the DLACBED stack?"))
		{
			croppedDLACBED.GetOrCreateImageDocument().ShowAtPosition(15, 30);
		}
		else
		{
			DigitalMicrograph::DeleteImage(croppedDLACBED);
		}
	}
	float montmin, montmax;
	montmin = ExtraGatan::Min(Montage);
	montmax = ExtraGatan::Max(Montage);
	DigitalMicrograph::SetLimits(Montage, montmin, montmax);
	if (show_dlacbed == false)
	{
		DigitalMicrograph::DeleteImage(DLACBEDimg);
	}
	std::string sectimestring;
	DigitalMicrograph::GetDate(f, date, 11);
	DigitalMicrograph::GetTime(f, time, 6);
	for (i = 0; i < 5; i++)
	{
		sectimestring += time[i];
	}
	datetimestring = datestring + "_" + sectimestring;
	DigitalMicrograph::Result("Processing complete: " + datetimestring + " ding, dong\n\n");
}