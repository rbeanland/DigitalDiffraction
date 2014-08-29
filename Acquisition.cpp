#include "stdafx.h"
#include "Acquisition.h"
#include "boost/lexical_cast.hpp"
#include <time.h>

Acquisition::Acquisition()
{ }

Acquisition::~Acquisition()
{ }

void Acquisition::CheckCamera()//Checks that a camera is inserted
{
	try
	{
		camera = Gatan::Camera::GetCurrentCamera();
		Gatan::Camera::CCD_GetSize(camera, &xpixels, &ypixels);
	}
	catch (...)
	{
		DigitalMicrograph::Result("failed in try get current camera\n");
		short error;
		long context;
		DigitalMicrograph::GetException(&error, &context);
		DigitalMicrograph::ErrorDialog(error);
		DigitalMicrograph::OpenAndSetProgressWindow("No Camera Detected", "", "");
		return;
	}
	inserted = false;
	try
	{
		inserted = Gatan::Camera::GetCameraInserted(camera);
	}
	catch (...)
	{
		DigitalMicrograph::Result("failed in try get inserted camera\n");
		short error;
		long context;
		DigitalMicrograph::GetException(&error, &context);
		DigitalMicrograph::ErrorDialog(error);
		DigitalMicrograph::OpenAndSetProgressWindow("Couldn't check camera", "status", "");
		return;
	}
	if (inserted != true)
	{
		DigitalMicrograph::OpenAndSetProgressWindow("Camera not inserted", "", "");
		return;
	}
}

DigitalMicrograph::ScriptObject Acquisition::GetFrameSetInfoPtr(DigitalMicrograph::ScriptObject &Acquis)

{
	DigitalMicrograph::Result("In get fsi\n");
	static DigitalMicrograph::Function __sFunction = (DM_FunctionToken)NULL;
	static const char *__sSignature = "ScriptObject GetFrameSetInfoPtr( ScriptObject * )";
	Gatan::PlugIn::DM_Variant params[2];
	params[1].v_object_ref = (DM_ObjectToken*)Acquis.get_ptr();
	GatanPlugIn::gDigitalMicrographInterface.CallFunction(__sFunction.get_ptr(), 2, params, __sSignature);
	return (DM_ScriptObjectToken_1Ref)params[0].v_object;
};

