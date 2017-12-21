
#define _USE_MATH_DEFINES

#include <Windows.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "../../buzz/MachineInterface.h"
//#include "GUI.h"
#include "Limiter.h"

inline double DBToAmplitude(double const a)
{
	return pow(10, (a * 0.05));
}

inline double ExpIntp(double const x, double const y, double const a)
{
        assert((x > 0 && y > 0) || (x < 0 && y < 0));
        double const lx = log(x);
        double const ly = log(y);
        return exp(lx + a * (ly - lx));
}


CMachineParameter const paraThreshold = { pt_byte, "Threshold", "Threshold", 0,	127, 255, MPF_STATE, 90 };
CMachineParameter const paraCeiling = { pt_byte, "Ceiling", "Ceiling", 0, 127, 255, MPF_STATE, 127 };
CMachineParameter const paraRelease = { pt_byte, "Release", "Release", 0, 127, 255, MPF_STATE, 80 };
CMachineParameter const paraFilterType = { pt_byte, "Filter Type", "Filter Type", 0, 5, 255, MPF_STATE, 0 };
CMachineParameter const paraLookahead = { pt_byte, "Look-ahead", "Look-ahead", 0, 127, 255, MPF_STATE, 127 };
CMachineParameter const paraReleaseShape = { pt_byte, "Rel. Shape", "Rel. Shape", 0, 1, 255, MPF_STATE, 0 };

CMachineParameter const *pParameters[] = { &paraThreshold, &paraCeiling, &paraRelease, &paraFilterType, &paraLookahead, &paraReleaseShape };

#pragma pack(1)		

class gvals
{
public:
	byte threshold;
	byte ceiling;
	byte release;
	byte filtertype;
	byte lookahead;
	byte releaseshape;
};

#pragma pack()

CMachineInfo const MacInfo = 
{
	MT_EFFECT,								// type
	MI_VERSION,	
	MIF_STEREO_EFFECT,						// flags
	0,										// min tracks
	0,										// max tracks
	6,										// numGlobalParameters
	0,										// numTrackParameters
	pParameters,
	0,
	NULL,
#ifdef _DEBUG
	"Jeskola Limiter (Debug build)",		// name
#else
	"Jeskola Limiter",					// name
#endif
	"Limiter",									// short name
	"Oskari Tammelin",						// author
	NULL
};

class mi;

class miex : public CMachineInterfaceEx
{
public:
	virtual int GetLatency();
	virtual bool HandleGUIMessage(CMachineDataOutput *pout, CMachineDataInput *pin);

public:
	mi *pmi;

};

class mi : public CMachineInterface
{
public:
	mi();
	virtual ~mi();

	virtual void Init(CMachineDataInput * const pi);
	virtual void Tick();
	virtual bool Work(float *psamples, int numsamples, int const mode);
	virtual char const *DescribeValue(int const param, int const value);

private:
			
	

public:
	miex ex;
	Limiter limiter;

	float lastmingain;
	bool IdleMode;
	int IdleCount;
	gvals gval;

};



int miex::GetLatency()
{
	return pmi->limiter.GetLatency();
}


DLL_EXPORTS

mi::mi()
{
	ex.pmi = this;
	GlobalVals = &gval;
	AttrVals = NULL;
}

mi::~mi()
{
	
		
}


void mi::Init(CMachineDataInput * const pi)
{
	pCB->SetMachineInterfaceEx(&ex);

	lastmingain = 0;
	IdleMode = true;
	IdleCount = 0;
}

char const *mi::DescribeValue(int const param, int const value)
{
	static char txt[16];

	switch(param)
	{
	case 0:        
	case 1:        
		sprintf_s(txt, 16, "%.1fdB", 30.0 * (value / 127.0 - 1));
		break;
	
	case 2:       
		sprintf_s(txt, 16, "%.1fms", ExpIntp(0.01, 1000.0, value / 127.0));
		break;

	case 3:
		switch(value)
		{
		case 0: return "sin64";
		case 1: return "lin64";
		case 2: return "sin32";
		case 3: return "lin32";
		case 4: return "silly";
		case 5: return "off";
		default:
			return NULL;
		};
		break;

	case 4:
		sprintf_s(txt, 16, "%.1f%%", 100.0 * value / 127.0);
		break;

	case 5:
		switch(value)
		{
		case 0: return "lin";
		case 1: return "exp";
		default:
			return NULL;
		}
		break;

		

	default:
		return NULL;
	}

	return txt;
}

bool miex::HandleGUIMessage(CMachineDataOutput *pout, CMachineDataInput *pin)
{
	int request;
	pin->Read(request);

	switch(request)
	{
	case 0:
		{
			float *mingain = pmi->limiter.GetMinGainPtr();

			float unchanged = 123.0f;

			if (*mingain != unchanged)
			{
				pmi->lastmingain = *mingain;
				*mingain = unchanged;
			}

			pout->Write((double)pmi->lastmingain);

			return true;
		}
	}

	return false;
}


void mi::Tick()
{
	if (gval.threshold != paraThreshold.NoValue)
		limiter.SetThreshold((float)DBToAmplitude(30.0 * (gval.threshold / 127.0 - 1.0)));

	if (gval.ceiling != paraCeiling.NoValue)
		limiter.SetCeiling((float)DBToAmplitude(30.0 * (gval.ceiling / 127.0 - 1.0)));

	if (gval.release != paraRelease.NoValue)
		limiter.SetRelease((float)ExpIntp(0.01, 1000.0, gval.release / 127.0) * pMasterInfo->SamplesPerSec / 1000.0f);

	if (gval.filtertype != paraFilterType.NoValue)
		limiter.SetFilterType(gval.filtertype);

	if (gval.lookahead != paraLookahead.NoValue)
		limiter.SetLookahead(gval.lookahead);

	if (gval.releaseshape != paraReleaseShape.NoValue)
		limiter.SetReleaseShape(gval.releaseshape);

}


bool mi::Work(float *psamples, int numsamples, int const mode)
{
	if (mode & WM_READ)
	{
		 IdleMode = false;
		 IdleCount = 0;
	}
	else
	{
		 if (IdleMode)
		 {
			 limiter.Idle();
			 return false;
		 }
		 else
		 {
			 IdleCount += numsamples;
			 if (IdleCount >= 1024)
				 IdleMode = true;

			 memset(psamples, 0, 2 * sizeof(float) * numsamples);
		 }
	}

	limiter.Process((sample *)psamples, numsamples);

	return true;
}

