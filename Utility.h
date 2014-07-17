#pragma once
#include "boost/lexical_cast.hpp"
#include "CL/OpenCl.h"
#include "clKernel.h"
#include "stdafx.h"


// A Class containing a few useful functions for getting values from dialogs, or updating parts of the DM UI like progress or results windows.
// Also contains a function that takes an OpenCL memory buffer (cl_mem) and displays it as an image in Digital Micrograph for easy testing.

class Utility
{
public:
	Utility(void);
	~Utility(void);

	template <class T> static bool GetDialogString(T &arg, CString variable) 
	{
		try
		{
			arg = boost::lexical_cast<T>(variable);
		}
		catch(...)
		{
			DigitalMicrograph::Result("Couldn't parse " + variable + "\n");
			return false;
		}
		return true;
	}

	static void SetProgressWindow(std::string s1, std::string s2, std::string s3)
	{
		DigitalMicrograph::OpenAndSetProgressWindow(s1.c_str(),s2.c_str(),s3.c_str());
	}
	// Outputs string to results window, automatically includes endline.
	static void SetResultWindow(std::string s1)
	{
		std::string res = s1+"\n";
		DigitalMicrograph::Result(res.c_str());
	}

	static void PrintCLMemToImage(cl_mem buffer,std::string name, int width, int height, clTypes type, clQueue* clq);

};

