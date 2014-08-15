#include "stdafx.h"
#include "Acquisition.h"

#include "boost/lexical_cast.hpp"
#include <time.h>


Acquisition::Acquisition()
{

}

Acquisition::~Acquisition()
{

}

//void Acquisition::CheckCamera(Gatan::Camera::Camera &camera, bool &inserted, Gatan::uint32 &xpixels, Gatan::uint32 &ypixels)
//{
//	DigitalMicrograph::Result("In check camera\n");
//	//Gatan::Camera::Camera camera;
//	try
//	{
//		camera = Gatan::Camera::GetCurrentCamera();
//		Gatan::Camera::CCD_GetSize(camera, &xpixels, &ypixels);
//	}
//	catch (...)
//	{
//		DigitalMicrograph::Result("failed in try get current camera\n");
//		short error;
//		long context;
//		DigitalMicrograph::GetException(&error, &context);
//		DigitalMicrograph::ErrorDialog(error);
//		DigitalMicrograph::OpenAndSetProgressWindow("No Camera Detected", "", "");
//		return;
//	}
//	inserted = false;
//	try
//	{
//		inserted = Gatan::Camera::GetCameraInserted(camera);
//	}
//	catch (...)
//	{
//		DigitalMicrograph::Result("failed in try get inserted camera\n");
//		short error;
//		long context;
//		DigitalMicrograph::GetException(&error, &context);
//		DigitalMicrograph::ErrorDialog(error);
//		DigitalMicrograph::OpenAndSetProgressWindow("Couldn't check camera", "status", "");
//		return;
//	}
//
//	if (inserted != true)
//	{
//		DigitalMicrograph::OpenAndSetProgressWindow("Camera not inserted", "", "");
//		return;
//	}
//}
//
//DigitalMicrograph::ScriptObject Acquisition::GetFrameSetInfoPtr(DigitalMicrograph::ScriptObject &Acquis)
//
//{
//	DigitalMicrograph::Result("In get fsi\n");
//	static DigitalMicrograph::Function __sFunction = (DM_FunctionToken)NULL;
//
//	static const char *__sSignature = "ScriptObject GetFrameSetInfoPtr( ScriptObject * )";
//
//	Gatan::PlugIn::DM_Variant params[2];
//	
//	params[1].v_object_ref = (DM_ObjectToken*)Acquis.get_ptr();
//
//	GatanPlugIn::gDigitalMicrographInterface.CallFunction(__sFunction.get_ptr(), 2, params, __sSignature);
//	
//	return (DM_ScriptObjectToken_1Ref)params[0].v_object;
//
//};
//
//void Acquisition::SetAcquireParameters(Gatan::Camera::AcquisitionProcessing &processing, Gatan::Camera::Camera &camera, double expo, int binning, Gatan::uint32 &xpixels, Gatan::uint32 &ypixels, Gatan::Camera::AcquisitionParameters &acqparams, Gatan::CM::AcquisitionPtr &acq, DigitalMicrograph::ScriptObject &acqtok, Gatan::CM::FrameSetInfoPtr &fsi, Gatan::Camera::AcquisitionImageSourcePtr &acqsource)
//{
//	DigitalMicrograph::Result("about to set processing to unprocessed\n");
//	processing = Gatan::Camera::kUnprocessed;
//
//	bool temp_kunprocessed_selected = false;
//	bool temp_kdarksubtracted_selected = false;
//	bool temp_kgainnormalized_selected = false;
//	bool temp_kmaxprocessing_selected = false;
//	DigitalMicrograph::TagGroup Persistent;
//	Persistent = DigitalMicrograph::GetPersistentTagGroup();
//	std::string Tag_path;
//	Tag_path = "DigitalDiffraction:Settings:";
//	try
//	{
//		DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (Tag_path + "kUnprocessed").c_str(), &temp_kunprocessed_selected);
//		DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (Tag_path + "kDarkSubtracted").c_str(), &temp_kdarksubtracted_selected);
//		DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (Tag_path + "kGainNormalized").c_str(), &temp_kgainnormalized_selected);
//		DigitalMicrograph::TagGroupGetTagAsBoolean(Persistent, (Tag_path + "kMaxProcessing").c_str(), &temp_kmaxprocessing_selected);
//	}
//	catch (...)
//	{
//		DigitalMicrograph::Result("Failed to load settings information\n");
//	}
//	if (temp_kunprocessed_selected == true)
//	{
//		processing = Gatan::Camera::kUnprocessed;
//		DigitalMicrograph::Result("Mode : kUnprocessed\n");
//	}
//	if (temp_kdarksubtracted_selected == true)
//	{
//		processing = Gatan::Camera::kDarkSubtracted;
//		DigitalMicrograph::Result("Mode : kDarkSubtracted\n");
//	}
//	if (temp_kgainnormalized_selected == true)
//	{
//		processing = Gatan::Camera::kGainNormalized;
//		DigitalMicrograph::Result("Mode : kGainNormalized\n");
//	}
//	if (temp_kmaxprocessing_selected == true)
//	{
//		processing = Gatan::Camera::kMaxProcessing;
//		DigitalMicrograph::Result("Mode : kMaxProcessing\n");
//	}
//
//
//	DigitalMicrograph::Result("binning: " + boost::lexical_cast<std::string>(binning)+"\n");
//	DigitalMicrograph::Result("expo: " + boost::lexical_cast<std::string>(expo)+"\n");
//
//	Gatan::uint32 binx, biny;
//	binx = (Gatan::uint32)binning;
//	biny = (Gatan::uint32)binning;
//	DigitalMicrograph::Result("About to try\n");
//	try
//	{
//		//acqparams = Gatan::Camera::CreateAcquisitionParameters(camera, processing, expo, binx, biny, 0, 0, ypixels, xpixels);
//		acqparams = Gatan::Camera::CreateAcquisitionParameters_FullCCD(camera, processing, expo, binx, biny);
//		DigitalMicrograph::Result("binning: " + boost::lexical_cast<std::string>(binning)+"\n");
//		DigitalMicrograph::Result("binning: " + boost::lexical_cast<std::string>(binx)+"\n");
//
//		Gatan::CM::SetDoContinuousReadout(acqparams, true);
//		DigitalMicrograph::Result("2\n");
//		Gatan::CM::SetQualityLevel(acqparams, 0); // Can't remember if fast or slow :D
//		DigitalMicrograph::Result("3\n");
//		Gatan::Camera::Validate_AcquisitionParameters(camera, acqparams);
//		DigitalMicrograph::Result("validated parameters\n");
//	}
//	catch (...)
//	{
//		DigitalMicrograph::Result("failed in try create acq params, readout, quality, validate\n");
//		short error;
//		long context;
//		DigitalMicrograph::GetException(&error, &context);
//		DigitalMicrograph::ErrorDialog(error);
//		DigitalMicrograph::OpenAndSetProgressWindow("Problem with acquisition", "parameters", "");
//		return;
//	}
//
//	acq = Gatan::CM::CreateAcquisition(camera, acqparams);
//	DigitalMicrograph::Result("5\n");
//	acqtok = DigitalMicrograph::ScriptObjectProxy<Gatan::Camera::AcquisitionImp, DigitalMicrograph::DMObject>::to_object_token(acq.get());
//	DigitalMicrograph::Result("6\n");
//	try
//	{
//		fsi = Acquisition::GetFrameSetInfoPtr(acqtok);
//		DigitalMicrograph::Result("7\n");
//
//
//		acqsource = Gatan::Camera::AcquisitionImageSource::New(acq, fsi, 0);
//		DigitalMicrograph::Result("8\n");
//	}
//	catch (...)
//	{
//		DigitalMicrograph::Result("Failed to get fsi/acqsource\n");
//	}
//}
//
//void Acquisition::AcquireImage(DigitalMicrograph::Image &AcquiredInput, Gatan::Camera::AcquisitionImageSourcePtr &acqsource, Gatan::CM::AcquisitionPtr &acq, bool &acqprmchanged)
//{
//	DigitalMicrograph::Image AcquiredImage;
//	DigitalMicrograph::Result("Before acq->Begin\n");
//	try
//	{
//		DigitalMicrograph::Result("In try, about to acq->beginacq\n");
//		acqsource->BeginAcquisition();
//		DigitalMicrograph::Result("After beginacqu\n");
//		/*}
//		catch (...)
//		{
//		DigitalMicrograph::Result("failed in ->begin\n");
//		short error;
//		long context;
//		DigitalMicrograph::GetException(&error, &context);
//		DigitalMicrograph::ErrorDialog(error);
//		DigitalMicrograph::Result("Couldn't Create Image for Acquire\n");
//		return;
//		}*/
//		DigitalMicrograph::Result("After acq->begin\n");
//		acqprmchanged = false;
//	
//		/*try
//		{*/
//		DigitalMicrograph::Result("Before create image for acquisition\n	");
//		AcquiredImage = Gatan::Camera::CreateImageForAcquire(acq, "Acquired Image");
//		DigitalMicrograph::Result("After create image for acquisition\n	");
//
//		/*}
//		catch (...)
//		{
//		DigitalMicrograph::Result("failed in try create image for acquire\n");
//		short error;
//		long context;
//		DigitalMicrograph::GetException(&error, &context);
//		DigitalMicrograph::ErrorDialog(error);
//		DigitalMicrograph::Result("Couldn't Create Image for Acquire\n");
//		return;
//		}*/
//		DigitalMicrograph::Result("about to try and acquire...\n");
//		//try
//		//{
//		DigitalMicrograph::Result("Before acquireto\n");
//		
//		clock_t start = clock();
//		if (!acqsource->AcquireTo(AcquiredImage, true, 0.5, acqprmchanged))
//		{
//			// Now wait for it to finish again but dont restart if it finishes durign call....
//			while (!acqsource->AcquireTo(AcquiredImage, false,/* 1*/0.5, acqprmchanged))
//			{
//				// Waiting for read to finish
//			}
//		}
//		clock_t finish = clock() - start;
//		DigitalMicrograph::Result("Acquisition time inside function = " + boost::lexical_cast<std::string>(finish)+" ticks\n");
//		DigitalMicrograph::Result("Acquisition time inside function = " + boost::lexical_cast<std::string>(((float)finish) / CLOCKS_PER_SEC) + " seconds\n");
//
//		DigitalMicrograph::Result("After acquireto\n");
//		//}
//		//catch (...)
//		//{
//		//	DigitalMicrograph::Result("failed in try acquire to\n");
//		//	short error;
//		//	long context;
//		//	DigitalMicrograph::GetException(&error, &context);
//		//	DigitalMicrograph::ErrorDialog(error);
//		//	DigitalMicrograph::OpenAndSetProgressWindow("Couldn't Acquire Image", "", "");
//		//	return;
//		//}
//		DigitalMicrograph::Result("Before finish acq\n");
//		acqsource->FinishAcquisition();
//		DigitalMicrograph::Result("finished acquisition in acquire function\n");
//		AcquiredInput = AcquiredImage;
//		return;
//	}
//	catch (...)
//	{
//		DigitalMicrograph::Result("Failed in acquisition\n");
//		return;
//	}
//}


