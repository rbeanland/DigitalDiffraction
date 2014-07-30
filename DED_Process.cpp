#include "stdafx.h"
#include "DED_Process.h"

void Process::ROIpos(DigitalMicrograph::Image Img,std::string prompt,double &t, double &l, double &b, double &r)
{
	long imgX,imgY;
	bool ClickedOK, onroi;
	DigitalMicrograph::Get2DSize(Img,&imgX,&imgY);
	DigitalMicrograph::ImageDisplay img_disp;
	img_disp = DigitalMicrograph::ImageGetImageDisplay(Img,0);
	DigitalMicrograph::ROI theROI=DigitalMicrograph::NewROI();
	DigitalMicrograph::ROISetRectangle(theROI,t,l,b,r);
	DigitalMicrograph::ImageDisplayAddROI(img_disp,theROI);
	onroi = false;
	ClickedOK=true;
	
	/*while (onroi == false)
	{
		try
		{
			ClickedOK = DigitalMicrograph::ContinueCancelDialog(prompt.c_str());
			if (ClickedOK)
			{
				onroi = true;
			}
		}
		catch (...)
		{
			DigitalMicrograph::OkDialog("Error occurred in ROIpos function");
			return;
		}
	}*/

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
		return;
	}


	DigitalMicrograph::ImageDisplayDeleteROI(img_disp,theROI);
	
	float _t,_l,_b,_r;
	_t=(float)t;
	_l=(float)l;
	_b=(float)b;
	_r=(float)r;

	if(ClickedOK)
	{
		DigitalMicrograph::ROIGetRectangle(theROI,&_t,&_l,&_b,&_r);
	}
	//End of function ROIpos
}

void Process::GetCoordsFromNTilts(long nTilts,long currentPoint, int &i, int &j)
{
	int side = 2*nTilts+1;
	j = floor(((double)currentPoint)/((double)side))-nTilts;
	i = ((currentPoint % side) - nTilts)*pow((float)(-1),(int)(j % 2));//NB % means modulo, flips sign every row
}

void Process::AddBlueCircle(DigitalMicrograph::Image *img,float t, float l, float b, float r)
{
	DigitalMicrograph::ImageDisplay imgdisp=DigitalMicrograph::ImageGetImageDisplay(*img,0);
	DigitalMicrograph::Component ov;
	ov=DigitalMicrograph::NewOvalAnnotation(t,l,b,r);
	DigitalMicrograph::ComponentAddChildAtEnd(imgdisp, ov);	
	//End of AddBlueCircle
}

