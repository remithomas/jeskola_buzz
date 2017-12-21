// SineSynth.h : main header file for the SineSynth DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CSineSynthApp
// See SineSynth.cpp for the implementation of this class
//

class CSineSynthApp : public CWinApp
{
public:
	CSineSynthApp();


// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