void Acquisition::SetAcquireParameters()//Sets up the acquisition parameters (once) ready for any image acquisitions made.
{
	DigitalMicrograph::Result("about to set processing to unprocessed\n");
	processing = Gatan::Camera::kUnprocessed;

	bool temp_kunprocessed_selected = false;
	bool temp_kdarksubtracted_selected = false;
	bool temp_kgainnormalized_selected = false;
	bool temp_kmaxprocessing_selected = false;
	DigitalMicrograph::TagGroup Persistent;
	Persistent = DigitalMicrograph::GetPersistentTagGroup();
	std::string Tag_path;
	Tag_path = "DigitalDiffraction:Settings:";
	//Getting the processing setting set in the global tag settings, the default is set to kUnprocessed if settings fail to load
	try
	{
		DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (Tag_path + "kUnprocessed").c_str(), &temp_kunprocessed_selected);
		DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (Tag_path + "kDarkSubtracted").c_str(), &temp_kdarksubtracted_selected);
		DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (Tag_path + "kGainNormalized").c_str(), &temp_kgainnormalized_selected);
		DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (Tag_path + "kMaxProcessing").c_str(), &temp_kmaxprocessing_selected);
	}
	catch (...)
	{
		DigitalMicrograph::Result("Failed to load settings information\n");
	}
	if (temp_kunprocessed_selected == true) //Change the processing mode to whichever was selected in the settings
	{
		processing = Gatan::Camera::kUnprocessed;
		DigitalMicrograph::Result("Mode : kUnprocessed\n");
	}
	if (temp_kdarksubtracted_selected == true)
	{
		processing = Gatan::Camera::kDarkSubtracted;
		DigitalMicrograph::Result("Mode : kDarkSubtracted\n");
	}
	if (temp_kgainnormalized_selected == true)
	{
		processing = Gatan::Camera::kGainNormalized;
		DigitalMicrograph::Result("Mode : kGainNormalized\n");
	}
	if (temp_kmaxprocessing_selected == true)
	{
		processing = Gatan::Camera::kMaxProcessing;
		DigitalMicrograph::Result("Mode : kMaxProcessing\n");
	}
	Gatan::uint32 binx, biny;
	binx = (Gatan::uint32)binning;
	biny = (Gatan::uint32)binning;
	try
	{
		acqparams = Gatan::Camera::CreateAcquisitionParameters_FullCCD(camera, processing, expo, binx, biny);
		Gatan::CM::SetDoContinuousReadout(acqparams, true);
		Gatan::CM::SetQualityLevel(acqparams, 0); // Can't remember if fast or slow :D
		Gatan::Camera::Validate_AcquisitionParameters(camera, acqparams);
		DigitalMicrograph::Result("validated parameters\n");
	}
	catch (...)
	{
		DigitalMicrograph::Result("failed in try create acq params, readout, quality, validate\n");
		short error;
		long context;
		DigitalMicrograph::GetException(&error, &context);
		DigitalMicrograph::ErrorDialog(error);
		DigitalMicrograph::OpenAndSetProgressWindow("Problem with acquisition", "parameters", "");
		return;
	}
	acq = Gatan::CM::CreateAcquisition(camera, acqparams);
	acqtok = DigitalMicrograph::ScriptObjectProxy<Gatan::Camera::AcquisitionImp, DigitalMicrograph::DMObject>::to_object_token(acq.get());
	DigitalMicrograph::Result("6\n");
	try
	{
		fsi = Acquisition::GetFrameSetInfoPtr(acqtok);
		acqsource = Gatan::Camera::AcquisitionImageSource::New(acq, fsi, 0);
		DigitalMicrograph::Result("8\n");
	}
	catch (...)
	{
		DigitalMicrograph::Result("Failed to get fsi/acqsource\n");
	}
}

void Acquisition::AcquireImage(DigitalMicrograph::Image &AcquiredInput) // SetAcquireParameters must be called once before this function is used. Currently replaced by AcquireImage2
{
	DigitalMicrograph::Image AcquiredImage;

	DigitalMicrograph::Result("Before acq->Begin\n");
	try
	{
		acqsource->BeginAcquisition();
		acqprmchanged = false;
		AcquiredImage = Gatan::Camera::CreateImageForAcquire(acq, "Acquired Image");
		clock_t start = clock();
		if (!acqsource->AcquireTo(AcquiredImage, true, 0.5, acqprmchanged))
		{
			// Now wait for it to finish again but dont restart if it finishes durign call....
			while (!acqsource->AcquireTo(AcquiredImage, false,/* 1*/0.5, acqprmchanged))
			{
				// Waiting for read to finish
			}
		}
		clock_t finish = clock() - start;
		DigitalMicrograph::Result("Acquisition time inside function = " + boost::lexical_cast<std::string>(((float)finish) / CLOCKS_PER_SEC) + " seconds\n");

		acqsource->FinishAcquisition();
		AcquiredInput = AcquiredImage;
		return;
	}
	catch (...)
	{
		DigitalMicrograph::Result("Failed in acquisition\n");
		return;
	}
}

void Acquisition::AcquireImage2(DigitalMicrograph::Image &AcquiredInput)//an acq_source->BeginAcquisition must be called once before this function is used, and ->FinishAcquisition must be called once after all acquisitions have been done
{
	try
	{
		acqprmchanged = false;
	//	AcquiredInput = Gatan::Camera::CreateImageForAcquire(acq, "Acquired Image");// This is now done outside the function, this was causing the live image problem
		if (!acqsource->AcquireTo(AcquiredInput, true, 0.5, acqprmchanged))//updateing the AcquiredInput image to current 'live' image.
		{
			// Now wait for it to finish again but dont restart if it finishes durign call....
			while (!acqsource->AcquireTo(AcquiredInput, false, 0.5, acqprmchanged))
			{
				// Waiting for read to finish
			}
		}
		return;
	}
	catch (...)
	{
		DigitalMicrograph::Result("Failed in acquisition\n");
		return;
	}
}

