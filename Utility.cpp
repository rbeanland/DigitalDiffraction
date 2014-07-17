#include "StdAfx.h"
#include "Utility.h"


Utility::Utility(void)
{
}


Utility::~Utility(void)
{
}

void Utility::PrintCLMemToImage(cl_mem buffer,std::string name, int width, int height, clTypes type, clQueue* clq)
{
	DM::Image Test;

	CString cname = name.c_str();

	if(type == clFloat)
	{
		Test = DM::RealImage(cname,4,width,height);
	}
	else if (type == clFloat2)
	{
		Test = DM::ComplexImage(cname,8,width,height);
	}

	Gatan::PlugIn::ImageDataLocker locker(Test);

	if(type == clFloat)
	{
		std::vector<float> host (width*height);
		clEnqueueReadBuffer(clq->cmdQueue,buffer,CL_TRUE,0,width*height*sizeof(cl_float),&host[0],0,NULL,NULL);

		float* data =(float*) locker.get();

		for(int i = 0 ; i< width*height ; i++)
		{
			data[i] = host[i];
		}

		host.clear();
		locker.~ImageDataLocker();

		Test.GetOrCreateImageDocument().Show();
	}
	else if (type == clFloat2)
	{
		std::vector<cl_float2> host (width*height);
		clEnqueueReadBuffer(clq->cmdQueue,buffer,CL_TRUE,0,width*height*sizeof(cl_float2),&host[0],0,NULL,NULL);

		cl_float2* data =(cl_float2*) locker.get();

		for(int i = 0 ; i< width*height ; i++)
		{
			data[i].s[0] = host[i].s[0];
			data[i].s[1] = host[i].s[1];
		}

		host.clear();
		locker.~ImageDataLocker();
		Test.GetOrCreateImageDocument().Show();
	}


}