//void Acquisition::AcquireImage(DigitalMicrograph::Image &AcquiredInput, Gatan::Camera::AcquisitionImageSourcePtr &acqsource, Gatan::CM::AcquisitionPtr &acq, bool &acqprmchanged)
//{
//	DigitalMicrograph::Image AcquiredImage;
//	DigitalMicrograph::Result("Before Acquire image\n");
//	try
//	{
//		Gatan::Camera::AcquireImage(camera)
//	}
//	catch (...)
//	{
//
//	}
//}


void Acquisition::CheckCamera()
{
	DigitalMicrograph::Result("In check camera\n");
	//Gatan::Camera::Camera camera;
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

void Acquisition::SetAcquireParameters()
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
	if (temp_kunprocessed_selected == true)
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


	DigitalMicrograph::Result("binning: " + boost::lexical_cast<std::string>(binning)+"\n");
	DigitalMicrograph::Result("expo: " + boost::lexical_cast<std::string>(expo)+"\n");

	Gatan::uint32 binx, biny;
	binx = (Gatan::uint32)binning;
	biny = (Gatan::uint32)binning;
	DigitalMicrograph::Result("About to try\n");
	try
	{
		//acqparams = Gatan::Camera::CreateAcquisitionParameters(camera, processing, expo, binx, biny, 0, 0, ypixels, xpixels);
		acqparams = Gatan::Camera::CreateAcquisitionParameters_FullCCD(camera, processing, expo, binx, biny);
		DigitalMicrograph::Result("binning: " + boost::lexical_cast<std::string>(binning)+"\n");
		DigitalMicrograph::Result("binning: " + boost::lexical_cast<std::string>(binx)+"\n");

		Gatan::CM::SetDoContinuousReadout(acqparams, true);
		DigitalMicrograph::Result("2\n");
		Gatan::CM::SetQualityLevel(acqparams, 0); // Can't remember if fast or slow :D
		DigitalMicrograph::Result("3\n");
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
	DigitalMicrograph::Result("5\n");
	acqtok = DigitalMicrograph::ScriptObjectProxy<Gatan::Camera::AcquisitionImp, DigitalMicrograph::DMObject>::to_object_token(acq.get());
	DigitalMicrograph::Result("6\n");
	try
	{
		fsi = Acquisition::GetFrameSetInfoPtr(acqtok);
		DigitalMicrograph::Result("7\n");


		acqsource = Gatan::Camera::AcquisitionImageSource::New(acq, fsi, 0);
		DigitalMicrograph::Result("8\n");
	}
	catch (...)
	{
		DigitalMicrograph::Result("Failed to get fsi/acqsource\n");
	}
}

void Acquisition::AcquireImage(DigitalMicrograph::Image &AcquiredInput)
{
	DigitalMicrograph::Image AcquiredImage;
	DigitalMicrograph::Result("Before acq->Begin\n");
	try
	{
		DigitalMicrograph::Result("In try, about to acq->beginacq\n");
		acqsource->BeginAcquisition();
		DigitalMicrograph::Result("After beginacqu\n");
		/*}
		catch (...)
		{
		DigitalMicrograph::Result("failed in ->begin\n");
		short error;
		long context;
		DigitalMicrograph::GetException(&error, &context);
		DigitalMicrograph::ErrorDialog(error);
		DigitalMicrograph::Result("Couldn't Create Image for Acquire\n");
		return;
		}*/
		DigitalMicrograph::Result("After acq->begin\n");
		acqprmchanged = false;

		/*try
		{*/
		DigitalMicrograph::Result("Before create image for acquisition\n	");
		AcquiredImage = Gatan::Camera::CreateImageForAcquire(acq, "Acquired Image");
		DigitalMicrograph::Result("After create image for acquisition\n	");

		/*}
		catch (...)
		{
		DigitalMicrograph::Result("failed in try create image for acquire\n");
		short error;
		long context;
		DigitalMicrograph::GetException(&error, &context);
		DigitalMicrograph::ErrorDialog(error);
		DigitalMicrograph::Result("Couldn't Create Image for Acquire\n");
		return;
		}*/
		DigitalMicrograph::Result("about to try and acquire...\n");
		//try
		//{
		DigitalMicrograph::Result("Before acquireto\n");

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
		DigitalMicrograph::Result("Acquisition time inside function = " + boost::lexical_cast<std::string>(finish)+" ticks\n");
		DigitalMicrograph::Result("Acquisition time inside function = " + boost::lexical_cast<std::string>(((float)finish) / CLOCKS_PER_SEC) + " seconds\n");

		DigitalMicrograph::Result("After acquireto\n");
		//}
		//catch (...)
		//{
		//	DigitalMicrograph::Result("failed in try acquire to\n");
		//	short error;
		//	long context;
		//	DigitalMicrograph::GetException(&error, &context);
		//	DigitalMicrograph::ErrorDialog(error);
		//	DigitalMicrograph::OpenAndSetProgressWindow("Couldn't Acquire Image", "", "");
		//	return;
		//}
		DigitalMicrograph::Result("Before finish acq\n");
		acqsource->FinishAcquisition();
		DigitalMicrograph::Result("finished acquisition in acquire function\n");
		AcquiredInput = AcquiredImage;
		//DigitalMicrograph::DeleteImage(AcquiredImage);
		return;
	}
	catch (...)
	{
		DigitalMicrograph::Result("Failed in acquisition\n");
		return;
	}
}


void Acquisition::AcquireImage2(DigitalMicrograph::Image &AcquiredInput)
{
	DigitalMicrograph::Image AcquiredImage;
	DigitalMicrograph::Result("Before acq->Begin\n");
	try
	{
		//DigitalMicrograph::Result("In try, about to acq->beginacq\n");
		//acqsource->BeginAcquisition();
		//DigitalMicrograph::Result("After acq->begin\n");
		acqprmchanged = false;
//		DigitalMicrograph::Result("Before create image for acquisition\n	");
		AcquiredImage = Gatan::Camera::CreateImageForAcquire(acq, "Acquired Image");
//		DigitalMicrograph::Result("After create image for acquisition\n	");

//		DigitalMicrograph::Result("Before acquireto\n");

		clock_t start = clock();
		if (!acqsource->AcquireTo(AcquiredImage, true, 0.5, acqprmchanged))
		{
			// Now wait for it to finish again but dont restart if it finishes durign call....
			while (!acqsource->AcquireTo(AcquiredImage, false,0.5, acqprmchanged))
			{
				// Waiting for read to finish
			}
		}
		clock_t finish = clock() - start;
//		DigitalMicrograph::Result("Acquisition time inside function = " + boost::lexical_cast<std::string>(finish)+" ticks\n");
		DigitalMicrograph::Result("Acquisition time inside function = " + boost::lexical_cast<std::string>(((float)finish) / CLOCKS_PER_SEC) + " seconds\n");
//		DigitalMicrograph::Result("After acquireto\n");
		//acqsource->FinishAcquisition();
//		DigitalMicrograph::Result("finished acquisition in acquire function\n");
		AcquiredInput = AcquiredImage;
		DigitalMicrograph::DeleteImage(AcquiredImage);
		return;
	}
	catch (...)
	{
		DigitalMicrograph::Result("Failed in acquisition\n");
		return;
	}
}
