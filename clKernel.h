#pragma once
#include "CL/OpenCl.h"
#include "DMPlugInStubs.h"

// Use this for specifying data type for some of my functions.
enum clTypes
{
	clFloat = 0,
	clFloat2 = 1,
	clDouble = 2,
	clDouble2 = 3
}; 

// Wrap OpenCl device information.
class clDevice
{
public:
	cl_uint numDevices;
	cl_device_id* devices;

	clDevice::clDevice(cl_uint numDevices, cl_device_id* devices);
	clDevice();
	~clDevice();
};

// Wrap OpenCl Command Queue.
class clQueue
{
public:
	cl_command_queue cmdQueue;
	cl_int status;

	//clQueue(cl_command_queue &cmdQueue);
	cl_int SetupQueue(cl_context &context,cl_device_id device);
	clQueue();
	~clQueue();

};

// This class wraps generation of an OpenCL kernel from a code string and will automatically produce output in the Digital Micrograph results window if something goes wrong.
class clKernel
{
public:
	clKernel(void);
	~clKernel(void);

	const char* kernelcode;
	cl_program kernelprogram;
	cl_int status;
	cl_context* context;
	cl_kernel kernel;
	std::string kernelname;
	size_t log;
	clDevice* cldev;
	clQueue* clq;

	//clKernel(const char* codestring, cl_context &context, cl_uint &numdevices, cl_device_id* &devices, std::string kernelname,cl_command_queue &commandQueue);
	clKernel(const char* codestring, cl_context &context, clDevice* cldev, std::string kernelname,clQueue* Queue);
	void BuildKernel();
	cl_int StatusOK();
	void Enqueue(size_t* globalWorkSize);
	void Enqueue3D(size_t* globalWorkSize);
	void Enqueue3D(size_t* globalWorkSize, size_t* localWorkSize);

	// Dont really need these with badass template :D
	//void SetArg(int position,cl_mem argument);
	//void SetArg(int position,int argument);
	//void SetArg(int position,float argument);

	// Function definition has to be in header for templates...
	// Sets arguments for clKernel
	template <class T> void SetArgT(int position, T &arg) 
	{
			status |= clSetKernelArg(kernel,position,sizeof(T),&arg);
	}

	// Use this if an argument is of type __local
	void SetArgLocalMemory(int position, int size, clTypes type) 
	{
		switch (type)
		{
		case clFloat:
			status |= clSetKernelArg(kernel,position,size*sizeof(cl_float),NULL);
			break;
		case clFloat2:
			status |= clSetKernelArg(kernel,position,size*sizeof(cl_float2),NULL);
			break;
		case clDouble:
			status |= clSetKernelArg(kernel,position,size*sizeof(cl_double),NULL);
			break;
		case clDouble2:
			status |= clSetKernelArg(kernel,position,size*sizeof(cl_double2),NULL);
			break;
		}
	}


};

class clVariables
{
private:
	bool gotContext;
	bool gotQueue;
	bool gotDevice;

	cl_context* context;
	clQueue* clq;
	clDevice* cldev;

public:
	clVariables();
	~clVariables();

	cl_int status; //////////////////////////all these are being defined by user
	cl_context			GetClContext();
	cl_context*			GetClContextPtr();
	cl_command_queue	GetCLCmdQueue();
	cl_command_queue*	GetClCmdQueuePtr();
	clQueue*			GetClQueue();
	clDevice*			GetClDevice();
	void				SetClContext(cl_context* context);
	void				SetClQueue(clQueue* clq);
	void				SetClDevice(clDevice* cldev);
};
