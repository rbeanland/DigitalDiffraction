#include "StdAfx.h"
#include "clKernel.h"
#include "standardfunctions.h"


clKernel::clKernel(void)
{
}


clKernel::~clKernel(void)
{
	clReleaseProgram(kernelprogram);
	clReleaseKernel(kernel);
}

// Constructor that sets all Command Queues and Contexts etc..
clKernel::clKernel(const char* codestring, cl_context &context, clDevice* cldev, std::string kernelname,clQueue* Queue)
{
	this->cldev = cldev;
	this->context = &context;
	this->kernelcode = codestring;
	this->kernelname = kernelname;
	this->clq = Queue;
}

void clKernel::BuildKernel()
{
	this->kernelprogram = clCreateProgramWithSource(*context,1,&kernelcode,NULL,&status);

	if(!status==0)
	{
		DigitalMicrograph::Result("Problem with " + kernelname + " source "+t_to_string(status));
		DigitalMicrograph::Result("\n");
		return;
	}

	this->status = clBuildProgram(kernelprogram,cldev->numDevices,cldev->devices,NULL,NULL,NULL);
	
	clGetProgramBuildInfo(kernelprogram, NULL, CL_PROGRAM_BUILD_LOG, 0, NULL, &log);
	char *buildlog = (char*)malloc(log*sizeof(char));
	clGetProgramBuildInfo(kernelprogram, NULL, CL_PROGRAM_BUILD_LOG, log, buildlog, NULL);

	if(!status==0)
	{
		DigitalMicrograph::Result("Problem with " + kernelname + " build " +t_to_string(status));
		DigitalMicrograph::Result("\n");
		DigitalMicrograph::Result(buildlog);
		DigitalMicrograph::Result("\n");
		return;
	}

	free(buildlog);

	this->kernel = clCreateKernel(kernelprogram,kernelname.c_str(),&status);

	if(!status==0)
	{
		DigitalMicrograph::Result("Problem with " + kernelname + " kernel "+t_to_string(status));
		DigitalMicrograph::Result("\n");
		return;
	}

}

// 0 is success
cl_int clKernel::StatusOK()
{
	if(status != CL_SUCCESS)
	{

		return status;
	}
	else return status;
}

void clKernel::Enqueue(size_t* globalWorkSize )
{
	status = clEnqueueNDRangeKernel(clq->cmdQueue,kernel,2,NULL,globalWorkSize,NULL,0,NULL,NULL);
}

void clKernel::Enqueue3D(size_t* globalWorkSize, size_t* localWorkSize )
{
	status = clEnqueueNDRangeKernel(clq->cmdQueue,kernel,3,NULL,globalWorkSize,localWorkSize,0,NULL,NULL);
}
void clKernel::Enqueue3D(size_t* globalWorkSize )
{
	status = clEnqueueNDRangeKernel(clq->cmdQueue,kernel,3,NULL,globalWorkSize,NULL,0,NULL,NULL);
}


clDevice::clDevice(cl_uint numDevices, cl_device_id* devices)
{
	this->numDevices = numDevices;
	this->devices = devices;
}

cl_int clQueue::SetupQueue(cl_context &context, cl_device_id device)
{
	this->cmdQueue = clCreateCommandQueue(context,device,0,&status);

	if(!status==CL_SUCCESS)
	{
		DigitalMicrograph::Result("Problem with Command Queue generation \n");
		return status;
	}


	return status;
}

clQueue::clQueue(void)
{
}

clVariables::clVariables()
{
	gotContext = false;
	gotDevice = false;
	gotQueue = false;
};

clVariables::~clVariables()
{
};

cl_context clVariables::GetClContext()
{
	if (gotContext) {
		return *context;
	}
	else {
		DigitalMicrograph::Result("Trying to access clContext without setting it first \n");
		exit(1);
	}
};

cl_context* clVariables::GetClContextPtr()
{
	if (gotContext) {
		return context;
	}
	else {
		DigitalMicrograph::Result("Trying to access clContext without setting it first \n");
		exit(1);
	}
};

cl_command_queue clVariables::GetCLCmdQueue()
{
	if (gotQueue) {
		return clq->cmdQueue;
	}
	else {
		DigitalMicrograph::Result("Trying to access clQueue without setting it first \n");
		exit(1);
	}
}

cl_command_queue* clVariables::GetClCmdQueuePtr()
{
	if (gotQueue) {
		return &(clq->cmdQueue);
	}
	else {
		DigitalMicrograph::Result("Trying to access clQueue without setting it first \n");
		exit(1);
	}
}

clQueue* clVariables::GetClQueue()
{
	if (gotQueue) {
		return clq;
	}
	else {
		DigitalMicrograph::Result("Trying to access clQueue without setting it first \n");
		exit(1);
	}
};

clDevice* clVariables::GetClDevice()
{
	if (gotDevice) {
		return cldev;
	}
	else {
		DigitalMicrograph::Result("Trying to access clQueue without setting it first \n");
		exit(1);
	}
};

void clVariables::SetClDevice(clDevice* cldev)
{
	this->cldev = cldev;
	gotDevice = true;
}

void clVariables::SetClContext(cl_context* context)
{
	this->context = context;
	gotContext = true;
}

void clVariables::SetClQueue(clQueue* clq)
{
	this->clq = clq;
	gotQueue = true;
}