#include "stdafx.h"
#include "DED_Process.h"

void Process::ROIpos(DigitalMicrograph::Image Img,std::string prompt,double &t, double &l, double &b, double &r)
{
	long imgX,imgY;
	bool ClickedOK/*, onroi*/;
	DigitalMicrograph::Get2DSize(Img,&imgX,&imgY);
	DigitalMicrograph::ImageDisplay img_disp;
	img_disp = DigitalMicrograph::ImageGetImageDisplay(Img,0);
	DigitalMicrograph::ROI theROI=DigitalMicrograph::NewROI();
	DigitalMicrograph::ROISetRectangle(theROI,t,l,b,r);
	DigitalMicrograph::ImageDisplayAddROI(img_disp,theROI);
	//onroi = false;
	ClickedOK=true;
	

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
		DigitalMicrograph::ROIGetRectangle(theROI,&_t,&_l,&_b,&_r);
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

void Process::AddBlueCircle(DigitalMicrograph::Image *img,float t, float l, float b, float r)
{
	DigitalMicrograph::ImageDisplay imgdisp=DigitalMicrograph::ImageGetImageDisplay(*img,0);
	DigitalMicrograph::Component ov;
	ov=DigitalMicrograph::NewOvalAnnotation(t,l,b,r);
	DigitalMicrograph::ComponentAddChildAtEnd(imgdisp, ov);	

	ExtraGatan::setCompCol(ov, 0, 0, 1);
	//End of AddBlueCircle
}

void Process::AddYellowArrow(DigitalMicrograph::Image *img, float x0, float y0, float x1, float y1)
{
	DigitalMicrograph::ImageDisplay imgdisp = DigitalMicrograph::ImageGetImageDisplay(*img, 0);
	DigitalMicrograph::Component arrowannot;
	arrowannot = DigitalMicrograph::NewArrowAnnotation(y0, x0, y1, x1);
	DigitalMicrograph::ComponentAddChildAtEnd(imgdisp, arrowannot);

	ExtraGatan::setCompCol(arrowannot, 1, 1, 0);
	//end of AddYellowArrow
}

void Process::UserG(DigitalMicrograph::Image Avg, float Rr, float &g1X, float &g1Y, float &g2X, float &g2Y, float &pX, float &pY)
{
	double t, l, b, r;
	long imgX, imgY;
	std::string prompt;
	DigitalMicrograph::Image AvgTemp;
	roicontinue = true;

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

	double t1 = t;

	ROIpos(Avg, prompt, t, l, b, r);
	DigitalMicrograph::Result("Done\n");

	//if (t == t1)
	//{

	//	if (!DigitalMicrograph::OkCancelDialog("central ROI has not been changed, continue?"))
	//	{
	//		DigitalMicrograph::Result("Cancelled user roi\n");
	//		return;
	//	}
	//	
	//}

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
	//DigitalMicrograph::Result("n = " + boost::lexical_cast<std::string>(n)+", nVecs = " + boost::lexical_cast<std::string>(nVecs)+"\n");
	nMeas = 0;
	double MagSum = 0;

	double currmagthetapix;
	double col2currentmagthetapix;

	for (i = 0; i < nVecs; i++)
	{
		currmagthetapix = DigitalMicrograph::GetPixel(*MagTheta, 0, i);
//		dMag = abs(MeanMag - DigitalMicrograph::GetPixel(MagTheta, 0, i));
		dMag = abs(MeanMag - currmagthetapix);
		//if (i == 0 || i == 1)
		//{
		//	DigitalMicrograph::Result("dmag = " + boost::lexical_cast<std::string>(dMag)+"\n");
		//	DigitalMicrograph::Result("magthetapix = " + boost::lexical_cast<std::string>(currmagthetapix)+"\n");
		//	DigitalMicrograph::UpdateImage(*MagTheta);
		//}

		col2currentmagthetapix = DigitalMicrograph::GetPixel(*MagTheta, 1, i);
	//	dTheta = abs(MeanTheta - DigitalMicrograph::GetPixel(*MagTheta, 1, i));

		dTheta = abs(MeanTheta - col2currentmagthetapix);


		//if (i == 0 || i == 1)
		//{
		//	DigitalMicrograph::Result("dtheta = " + boost::lexical_cast<std::string>(dTheta)+" col2magthetapix = " + boost::lexical_cast<std::string>(col2currentmagthetapix)+"\n");
		//}

		if ((dTheta < tolAng) && (dMag < tolMag))
		{
			nMeas++;
			//MagSum += DigitalMicrograph::GetPixel(*MagTheta, 0, i);
			MagSum += currmagthetapix;

			//ThetaSum += DigitalMicrograph::GetPixel(*MagTheta, 1, i);
			ThetaSum += col2currentmagthetapix;


			x = DigitalMicrograph::GetPixel(*MagTheta, 2, i);
			y = DigitalMicrograph::GetPixel(*MagTheta, 3, i);
			signX = (abs(x)) / x;

			VxSum += x*((abs(MeanVx)) / MeanVx)*signX;//second part here reverses
			VySum += y*((abs(MeanVx)) / MeanVx)*signX;//sign if x is opposite sign


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

		} //end of if(dtheta)

	}//end of for(i)
	//DigitalMicrograph::Result("If error here, it's in GetMeanG\n");
	//End of GetMeanG


	//DigitalMicrograph::Result("vxsum = " + boost::lexical_cast<std::string>(VxSum)+" meanvx = " + boost::lexical_cast<std::string>(MeanVx)+"\n");


	//currmagthetapix = DigitalMicrograph::GetPixel(*MagTheta, 0, i);
	//DigitalMicrograph::Result("Checking here.......Magtheta pix[0] = " + boost::lexical_cast<std::string>(currmagthetapix)+"\n");


}

//void Process::GetMeanG(DigitalMicrograph::Image MagTheta, DigitalMicrograph::Image &Cluster, float* Clusterpix, double &MeanMag, double &MeanTheta, double &MeanVx, double &MeanVy, int &nMeas)
//{
//
//	//incoming Mean values are a single vector
//	//outgoing mean values are an average of the vectors deemed to be in the cluster
//	//incoming mag/theta is the same for +/- vectors [since tan(q+pi)=tan(q)]
//	//so flip the vectors if the x-component has an opposite sign
//
//	double tolMag = 5;//tolerance in pixels to say it's in a cluster
//	double tolAng = 5;//tolerance in degrees to say it's in a cluster
//	long n, nVecs;
//	int i;
//	double dTheta, dMag, ThetaSum, VxSum, VySum, x, y, signX;
//
//	DigitalMicrograph::Image Magthetaclone;
//	Magthetaclone = DigitalMicrograph::ImageClone(MagTheta);
//	//Gatan::PlugIn::ImageDataLocker magthetaclonelock(Magthetaclone);
//	//float* magthetapix = (float*)magthetaclonelock.get();
//
//
//
//
//	DigitalMicrograph::Get2DSize(Cluster, &n, &nVecs);
//	//DigitalMicrograph::Result("n = " + boost::lexical_cast<std::string>(n)+", nVecs = " + boost::lexical_cast<std::string>(nVecs)+"\n");
//	nMeas = 0;
//	double MagSum = 0;
//
//	double currmagthetapix;
//	double col2currentmagthetapix;
//
//	for (i = 0; i < nVecs; i++)
//	{
//		currmagthetapix = DigitalMicrograph::GetPixel(Magthetaclone, 0, i);
//		//		dMag = abs(MeanMag - DigitalMicrograph::GetPixel(MagTheta, 0, i));
//		dMag = abs(MeanMag - currmagthetapix);
//		if (i == 0 || i == 1)
//		{
//			DigitalMicrograph::Result("dmag = " + boost::lexical_cast<std::string>(dMag)+"\n");
//			DigitalMicrograph::Result("magthetapix = " + boost::lexical_cast<std::string>(currmagthetapix)+"\n");
//			//DigitalMicrograph::UpdateImage(*MagTheta);
//		}
//
//		col2currentmagthetapix = DigitalMicrograph::GetPixel(Magthetaclone, 1, i);
//		//	dTheta = abs(MeanTheta - DigitalMicrograph::GetPixel(*MagTheta, 1, i));
//
//		dTheta = abs(MeanTheta - col2currentmagthetapix);
//
//
//		if (i == 0 || i == 1)
//		{
//			DigitalMicrograph::Result("dtheta = " + boost::lexical_cast<std::string>(dTheta)+" col2magthetapix = " + boost::lexical_cast<std::string>(col2currentmagthetapix)+"\n");
//		}
//
//		if ((dTheta < tolAng) && (dMag < tolMag))
//		{
//			nMeas++;
//			//MagSum += DigitalMicrograph::GetPixel(*MagTheta, 0, i);
//			MagSum += currmagthetapix;
//
//			//ThetaSum += DigitalMicrograph::GetPixel(*MagTheta, 1, i);
//			ThetaSum += col2currentmagthetapix;
//
//
//			x = DigitalMicrograph::GetPixel(Magthetaclone, 2, i);
//			y = DigitalMicrograph::GetPixel(Magthetaclone, 3, i);
//			signX = abs(x) / x;
//
//			VxSum += x*(abs(MeanVx) / MeanVx)*signX;//second part here reverses
//			VySum += y*(abs(MeanVx) / MeanVx)*signX;//sign if x is opposite sign
//
//
//			Clusterpix[i] = 1;
//
//
//			//int ii,jj;
//			//for (ii = i; ii <= i + 1; ii++)
//			//{
//			//	for (jj = 0; jj <= 1; jj++)
//			//	{
//			//		Clusterpix[n*jj+ii] = 1;//it is in the cluster
//			//	}
//			//}
//
//			//Cluster[i, 0, i + 1, 1] = 1;//it is in the cluster
//
//			MeanMag = MagSum / (double)nMeas;
//			MeanTheta = ThetaSum / (double)nMeas;
//			MeanVx = VxSum / (double)nMeas;
//			MeanVy = VySum / (double)nMeas;
//
//		} //end of if(dtheta)
//
//	}//end of for(i)
//	//DigitalMicrograph::Result("If error here, it's in GetMeanG\n");
//	//End of GetMeanG
//
//	//currmagthetapix = DigitalMicrograph::GetPixel(*MagTheta, 0, i);
//	//DigitalMicrograph::Result("Checking here.......Magtheta pix[0] = " + boost::lexical_cast<std::string>(currmagthetapix)+"\n");
//
//
//}


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

	//DigitalMicrograph::ImageDocumentShowAtPosition(DigitalMicrograph::ImageGetOrCreateImageDocument(AvCC), 15, 30);

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
	DigitalMicrograph::Result("about to tert...\n");
	for (int i = 0; i < (avccX*avccY); i++)
	{
		if (avccpix[i]>_top)
		{
			avccpix[i] = avccpix[i];
		}
		else
		{
			avccpix[i] = 0.001;
		}
	}
	DigitalMicrograph::ImageDataChanged(AvCC);
	//DigitalMicrograph::UpdateImage(AvCC);

	DigitalMicrograph::Image X, Y, TempImg;
	//DigitalMicrograph::Result("made xy images\n");
	//x- and y-coords as column vectors
	X = DigitalMicrograph::RealImage("X-coords", 4, 1, nPeaks);
	Y = DigitalMicrograph::RealImage("Y-coords", 4, 1, nPeaks);
	//DigitalMicrograph::Result("About to make disc\n");
	TempImg = DigitalMicrograph::RealImage("Deleting disc", 4, (2 * DdSize), (2 * DdSize));
	//DigitalMicrograph::Result("Made real image\n");
	Gatan::PlugIn::ImageDataLocker templock(TempImg);
	float* temppix = (float*)templock.get();
	//DigitalMicrograph::Result("At for loop to make 1s\n");
	for (int i = 0; i < ((2 * DdSize)*(2 * DdSize)); i++)
	{
		temppix[i] = 1;
	}
	ExtraGatan::InvCircularMask2D(&TempImg, DdSize, DdSize, DdSize);
	
	//DigitalMicrograph::ImageDocumentShowAtPosition(DigitalMicrograph::ImageGetOrCreateImageDocument(TempImg), 15, 30);

	DigitalMicrograph::Result("Made temp disc image\n");


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


			//for (k = 0; k < avccX*avccY; k++)
			//{
			//	avccpix[k] *= temppix[i];//this peak is done, delete it
			//}

			DigitalMicrograph::ImageDataChanged(AvCC);
			//DigitalMicrograph::UpdateImage(AvCC);

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

	//DigitalMicrograph::ImageDocumentShowAtPosition(DigitalMicrograph::ImageGetOrCreateImageDocument(Xx), 15, 30);
	//DigitalMicrograph::ImageDocumentShowAtPosition(DigitalMicrograph::ImageGetOrCreateImageDocument(Xxtrans), 15, 30);
	//DigitalMicrograph::ImageDocumentShowAtPosition(DigitalMicrograph::ImageGetOrCreateImageDocument(Vx), 15, 30);

	//DigitalMicrograph::ImageDocumentShowAtPosition(DigitalMicrograph::ImageGetOrCreateImageDocument(Vy), 15, 30);
	//DigitalMicrograph::ImageDocumentShowAtPosition(DigitalMicrograph::ImageGetOrCreateImageDocument(Yytrans), 15, 30);
	//DigitalMicrograph::ImageDocumentShowAtPosition(DigitalMicrograph::ImageGetOrCreateImageDocument(Yy), 15, 30);


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


	//DigitalMicrograph::ImageDocumentShowAtPosition(DigitalMicrograph::ImageGetOrCreateImageDocument(Vmag), 15, 30);
	//if (!DigitalMicrograph::OkCancelDialog("continue?"))
	//{
	//	return;
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


	//DigitalMicrograph::ImageDocumentShowAtPosition(DigitalMicrograph::ImageGetOrCreateImageDocument(Vtheta), 15, 30);
	//if (!DigitalMicrograph::OkCancelDialog("before magtheta, continue?"))
	//{
	//	return;
	//}


	//sort magnitude ascending into new column vector MagTheta

	long nVecs = (nPeaks*nPeaks - nPeaks) / 2;//number of different vectors


	DigitalMicrograph::Image MagTheta;
	MagTheta = DigitalMicrograph::RealImage("Mag-Theta-X-Y", 4, 4, nVecs);
	Gatan::PlugIn::ImageDataLocker magthetaLock(MagTheta);
	float* magthetapix = (float*)magthetaLock.get();

	DigitalMicrograph::Result("Before min\n");
	//may need to check min function
	float Mag = ExtraGatan::Min(Vmag);//lowest magnitude in the list
	ExtraGatan::PixelPos(Vmag, Mag, &xp, &yp, false);

	DigitalMicrograph::Result("After min and pixpos\n");

	//DigitalMicrograph::ImageDocumentShowAtPosition(DigitalMicrograph::ImageGetOrCreateImageDocument(Vmag), 15, 30);

	int iii=0;
	while (Mag < complete)
	{
		//DigitalMicrograph::Result("Mag = " + boost::lexical_cast<std::string>(Mag)+"\n");
		//DigitalMicrograph::Result("xp = " + boost::lexical_cast<std::string>(xp)+"\n");

		magthetapix[iii * 4] = vmagpix[nPeaks*yp + xp];
		magthetapix[iii * 4 + 1] = vthetapix[nPeaks*yp + xp];
		magthetapix[iii * 4 + 2] = vxpix[nPeaks*yp + xp];
		magthetapix[iii * 4 + 3] = vypix[nPeaks*yp + xp];
		vmagpix[nPeaks*yp + xp] = complete;
		
		//DigitalMicrograph::UpdateImage(Vmag);
		iii++;
		Mag = ExtraGatan::Min(Vmag);
		ExtraGatan::PixelPos(Vmag, Mag, &xp, &yp, false);
	}




	//DigitalMicrograph::ImageDocumentShowAtPosition(DigitalMicrograph::ImageGetOrCreateImageDocument(MagTheta), 15, 30);


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


	double MeanMag, MeanTheta, MeanVx, MeanVy; // may need to change to float???
	MeanMag = DigitalMicrograph::GetPixel(MagTheta, 0, 0);
	MeanTheta = DigitalMicrograph::GetPixel(MagTheta, 1, 0);
	MeanVx = DigitalMicrograph::GetPixel(MagTheta, 2, 0);
	MeanVy = DigitalMicrograph::GetPixel(MagTheta, 3, 0);

//	DigitalMicrograph::ImageDocumentShowAtPosition(DigitalMicrograph::ImageGetOrCreateImageDocument(Cluster), 40, 30);
	//DigitalMicrograph::ImageDocumentShowAtPosition(DigitalMicrograph::ImageGetOrCreateImageDocument(gVectors), 60, 30);


	DigitalMicrograph::Result("MeanMag = " + boost::lexical_cast<std::string>(MeanMag)+", MeanTheta = "+boost::lexical_cast<std::string>(MeanTheta)+"\n");


	int jjj = 1, kkk = 0;
	int nMeas = 1;//number of measured points to give an average g-vector

	//Go through and get clusters
	while (ExtraGatan::Sum(clusterpix, nVecs) < nVecs)
	{

		//DigitalMicrograph::Result("get cluster while loop poisition: " + boost::lexical_cast<std::string>(ExtraGatan::Sum(clusterpix, nVecs)) + "\n");

		GetMeanG(&MagTheta, Cluster, clusterpix, MeanMag, MeanTheta, MeanVx, MeanVy, nMeas);
		//DigitalMicrograph::UpdateImage(Cluster);
		gvecpix[kkk * 5] = MeanMag;
		gvecpix[kkk * 5 + 1] = MeanTheta;
		gvecpix[kkk * 5 + 2] = MeanVx;
		gvecpix[kkk * 5 + 3] = MeanVy;
		gvecpix[kkk * 5 + 4] = nMeas;
		//Find next unmatched point

		//DigitalMicrograph::UpdateImage(gVectors);

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
	g1X = DigitalMicrograph::GetPixel(gVectors, 2, 0);//these were changed to use the use the third instead of the first g-vectors in magtheta
	g1Y = DigitalMicrograph::GetPixel(gVectors, 3, 0);
	g2X = DigitalMicrograph::GetPixel(gVectors, 2, 1);
	g2Y = DigitalMicrograph::GetPixel(gVectors, 3, 1);
	
	DigitalMicrograph::Result("Found " + boost::lexical_cast<std::string>(nPeaks)+" different CBED disks, \n");
	DigitalMicrograph::Result("giving " + boost::lexical_cast<std::string>(kkk)+" different g-vectors\n");
	//show g-vectors on average image
	AddYellowArrow(&Avg, pX, pY, (pX + g1X), (pY + g1Y));//think this is wrong??
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

	DigitalMicrograph::Result("imgX =" + boost::lexical_cast<std::string>(imgX)+"\n");

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
	float tInc = Rr*0.8; //Must match tInc in collect!

	DigitalMicrograph::Result("nPts = " + boost::lexical_cast<std::string>(nPts)+"\n");

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
	DigitalMicrograph::Result("Made avg image\n");
	Gatan::PlugIn::ImageDataLocker avglock(Avg);
	float* avgpix = (float*)avglock.get();

	DigitalMicrograph::ImageDocumentShowAtPosition(DigitalMicrograph::ImageGetOrCreateImageDocument(CBED_stack), 15, 30);
	DigitalMicrograph::SetWindowSize(CBED_stack, 800, 800);
	DigitalMicrograph::Result("about to loop through pts...\n");
	//run through CBED stack
	//Result("disX = "+didsX+", disY = "+disY+"\n")
	long disX, disY, pt;
	DigitalMicrograph::Result("Radius :" + boost::lexical_cast<std::string>(Rr)+"\n");

	DigitalMicrograph::ImageDocument avgdisp;
	avgdisp = DigitalMicrograph::ImageGetOrCreateImageDocument(Avg);


	DigitalMicrograph::ImageDocumentShowAtPosition(avgdisp, 15, 30);
	try
	{
		for (pt = 0; pt < nPts; pt++)
		{

			DigitalMicrograph::OpenAndSetProgressWindow("Making average image of CBED Stack", (boost::lexical_cast<std::string>(ExtraGatan::round(((float)pt / nPts) * 100)) + "%").c_str(), "");
			//DigitalMicrograph::Result("Got to 1\n");
			try
			{
				GetCoordsFromNTilts(nTilts, pt, _i, _j);
				disX = ExtraGatan::round(_i*tInc);
				disY = ExtraGatan::round(_j*tInc);
				//DigitalMicrograph::Result("disX: " + boost::lexical_cast<std::string>(disX)+"disY: " + boost::lexical_cast<std::string>(disY)+"\n");
			}
			catch (...)
			{
				DigitalMicrograph::Result("GetCoords failed\n");
				return;
			}
			//DigitalMicrograph::Result("_i = " + boost::lexical_cast<std::string>(_i)+"_j = " + boost::lexical_cast<std::string>(_j)+"\n");
			//DigitalMicrograph::Result("Got to 2\n");
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
			//DigitalMicrograph::Result("Got to 3\n");
			//if (pt%20 == 0)
			//{
			//	DigitalMicrograph::UpdateImage(Avg);
			//}
			DigitalMicrograph::ImageDataChanged(Avg);
			//DigitalMicrograph::ImageDocumentUpdateDisplay(avgdisp);
			//DigitalMicrograph::UpdateImage(Avg);

		}//end of for pt<nPts
	//	DigitalMicrograph::UpdateImage(Avg);
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
	//DigitalMicrograph::ImageDocumentShowAtPosition(DigitalMicrograph::ImageGetOrCreateImageDocument(Disc), 15, 30);
	
	DigitalMicrograph::Image AvCC = DigitalMicrograph::CrossCorrelate(Avg, Disc);
	//DigitalMicrograph::ImageDocumentShowAtPosition(DigitalMicrograph::ImageGetOrCreateImageDocument(AvCC), 15, 30);

	
	//if (!DigitalMicrograph::OkCancelDialog("continue?"))
	//{
	//	return;
	//}

	
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
	//long t, l, b, r;
	std::string prompt;
	DigitalMicrograph::Image AvgTemp;
	float ppx, ppy;
	ppx = (float)pX;
	ppy = (float)pY;

	int nocount=0;

	while (!DigitalMicrograph::TwoButtonDialog("Are the measured g-vectors good?", "Yes", "No"))
	{

		if (nocount > 3)
		{
			if (DigitalMicrograph::OkCancelDialog("g-vectors have been attempted 3 or more times, would you like to quit?"))
			{
				DigitalMicrograph::Result("Quit at user request\n");
				return;
			}
		}

		UserG(Avg, Rr, g1X, g1Y, g2X, g2Y, ppx, ppy);
		nocount++;
		DigitalMicrograph::Result("Nocount: " + boost::lexical_cast<std::string>(nocount)+"\n");

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
	float Wfrac=75.0;
	if (!DigitalMicrograph::GetNumber("Percentage of spot used, 67-100%?", Wfrac, &Wfrac))
	{
		return;
	}

	DigitalMicrograph::Result("Creating stack of D-LACBED images...\n");

	long stackz = (long)((2 * nV1 + 1)*(2 * nV2 + 1));

	DigitalMicrograph::Image DLACBEDimg;
	DLACBEDimg = DigitalMicrograph::RealImage("D_LACBED Stack", 4, imgX, imgY, stackz);
	Gatan::PlugIn::ImageDataLocker DLACBEDlock(DLACBEDimg);
	float* DLACBEDpix = (float*)DLACBEDlock.get();
	//long stackz = ((2 * nV1 + 1)*(2 * nV2 + 1));
	for (ii = 0; ii < (imgX*imgY*stackz);ii++)
	{
		DLACBEDpix[ii] = 0;
	}



	DigitalMicrograph::Result("Made D_LACBED stack and set to 0\n");

	DigitalMicrograph::ImageDocumentShowAtPosition(DigitalMicrograph::ImageGetOrCreateImageDocument(DLACBEDimg), 15, 465);
	DigitalMicrograph::SetWindowSize(DLACBEDimg, 200, 200);
	DigitalMicrograph::ImageDocument LACBEDdisp;
	LACBEDdisp = DigitalMicrograph::GetFrontImageDocument();

	//Create scratch image for calculation of average
	DigitalMicrograph::Image ScratImg;
	ScratImg = DigitalMicrograph::RealImage("Average", 4, imgX, imgY);
	Gatan::PlugIn::ImageDataLocker scratlock(ScratImg);
	float* scratpix = (float*)scratlock.get();

	long Rr2 = (float)ExtraGatan::round((Wfrac*Rr) / 100);

	



	//DigitalMicrograph::Result("Made Scratimg and got Rr2\n");


	//other images for cut and copy
	DigitalMicrograph::Image TempImg, vTempImg;

	TempImg = DigitalMicrograph::RealImage("Disk", 4, 2 * Rr2, 2 * Rr2);
	Gatan::PlugIn::ImageDataLocker tempimglock(TempImg);
	float* tempimgpix = (float*)tempimglock.get();


	DigitalMicrograph::Result("Made tempimg \n");

	//vTempImg = DigitalMicrograph::RealImage("Temp", 4, 2 * Rr2, 2 * Rr2);

	vTempImg = ExtraGatan::MakeDisc(2 * Rr2, 2 * Rr2, Rr2);
	Gatan::PlugIn::ImageDataLocker vtempimglock(vTempImg);
	float* vtempimgpix = (float*)vtempimglock.get();


	//loop over DLACBED image
	long gNo = 0;
	long X, Y, m1, m2;
	bool inside;


	DigitalMicrograph::Result("about to loop\n");


	for (m1 = -nV1; m1 < nV1 + 1; m1++)
	{
		for (m2 = -nV2; m2 < nV2 + 1; m2++)
		{
			
			DigitalMicrograph::OpenAndSetProgressWindow("Making DLACBED Stack", (boost::lexical_cast<std::string>(ExtraGatan::round(((float)gNo / (stackz) * 100))) + "%").c_str(), "");

			//loop over CBED stack
			for (pt = 0; pt < nPts; pt++)
			{
				GetCoordsFromNTilts(nTilts, pt, _i, _j);
				//appropriate vector for disk
				//DigitalMicrograph::Result("Got coordsfromntilts : i ="+boost::lexical_cast<std::string>(_i)+", j ="+boost::lexical_cast<std::string>(_j)+"\n");
				X = ExtraGatan::round(pX + ((float)_i)*tInc + (float)(m1*g1X) + (float)(m2*g2X));
				Y = ExtraGatan::round(pY + ((float)_j)*tInc + (float)(m1*g1Y) + (float)(m2*g2Y));

				//DigitalMicrograph::Result("X =" + boost::lexical_cast<std::string>(X)+", Y =" + boost::lexical_cast<std::string>(Y)+" about to decide if inside\n");


				inside = !((X - Rr2<0) || (X + Rr2>imgX) || (Y - Rr2<0) || (Y + Rr2>imgY));

				//DigitalMicrograph::Result("inside =" + boost::lexical_cast<std::string>(inside)+"\n");
				
				
				//std::copy(img1Pix, img1Pix + (imgX*imgY), CBEDPix + (pt*imgX*imgY));



				if (inside)
				{

					///////////////////////////////////////////////////////////////////////////////////////////////
					//for (ii = 0; ii < 2 * Rr2; ii++)
					//{
					//	for (jj = 0; jj < 2 * Rr2; jj++)
					//	{
					//		tempimgpix[jj*(2*Rr2) + ii] = cbedpix[pt*imgX*imgY + (jj + Y - Rr2)*imgX + (ii + X - Rr2)];
					//	}
					//}
					///////////////////////////////////////////////////////////////////////////////////////////////////

					for (jj = 0; jj < 2 * Rr2; jj++)
					{
						std::copy(cbedpix + pt*imgX*imgY + (jj + Y - Rr2)*imgX + (X - Rr2), cbedpix + pt*imgX*imgY + (jj + Y - Rr2)*imgX + (X + Rr2), tempimgpix +(2*Rr2*jj));
					}


					//Cropped to be circular
					
					//DigitalMicrograph::Result("About to mask circle\n");

					ExtraGatan::CircularMask2D(&TempImg, Rr2, Rr2, Rr2); //may need to adjust the y-coordinate in this function

					//TempImg.GetOrCreateImageDocument().ShowAtPosition(440, 30);
					

					//Add it to the LACBED pattern

					//DigitalMicrograph::Result("about to add to dlacbed, gNo = "+boost::lexical_cast<std::string>(gNo)+"\n");

				

					for (ii = 0; ii < 2*Rr2; ii++)
					{
						for (jj = 0; jj < 2*Rr2; jj++)
						{
							DLACBEDpix[imgX*imgY*gNo + imgX*(jj+Y-Rr2) + (ii+X-Rr2)] += tempimgpix[jj*(2*Rr2) + ii];//may be wrong index numbers
							
						}
					}

					//for (jj = 0; jj < 2 * Rr2; jj++)
					//{
					//	std::copy(tempimgpix + (jj*Rr2), tempimgpix + (jj*Rr2) + Rr2, DLACBEDpix + imgX*imgY*gNo + imgX*(jj + Y - Rr2) + (X - Rr2));
					//}


					//DigitalMicrograph::UpdateImage(DLACBEDimg);
				//	DigitalMicrograph::UpdateDisplay(DLACBEDimg, imgY*gNo , imgY*gNo + imgX*imgY);
				//	DigitalMicrograph::ImageDocumentUpdateDisplay(LACBEDdisp);
					DigitalMicrograph::ImageDataChanged(DLACBEDimg);



					//Update mask which keeps count of the number of images in one pixel


					for (ii = 0; ii < 2*Rr2; ii++)
					{
						for (jj = 0; jj < 2*Rr2; jj++)
						{
							scratpix[imgX*(jj+Y-Rr2) + (ii+X-Rr2)] += vtempimgpix[jj*(2*Rr2) + ii];//may be wrong index numbers, also +=
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

			//if (!DigitalMicrograph::OkCancelDialog("continue?"))
			//{
			//	return;
			//}


			//make pixels with zero values equal 1
			//DigitalMicrograph::UpdateImage(DLACBEDimg);
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
		//	DigitalMicrograph::UpdateImage(DLACBEDimg);
			gNo++;
		//	DigitalMicrograph::Result("At end of m2 loop\n");
		}//end of for m2
		

	}

	float dlacmin, dlacmax;
	DigitalMicrograph::Result("About to find min values\n");

	if (!DigitalMicrograph::OkCancelDialog("continue?"))
	{
		return;
	}


	dlacmin = ExtraGatan::Min3D(DLACBEDimg);
	dlacmax = ExtraGatan::Max3D(DLACBEDimg);
	DigitalMicrograph::Result("Got min max, about to set limits\n");


	DigitalMicrograph::SetLimits(DLACBEDimg, dlacmin, dlacmax);
	DigitalMicrograph::Result("Found values, about to tidy up\n");

	//Tidy up
	DigitalMicrograph::DeleteImage(TempImg);
	DigitalMicrograph::DeleteImage(vTempImg);
	DigitalMicrograph::Result(" Done\n");

	///////////////////////////////////////////
	//Montage of D-LACBED images
	DigitalMicrograph::Result("Creating Montage of D-LACBED images...\n");

	bool ttt, fff;
	ttt = true;
	fff = false;

	DigitalMicrograph::Result("true = " + boost::lexical_cast<std::string>(ttt)+", false = " + boost::lexical_cast<std::string>(fff)+"\n");


	if (!DigitalMicrograph::OkCancelDialog("continue?"))
	{
		return;
	}


	//each D-LACBED image is (2*nTilts+3)*Rr wide

	float wid;
	wid = (nTilts + 1)*Rr;
	//Scaling factors between CBED image and montage image
	//F is the relative size of D-LACBED vs original disc size
	//Uses the largest component of the smallest g-vector

	DigitalMicrograph::Result("Wid = " + boost::lexical_cast<std::string>(wid)+" \n");

	float F, FimgX;//may need to change types

	float maxofg1, maxofg2;
	maxofg1 = ExtraGatan::maxoftwo(g1X, g1Y);
	maxofg2 = ExtraGatan::maxoftwo(g2X, g2Y);

	DigitalMicrograph::Result("maxg1 = " + boost::lexical_cast<std::string>(maxofg1)+" \n");
	DigitalMicrograph::Result("maxg2 = " + boost::lexical_cast<std::string>(maxofg2)+" \n");



	if (g1mag < g2mag)
	{
		F = (/*2 */ wid) / (ExtraGatan::minoftwo(abs(g1X), abs(g1Y)));
	}
	else
	{
		F = (/*2 */ wid) / (ExtraGatan::minoftwo(abs(g2X), abs(g2Y)));
	}


	//F = 2.5*(2 * nTilts + 3) / 4;

	DigitalMicrograph::Result("F = " + boost::lexical_cast<std::string>(F)+" \n");


	//FimgX is the size of the montage, uses the largest dimension
	//depending on number of reflections and magnitude


	if (ExtraGatan::maxoftwo(-g1X, -g1Y) > ExtraGatan::maxoftwo(-g2X, -g2Y))
	{
		FimgX = ExtraGatan::maxoftwo((4 * nV1*ExtraGatan::maxoftwo(g1X, g1Y)*F + 2 * wid), (4 * nV2 * 2 * wid + 2 * wid));
	}
	else
	{
		FimgX = ExtraGatan::maxoftwo((4 * nV2*ExtraGatan::maxoftwo(g1X, g1Y)*F + 2 * wid), (4 * nV1 * 2 * wid + 2 * wid));
	}


	//FimgX = 1.4*(2 * ExtraGatan::maxoftwo(nV1, nV2) + 1)*(2 * nTilts + 3)*Rr;
	

	DigitalMicrograph::Result("FimgX = " + boost::lexical_cast<std::string>(FimgX)+" \n");


	float FimgY = FimgX;
	//The 000 image will be in the centre
	float Lx, Ly;
	Lx = ExtraGatan::round(FimgX / 2);
	Ly = ExtraGatan::round(FimgY / 2);

	DigitalMicrograph::Image Montage;
	Montage = DigitalMicrograph::RealImage("D-LACBED montage", 4, (long)FimgX, (long)FimgY);
	Gatan::PlugIn::ImageDataLocker montagelock(Montage);
	float* montagepix = (float*)montagelock.get();

	DigitalMicrograph::ImageDocument montagedisp;

	montagedisp = DigitalMicrograph::ImageGetOrCreateImageDocument(Montage);

	montagedisp.ShowAtPosition(440, 30);
	DigitalMicrograph::SetWindowSize(Montage, 0.75*imgX, 0.75*imgY);
	//DigitalMicrograph::UpdateImage(Montage);

	//DigitalMicrograph::ImageDocumentUpdateDisplay(montagedisp);
	DigitalMicrograph::ImageDataChanged(Montage);


	DigitalMicrograph::Image croppedDLACBED;
	croppedDLACBED = DigitalMicrograph::RealImage("Cropped DLACBED", 4, 2 * wid, 2 * wid, stackz);

	Gatan::PlugIn::ImageDataLocker cropDLACBEDlock(croppedDLACBED);
	float* croppedpix = (float*)cropDLACBEDlock.get();



	long t2, l2, b2, r2, t1, l1, b1, r1;
	float a2X, a2Y, a1X, a1Y;
	gNo = 0;

	//bool insidecpy;
	//insidecpy = true;
	long insidelong;
	insidelong = 2;

	DigitalMicrograph::Result("g1X = " + boost::lexical_cast<std::string>(g1X)+", g2X = " + boost::lexical_cast<std::string>(g2X)+"\n");
	DigitalMicrograph::Result("g1Y = " + boost::lexical_cast<std::string>(g1Y)+", g2Y = " + boost::lexical_cast<std::string>(g2Y)+"\n");
	int notinsidecount;

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
			
			//DigitalMicrograph::Result("t2 = " + boost::lexical_cast<std::string>(t2)+", l2 = " + boost::lexical_cast<std::string>(l2));
			//DigitalMicrograph::Result(", b2 = " + boost::lexical_cast<std::string>(b2)+", r2 = " + boost::lexical_cast<std::string>(r2)+"\n");


			//a1 is the location of the rectangle where the D-LACBED image will go in the montage
			a1X = ExtraGatan::round(Lx + (m1*g1X + m2*g2X)*F);
			a1Y = ExtraGatan::round(Ly + (m1*g1Y + m2*g2Y)*F);

			DigitalMicrograph::Result("a1X = " + boost::lexical_cast<std::string>(a1X)+", a1Y = " + boost::lexical_cast<std::string>(a1Y)+"\n");


			t1 = (long)ExtraGatan::round(a1Y - a2Y + t2);
			l1 = (long)ExtraGatan::round(a1X - a2X + l2);
			b1 = (long)ExtraGatan::round(a1Y + b2 - a2Y);
			r1 = (long)ExtraGatan::round(a1X + r2 - a2X);



			//DigitalMicrograph::Result("t1 = " + boost::lexical_cast<std::string>(t1)+", l1 = " + boost::lexical_cast<std::string>(l1));
			//DigitalMicrograph::Result(", b1 = " + boost::lexical_cast<std::string>(b1)+", r1 = " + boost::lexical_cast<std::string>(r1)+"\n");


			//if (!DigitalMicrograph::OkCancelDialog("about to add to montage, continue?"))
			//{
			//	return;
			//}

			//DigitalMicrograph::Result("Deciding if inside...\n");
			//DigitalMicrograph::Result("l1<0 = " + boost::lexical_cast<std::string>(l1 < 0) + "\n");
			//DigitalMicrograph::Result("r1>FimgX = " + boost::lexical_cast<std::string>(r1 > FimgX) + "\n");
			//DigitalMicrograph::Result("t1<0 = " + boost::lexical_cast<std::string>(t1 < 0) + "\n");
			//DigitalMicrograph::Result("b1>FimgY = " + boost::lexical_cast<std::string>(r1 > FimgY) + "\n");


			inside = !((l1<0) || (r1>FimgX) || (t1<0) || (b1>FimgY));
			//insidelong = (long)(l1 < 0);
			//insidelong = insidelong + (long)(r1 > FimgX);
			//insidelong = insidelong + (long)(t1 < 0);
			//insidelong = insidelong + (long)(b1 > FimgY);
			//insidecpy = inside;
			//insidelong = (long)inside;
			//DigitalMicrograph::Result("Inside = " + boost::lexical_cast<std::string>(insidecpy)+"\n");
			//DigitalMicrograph::Result("Insidelong = " + boost::lexical_cast<std::string>(insidelong)+"\n");


			if (inside)
			{
				for (ii = 0; ii < abs(r1-l1); ii++)
				{
					for (jj = 0; jj < abs(b1-t1); jj++)
					{
						if (DLACBEDpix[(imgX*imgY*gNo) + (jj + t2)*imgX + (ii + l2)] == 0)
						{
							//DigitalMicrograph::Result("LACBED Pixel was 0\n");
							montagepix[(t1 + jj)*(long)FimgX + (l1 + ii)] = montagepix[(t1 + jj)*(long)FimgX + (l1 + ii)];
						}
						else
						{
							//if (gNo == 4 || gNo == 5)
							//{
							//	DigitalMicrograph::Result("Going to change pixel...\n");
							//}
							//DigitalMicrograph::Result("Wasn't 0\n");

							montagepix[(t1 + jj)*(long)FimgX + (l1 + ii)] = DLACBEDpix[(imgX*imgY*gNo) + (jj + t2)*imgX + (ii + l2)];
						}

						
						croppedpix[(long)(wid*wid*4*gNo) +jj * 2 * (long)wid + ii] = DLACBEDpix[(imgX*imgY*gNo) + (jj + t2)*imgX + (ii + l2)];



					}
				}
				DigitalMicrograph::ImageDataChanged(Montage);//end of if(inside)
			}
			else
			{
				notinsidecount++;
			}
			
			DigitalMicrograph::Result("End of if inside\n");
			DigitalMicrograph::Result("LACBED image: " + boost::lexical_cast<std::string>(gNo)+"\n");
			gNo++;

			
		}
	}


if (notinsidecount > 0)
{
	DigitalMicrograph::OkDialog("One or more of the LACBED images were not included in the montage\nYou may wish to retry processing and ensure the correct g-vectors are found");
}

if (DigitalMicrograph::OkCancelDialog("Would you like a cropped version of the DLACBED stack?"))
{

	
	//DigitalMicrograph::UpdateImage(croppedDLACBED);
	
	croppedDLACBED.GetOrCreateImageDocument().ShowAtPosition(15, 30);


}
else
{
	DigitalMicrograph::DeleteImage(croppedDLACBED);
}


	float montmin, montmax;
	montmin = ExtraGatan::Min(Montage);
	montmax = ExtraGatan::Max(Montage);
	DigitalMicrograph::SetLimits(Montage, montmin, montmax);

	DigitalMicrograph::GetDate(f, date, 11);
	DigitalMicrograph::GetTime(f, time, 6);
	for (i = 0; i<10; i++)
	{
		datestring += date[i];
	}
	for (i = 0; i < 5; i++)
	{
		timestring += time[i];
	}
	datetimestring = datestring + "_" + timestring;
	DigitalMicrograph::Result("Processing complete: " + datetimestring + " ding, dong\n\n");

}