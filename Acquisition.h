#pragma once
#include "stdafx.h"
#include "DMPluginCamera.h"

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
};