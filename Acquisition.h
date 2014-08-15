#pragma once
#include "stdafx.h"
#include "DMPluginCamera.h"


//class Acquisition
//{
//public:
//	Acquisition();
//	virtual ~Acquisition();
//
//	static void CheckCamera(Gatan::Camera::Camera &camera, bool &inserted, Gatan::uint32 &xpixels, Gatan::uint32 &ypixels);
//	static DigitalMicrograph::ScriptObject GetFrameSetInfoPtr(DigitalMicrograph::ScriptObject&);
//	static void SetAcquireParameters(Gatan::Camera::AcquisitionProcessing &processing, Gatan::Camera::Camera &camera, double expo, int binning, Gatan::uint32 &xpixels, Gatan::uint32 &ypixels, Gatan::Camera::AcquisitionParameters &acqparams, Gatan::CM::AcquisitionPtr &acq, DigitalMicrograph::ScriptObject &acqtok, Gatan::CM::FrameSetInfoPtr &fsi, Gatan::Camera::AcquisitionImageSourcePtr &acqsource);
//	static void AcquireImage(DigitalMicrograph::Image &AcquiredInput, Gatan::Camera::AcquisitionImageSourcePtr &acqsource, Gatan::CM::AcquisitionPtr &acq, bool &acqprmchanged);
//	//static void AcquireImage()
//};


class Acquisition
{
public:
	Acquisition();
	virtual ~Acquisition();
	Gatan::Camera::Camera camera;
	Gatan::uint32 xpixels;
	Gatan::uint32 ypixels;
	bool inserted;
	Gatan::Camera::AcquisitionProcessing processing;
	Gatan::Camera::AcquisitionParameters acqparams;
	double expo;
	int binning;

	Gatan::CM::AcquisitionPtr acq;
	DigitalMicrograph::ScriptObject acqtok;
	Gatan::CM::FrameSetInfoPtr fsi;
	Gatan::Camera::AcquisitionImageSourcePtr acqsource;

	bool acqprmchanged;
	DigitalMicrograph::Image AcquiredImage;

	void CheckCamera();
	DigitalMicrograph::ScriptObject GetFrameSetInfoPtr(DigitalMicrograph::ScriptObject&);
	void SetAcquireParameters();
	void AcquireImage(DigitalMicrograph::Image &AcquiredInput);
	void AcquireImage2(DigitalMicrograph::Image &AcquiredInput);
	//static void AcquireImage()
};