void Process::AddYellowArrow(DigitalMicrograph::Image *img, float x0, float y0, float x1, float y1)
{
	DigitalMicrograph::ImageDisplay imgdisp = DigitalMicrograph::ImageGetImageDisplay(*img, 0);
	DigitalMicrograph::Component arrowannot;
	arrowannot = DigitalMicrograph::NewArrowAnnotation(y0, x0, y1, x1);
	DigitalMicrograph::ComponentAddChildAtEnd(imgdisp, arrowannot);
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
	Avg.GetOrCreateImageDocument().ShowAtPosition(15, 30);
	DigitalMicrograph::SetWindowSize(Avg, 1000, 1000);
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
	{
		return;
	}
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
	ROIpos(Avg, prompt, t, l, b, r);

	order = 2;
	if (!DigitalMicrograph::GetNumber("Diffraction order?", order, &order))
	{
		return;
	}
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

void Process::GetMeanG(DigitalMicrograph::Image MagTheta,DigitalMicrograph::Image &Cluster ,float* Clusterpix,double &MeanMag,double &MeanTheta, double &MeanVx, double &MeanVy, int &nMeas)
{
	//&cluster will need to be img locker

	//incoming Mean values are a single vector
	//outgoing mean values are an average of the vectors deemed to be in the cluster
	//incoming mag/theta is the same for +/- vectors [since tan(q+pi)=tan(q)]
	//so flip the vectors if the x-component has an opposite sign

	double tolMag = 5;//tolerance in pixels to say it's in a cluster
	double tolAng = 5;//tolerance in degrees to say it's in a cluster
	long n, nVecs;
	int i;
	double dTheta, dMag, ThetaSum, VxSum, VySum, x, y, signX;

	DigitalMicrograph::Get2DSize(Cluster, &n, &nVecs);
	nMeas = 0;
	double MagSum = 0;

	for (i = 0; i < nVecs; i++)
	{
		dMag = abs(MeanMag - DigitalMicrograph::GetPixel(MagTheta, 0, i));
		dTheta = abs(MeanTheta - DigitalMicrograph::GetPixel(MagTheta, 1, i));
		if ((dTheta < tolAng) && (dMag < tolMag))
		{
			nMeas++;
			MagSum += DigitalMicrograph::GetPixel(MagTheta, 0, i);
			ThetaSum += DigitalMicrograph::GetPixel(MagTheta, 1, i);

			x = DigitalMicrograph::GetPixel(MagTheta, 2, i);
			y = DigitalMicrograph::GetPixel(MagTheta, 3, i);
			signX = abs(x) / x;

			VxSum += x*(abs(MeanVx) / MeanVx)*signX;//second part here reverses
			VySum += y*(abs(MeanVx) / MeanVx)*signX;//sign if x is opposite sign


			Clusterpix[i] = 1;


			//int ii,jj;
			//for (ii = i; ii <= i + 1; ii++)
			//{
			//	for (jj = 0; jj <= 1; jj++)
			//	{
			//		Clusterpix[n*jj+ii] = 1;//it is in the cluster
			//	}
			//}

			//Cluster[i, 0, i + 1, 1] = 1;//it is in the cluster

			MeanMag = MagSum / (double)nMeas;
			MeanTheta = ThetaSum / (double)nMeas;
			MeanVx = VxSum / (double)nMeas;
			MeanVy = VySum / (double)nMeas;

		} 

	}
	DigitalMicrograph::Result("If error here, it's in GetMeanG\n");
	//End of GetMeanG
}

void Process::GetG_Vectors(DigitalMicrograph::Image Avg, float Rr, float &g1X, float &g1Y, float &g2X, float &g2Y, long &pX, long &pY)
{
	long nPeaks = 25;
	long imgX, imgY;
	DigitalMicrograph::Get2DSize(Avg, &imgX, &imgY);

	//start by getting a cross-correlation image
	//image of a blank disk, radius Rr

	DigitalMicrograph::Image Disc = ExtraGatan::MakeDisc(imgX, imgY, Rr);

	DigitalMicrograph::Image AvCC = DigitalMicrograph::CrossCorrelate(Avg, Disc);

	//// adding here so that ln 256 can be done
	long avccX, avccY;
	DigitalMicrograph::Get2DSize(AvCC, &avccX, &avccY);
	///////////////////////////////////////////

	double maxval;
	long xp, yp;
	maxval = ExtraGatan::Max(AvCC);
	ExtraGatan::PixelPos(AvCC, maxval, &xp, &yp, false);
	pX = xp;
	pY = yp;

	float dSize = 3.5;//a multiplying factor for the disc size when deleting peaks which have already been maeasured. Ideally dSize*Rr should be half of the smallest g-vector.
	float DdSize = ExtraGatan::round(dSize*Rr);
	
	DigitalMicrograph::DeleteImage(Disc);//tidy up
	//delete all info below 2% of best correlation
	float _top = maxval*0.02;
	AvCC = ExtraGatan::tert(AvCC, 0.001);//need to check

	DigitalMicrograph::Image X, Y, TempImg;
	//x- and y-coords as column vectors
	X = DigitalMicrograph::RealImage("X-coords", 4, 1, nPeaks);
	Y = DigitalMicrograph::RealImage("Y-coords", 4, 1, nPeaks);
	TempImg = ExtraGatan::MakeDisc(2 * DdSize, 2 * DdSize, dSize*Rr);//A dark circle


	Gatan::PlugIn::ImageDataLocker avcclock(AvCC);
	float* avccpix = (float*)avcclock.get();

	Gatan::PlugIn::ImageDataLocker templock(TempImg);
	float* temppix = (float*)templock.get();


	int i,k;
	int Nmax = nPeaks;
	bool flag = false;
	
	for (i = 0; i < nPeaks; i++)
	{//get peak position, in descending order of correlation/intensity
		if (maxval>_top)
		{//only keep going if there are peaks to be found
			maxval = ExtraGatan::Max(AvCC);
			ExtraGatan::PixelPos(AvCC, maxval, &xp, &yp, false);
			DigitalMicrograph::SetPixel(X, 0, i, xp);//maybe X Y can be arrays?????
			DigitalMicrograph::SetPixel(Y, 0, i, yp);
			AddBlueCircle(&Avg, yp - Rr, xp - Rr, yp + Rr, xp + Rr);
			
			DigitalMicrograph::SetSelection(AvCC, (yp - DdSize), (xp - DdSize), (yp + DdSize), (xp + DdSize));
			for (k = 0; k < avccX*avccY; k++)
			{
				avccpix[k] *= temppix[i];//this peak is done, delete it
			}

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

	}

	nPeaks = Nmax;
	if (nPeaks < 3)
	{
		return;
	}
	//Find difference vectors Vx, Vy, by replicationg X and Y into square matrices Xx, Yy, and subtracting the transpose

	DigitalMicrograph::Image Xx, Xxtrans, Vx, Yy, Yytrans, Vy;

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

	//Polar coordinates, Vmag and Vtheta
	DigitalMicrograph::Image Vmag, Vtheta;
	Vmag = DigitalMicrograph::RealImage("Vmag", 4, nPeaks, nPeaks);
	Gatan::PlugIn::ImageDataLocker VmagLock(Vmag);
	float* vmagpix = (float*)VmagLock.get();

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

	//for (ii = 0; ii < nPeaks; ii++)
	//{
	//	for (jj = 0; jj < nPeaks; jj++)
	//	{
	//		if (vxpix[nPeaks*jj + ii] == 0)//get rid of divide by zero error
	//		{
	//			vxpix[nPeaks*jj + ii] = 1000000; //moved to the vxx matrix construction
	//		}
	//	}
	//}

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

	//sort magnitude ascending into new column vector MagTheta

	long nVecs = (nPeaks*nPeaks - nPeaks) / 2;//number of different vectors


	DigitalMicrograph::Image MagTheta;
	MagTheta = DigitalMicrograph::RealImage("Mag-Theta-X-Y", 4, 4, nVecs);
	Gatan::PlugIn::ImageDataLocker magthetaLock(MagTheta);
	float* magthetapix = (float*)magthetaLock.get();


	//may need to check min function
	float Mag = ExtraGatan::Min(Vmag);//lowest magnitude in the list
	ExtraGatan::PixelPos(Vmag, Mag, &xp, &yp, false);

	int iii=0;
	while (Mag < complete)
	{
		magthetapix[iii*yp] = vmagpix[nPeaks*yp + xp];//first col=magnitude
		magthetapix[iii*yp + 1] = vthetapix[nPeaks*yp + xp];//second column=theta
		magthetapix[iii*yp + 2] = vxpix[nPeaks*yp + xp];//third column=Vx
		magthetapix[iii*yp + 3] = vypix[nPeaks*yp + xp];//fourth column=Vy
		vmagpix[nPeaks*yp + xp] = complete;//this point is done, eliminate from Vmag
		iii = iii + 1;
		Mag = ExtraGatan::Min(Vmag);
		ExtraGatan::PixelPos(Vmag, Mag, &xp, &yp, false);
	}

	//Find clusters - similar g-vectors in mag-theta space
	DigitalMicrograph::Image Cluster, gVectors;
	Cluster = DigitalMicrograph::RealImage("Cluster", 4, 1, nVecs);
	Gatan::PlugIn::ImageDataLocker clusterlock(MagTheta);
	float* clusterpix = (float*)clusterlock.get();
	for (iii = 0; iii < nVecs; iii++)
	{
		clusterpix[iii] = 0;
	}


	gVectors = DigitalMicrograph::RealImage("g's", 4, 5, nVecs);
	Gatan::PlugIn::ImageDataLocker gveclock(MagTheta);
	float* gvecpix = (float*)gveclock.get();


	double MeanMag, MeanTheta, MeanVx, MeanVy; // may need to change to float???
	MeanMag = DigitalMicrograph::GetPixel(MagTheta, 0, 0);
	MeanTheta = DigitalMicrograph::GetPixel(MagTheta, 1, 0);
	MeanVx = DigitalMicrograph::GetPixel(MagTheta, 2, 0);
	MeanVy = DigitalMicrograph::GetPixel(MagTheta, 3, 0);

	int jjj = 1, kkk = 0;
	int nMeas = 1;//number of measured points to give an average g-vector

	//Go through and get clusters
	while (ExtraGatan::Sum(clusterpix, nVecs) < nVecs)
	{
		GetMeanG(MagTheta, Cluster, clusterpix, MeanMag, MeanTheta, MeanVx, MeanVy, nMeas);
		gvecpix[kkk * 5] = MeanMag;
		gvecpix[kkk * 5 + 1] = MeanTheta;
		gvecpix[kkk * 5 + 2] = MeanVx;
		gvecpix[kkk * 5 + 3] = MeanVy;
		gvecpix[kkk * 5 + 4] = nMeas;
		//Find next unmatched point
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
	
	DigitalMicrograph::Result("Found " + boost::lexical_cast<std::string>(nPeaks)+" different CBED disks, \n");
	DigitalMicrograph::Result("giving " + boost::lexical_cast<std::string>(kkk)+" different g-vectors\n");
	//show g-vectors on average image
	AddYellowArrow(&Avg, pX, pY, (pX + g1X), (pY + g1Y));
	AddYellowArrow(&Avg, pX, pY, (pX + g2X), (pY + g2Y));

	//End of GetG_Vectors

}

void Process::DoProcess()
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

	//Get 3D data stack
	DigitalMicrograph::Image CBED_stack;
	CBED_stack = DigitalMicrograph::GetFrontImage();

	Gatan::PlugIn::ImageDataLocker cbedlock(CBED_stack);
	float* cbedpix = (float*)cbedlock.get();

	
	long imgX, imgY, nPts;

	DigitalMicrograph::Get3DSize(CBED_stack, &imgX, &imgY, &nPts);
	long data_type = DigitalMicrograph::GetDataType(CBED_stack);

	//get image tags

	DigitalMicrograph::TagGroup CBEDtags;
	CBEDtags = DigitalMicrograph::ImageGetTagGroup(CBED_stack);


	float Rr;

	DigitalMicrograph::TagGroupGetTagAsFloat(CBEDtags, "Info:Radius", &Rr);

	float tInc = Rr*0.8; //Must match tInc in collect!

	//////////////////////////
	//Make sum of all individual images
	float nTilts = (sqrt((float)nPts) - 1) / 2;
	int _i, _j;
	int ii, jj;
	DigitalMicrograph::Image Avg;
	Avg = DigitalMicrograph::RealImage("Average CBED image", 4, imgX, imgY);

	Gatan::PlugIn::ImageDataLocker avglock(Avg);
	float* avgpix = (float*)avglock.get();

	DigitalMicrograph::ImageDocumentShowAtPosition(DigitalMicrograph::ImageGetOrCreateImageDocument(CBED_stack), 15, 30);
	DigitalMicrograph::SetWindowSize(CBED_stack, 800, 800);

	//run through CBED stack
	long disX, disY, pt;
	for (pt = 0; pt < nPts; pt++)
	{
		GetCoordsFromNTilts(nTilts, pt, _i, _j);
		disX = ExtraGatan::round(_i*tInc);
		disY = ExtraGatan::round(_j*tInc);
		
		if (disX < 0 && disY < 0)
		{
			for (ii = disX; ii < imgX; ii++)
			{
				for (jj = disY; jj < imgY; jj++)
				{
					avgpix[jj*imgX + ii] = cbedpix[pt*(imgX*imgY) + (jj - disY)*imgX + (ii - disX)];
				}
			}
		}
		else if (disX < 0 && disY >= 0)
		{
			for (ii = disX; ii < imgX; ii++)
			{
				for (jj = 0; jj < (imgY - disY); jj++)
				{
					avgpix[jj*imgX + ii] = cbedpix[pt*imgX*imgY + (jj + disY)*imgX + ii];
				}
			}
		}
		else if (disX >= 0 && disY < 0)
		{
			for (ii = 0; ii < (imgX - disX); ii++)
			{
				for (jj = disY; jj < imgY; jj++)
				{
					avgpix[jj*imgX + ii] = cbedpix[pt*imgX*imgY + (jj - disY)*imgX + (ii + disX)];
				}
			}
		}
		else if (disX >= 0 && disY >= 0)
		{
			for (ii = 0; ii < (imgX - disX); ii++)
			{
				for (jj = 0; jj < (imgY - disY); jj++)
				{
					avgpix[jj*imgX + ii] = cbedpix[pt*imgX*imgY + (jj + disY)*imgX + (ii + disX)];
				}
			}
		}

		DigitalMicrograph::UpdateImage(Avg);
		
	}//end of for pt<nPts

	////////////
	//Find g-vectors
	float g1X, g1Y, g1mag, g2X, g2Y, g2mag, ratio, angle;
	long pX, pY;

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
	long t, l, b, r;
	std::string prompt;
	DigitalMicrograph::Image AvgTemp;
	float ppx, ppy;
	ppx = (float)pX;
	ppy = (float)pY;
	while (!DigitalMicrograph::TwoButtonDialog("Are the measured g-vectors good?", "Yes", "No"))
	{
		UserG(Avg, Rr, g1X, g1Y, g2X, g2Y, ppx, ppy);
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
	{
		return;
	}
	float nV2 = 3;
	if (!DigitalMicrograph::GetNumber("Number of spots for 2nd g (+/-)?", nV2, &nV2))
	{
		return;
	}

	//Diameter of circular selection
	float Wfrac;
	if (!DigitalMicrograph::GetNumber("Percentage of spot used, 67-100%?", Wfrac, &Wfrac))
	{
		return;
	}

	DigitalMicrograph::Result("Creating stack of D-LACBED images...\n");

	DigitalMicrograph::Image DLACBEDimg;
	DLACBEDimg = DigitalMicrograph::RealImage("D_LACBED Stack", 4, imgX, imgY, ((2 * nV1 + 1)*(2 * nV2 + 1)));
	Gatan::PlugIn::ImageDataLocker DLACBEDlock(DLACBEDimg);
	float* DLACBEDpix = (float*)DLACBEDlock.get();
	long stackz = ((2 * nV1 + 1)*(2 * nV2 + 1));
	for (ii = 0; ii < (imgX*imgY*stackz);ii++)
	{
		DLACBEDpix[ii] = 0;
	}

	DigitalMicrograph::ImageDocumentShowAtPosition(DigitalMicrograph::ImageGetOrCreateImageDocument(DLACBEDimg), 15, 465);
	DigitalMicrograph::SetWindowSize(DLACBEDimg, 200, 200);

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


	
	//vTempImg = DigitalMicrograph::RealImage("Temp", 4, 2 * Rr2, 2 * Rr2);

	vTempImg = ExtraGatan::MakeDisc(2 * Rr2, 2 * Rr2, Rr2);
	Gatan::PlugIn::ImageDataLocker vtempimglock(vTempImg);
	float* vtempimgpix = (float*)vtempimglock.get();


	//loop over DLACBED image
	long gNo = 0;
	long X, Y, m1, m2;
	bool inside;

	for (m1 = -nV1; m1 < nV1 + 1; m1++)
	{
		for (m2 = -nV2; m2 < nV2 + 1; m2++)
		{
			//loop over CBED stack
			for (pt = 0; pt < nPts; pt++)
			{
				GetCoordsFromNTilts(nTilts, pt, _i, _j);
				//appropriate vector for disk

				X = ExtraGatan::round(pX + _i*tInc + (m1*g1X) + (m2*g2X));
				Y = ExtraGatan::round(pY + _j*tInc + (m1*g1Y) + (m2*g2Y));
				inside = !((X - Rr2<0) || (X + Rr2>imgX) || (Y - Rr2<0) || (Y + Rr2>imgY));
				if (inside)
				{
					for (ii = 0; ii < 2 * Rr2; ii++)
					{
						for (jj = 0; jj < 2 * Rr2; jj++)
						{
							tempimgpix[jj*Rr2 + ii] = cbedpix[pt*imgX*imgY + (jj + Y - Rr2)*imgX + (ii + X - Rr2)];
						}
					}
					//Cropped to be circular
					ExtraGatan::CircularMask2D(&TempImg, Rr2, Rr2, Rr2); //may need to adjust the y-coordinate in this function

					//Add it to the LACBED pattern
					for (ii = (X - Rr2); ii < (X + Rr2); ii++)
					{
						for (jj = (Y - Rr2); jj < (Y + Rr2); jj++)
						{
							DLACBEDpix[imgX*imgY*gNo + imgX*jj + ii] += tempimgpix[(jj-(Y-Rr2))*imgX + (ii-(X-Rr2))];//may be wrong index numbers, also +=
						}
					}

					//Update mask which keeps count of the number of images in one pixel

					for (ii = (X - Rr2); ii < (X + Rr2); ii++)
					{
						for (jj = (Y - Rr2); jj < (Y + Rr2); jj++)
						{
							scratpix[imgX*jj + ii] += vtempimgpix[(jj - (Y - Rr2))*imgX + (ii - (X - Rr2))];//may be wrong index numbers, also +=
						}
					}

					////////////////////

					for (ii = 0; ii < 2 * Rr2; ii++)
					{
						for (jj = 0; jj < 2 * Rr2; jj++)
						{
							if (vtempimgpix[(2 * Rr2*jj) + ii] > tempimgpix[(2 * Rr2*jj) + ii])
							{
								tempimgpix[(2 * Rr2*jj) + ii] = vtempimgpix[(2 * Rr2*jj) + ii];
							}
						}
					}//this should probably be an external tert function?

				}//end of if(inside)

			}//end of for(pt) loop

			//make pixels with zero values equal 1
			for (ii = 0; ii < imgX; ii++)
			{
				for (jj = 0; jj < imgY * Rr2; jj++)
				{
					if (scratpix[imgX*jj + ii] == 0)
					{
						scratpix[imgX*jj + ii] += 1;
					}
				}
			}

			//divide by mask
			for (ii = 0; ii < (imgX*imgY); ii++)
			{
				DLACBEDpix[ii] /= scratpix[ii];
			}

			for (ii = 0; ii < (imgX*imgY); ii++)
			{
				scratpix[ii] = 0;
			}

			gNo++;

		}
	}

	float dlacmin, dlacmax;
	dlacmin = ExtraGatan::Min(DLACBEDimg);
	dlacmax = ExtraGatan::Max(DLACBEDimg);
	DigitalMicrograph::SetLimits(DLACBEDimg, dlacmin, dlacmax);

	//Tidy up
	DigitalMicrograph::DeleteImage(TempImg);
	DigitalMicrograph::DeleteImage(vTempImg);
	DigitalMicrograph::Result(" Done\n");

	///////////////////////////////////////////
	//Montage of D-LACBED images
	DigitalMicrograph::Result("Creating Montage of D-LACBED images...\n");

	//each D-LACBED image is (2*nTilts+3)*Rr wide

	float wid;
	wid = (nTilts + 1.5)*Rr;
	//Scaling factors between CBED image and montage image
	//F is the relative size of D-LACBED vs original disc size
	//Uses the largest component of the smallest g-vector

	float F, FimgX;//may need to change types
	if (g1mag < g2mag)
	{
		F = (2 * wid) / (ExtraGatan::maxoftwo(g1X, g1Y));
	}
	else
	{
		F = (2 * wid) / (ExtraGatan::maxoftwo(g2X, g2Y));
	}

	//FimgX is the size of the montage, uses the largest dimension
	//depending on number of reflections and magnitude
	if (ExtraGatan::maxoftwo(g1X, g1Y) > ExtraGatan::maxoftwo(g2X, g2Y))
	{
		FimgX = ExtraGatan::maxoftwo((2 * nV1*ExtraGatan::maxoftwo(g1X, g1Y)*F + 2 * wid), (2 * nV2 * 2 * wid + 2 * wid));
	}
	else
	{
		FimgX = ExtraGatan::maxoftwo((2 * nV2*ExtraGatan::maxoftwo(g1X, g1Y)*F + 2 * wid), (2 * nV1 * 2 * wid + 2 * wid));
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


	Montage.GetOrCreateImageDocument().ShowAtPosition(440, 30);
	DigitalMicrograph::SetWindowSize(Montage, 0.75*imgX, 0.75*imgY);
	DigitalMicrograph::UpdateImage(Montage);

	long t2, l2, b2, r2, t1, l1, b1, r1;
	float a2X, a2Y, a1X, a1Y;
	gNo = 0;

	for (m1 = -nV1; m1 < nV1 + 1; m1++)
	{
		for (m2 = -nV2; m2 < nV2 + 1; m2++)
		{
			//a2 is the centre of the rectangle where the D_LACBED images comes from in the stack
			a2X = ExtraGatan::round(pX + m1*g1X + m2*g2X);
			a2Y = ExtraGatan::round(pY + m1*g1Y + m2*g2Y);
			
			//Bounding rectangle for each D-LACBED image
			t2 = (long)ExtraGatan::round((a2Y - wid)*(1 - ((a2Y - wid) < 0 ? 1 : 0)));
			l2 = (long)ExtraGatan::round((a2X - wid)*(1 - ((a2X - wid) < 0 ? 1 : 0)));
			b2 = (long)ExtraGatan::round((a2Y + wid)*(1 - ((a2Y + wid) > imgY ? 1 : 0)) + ((a2Y + wid) > imgY ? 1 : 0)*imgY);
			b2 = (long)ExtraGatan::round((a2X + wid)*(1 - ((a2X + wid) > imgX ? 1 : 0)) + ((a2X + wid) > imgX ? 1 : 0)*imgX);
			
			//a1 is the location of the rectangle where the D-LACBED image will go in the montage
			a1X = ExtraGatan::round(Lx + (m1*g1X + m2*g2X)*F);
			a1Y = ExtraGatan::round(Ly + (m1*g1Y + m2*g2Y)*F);

			t1 = (long)ExtraGatan::round(a1Y - a2Y + t2);
			l1 = (long)ExtraGatan::round(a1X - a2X + l2);
			b1 = (long)ExtraGatan::round(a1Y + b2 - a2Y);
			r1 = (long)ExtraGatan::round(a1X + r2 - a2X);

			inside = !((l1<0) || (r1>FimgX) || (t1<0) || (b1>FimgY));
			if (inside);
			{
				for (ii = 0; ii < (r1-l1); ii++)
				{
					for (jj = 0; jj < (b1-t1); jj++)
					{
						if (DLACBEDpix[(imgX*imgY*gNo) + (jj + t2)*imgX + (ii + l2)] == 0)
						{
							montagepix[(t1 + jj)*(long)FimgX + (l1 + ii)] = montagepix[(t1 + jj)*(long)FimgX + (l1 + ii)];
						}
						else
						{
							montagepix[(t1 + jj)*(long)FimgX + (l1 + ii)] = DLACBEDpix[(imgX*imgY*gNo) + (jj + t2)*imgX + (ii + l2)];
						}
					}
				}

			}//end of if(inside)

			gNo++;

			
		}
	}

	float montmin, montmax;
	montmin = ExtraGatan::Min(Montage);
	montmax = ExtraGatan::Max(Montage);
	DigitalMicrograph::SetLimits(Montage, montmin, montmax);

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

	DigitalMicrograph::Result("Processing complete: " + datetimestring + " ding, dong\n\n");

}