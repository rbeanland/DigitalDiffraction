// TemplateMFCDialogPlugIn.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "TemplateMFCDialogPlugIn.h"
#include "MyTabCtrl.h"

#define _GATAN_USE_STL_STRING

#include "DMPluginMain.h"
extern AFX_EXTENSION_MODULE gPlugInExtensionModule = { NULL, NULL };

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// Here we set the name of the Plugins windows in DM and its size (3 for right dock, 2 for left dock)
void CTemplateMFCDialogPlugInApp::Start(void)
{
	fMFCPaletteHandle1=RegisterMFCPalette("D-ED_V1", GetPlugInOSHandle(), RUNTIME_CLASS(CDMDialog),3);
}
void CTemplateMFCDialogPlugInApp::End(void)
{
	UnregisterMFCPalette(fMFCPaletteHandle1);
}

// The one and only CTemplateMFCDialogPlugInApp object

CTemplateMFCDialogPlugInApp theApp;

