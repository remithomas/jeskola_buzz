
#include "MachineInterface.h"
#include "MDK.h"



CMachineParameter const paraRate = { pt_byte, "Rate", "Modulation rate", 0, 127, 255, MPF_STATE, 0 };
CMachineParameter const paraDepth = { pt_byte, "Depth", "Modulation depth", 0, 127, 255, MPF_STATE, 0 };

CMachineParameter const *pParameters[] = 
{ 
	// global
	&paraRate,
	&paraDepth
};


#pragma pack(1)		

class gvals
{
public:
	byte rate;
	byte depth;
};


#pragma pack()

CMachineInfo const MacInfo = 
{
	MT_EFFECT,								// type
	MI_VERSION,	
	MIF_DOES_INPUT_MIXING,					// flags
	0,										// min tracks
	0,										// max tracks
	2,										// numGlobalParameters
	0,										// numTrackParameters
	pParameters,
	0,
	NULL,
#ifdef _DEBUG
	"Jeskola AM 3000 (Debug build)",		// name
#else
	"Jeskola AM 3000",					// name
#endif
	"AM 3000",									// short name
	"Oskari Tammelin",						// author
	NULL
};


class miex : public CMDKMachineInterfaceEx
{

};

class mi : public CMDKMachineInterface
{
public:
	mi();
	virtual ~mi();

	virtual void Tick();

	virtual void MDKInit(CMachineDataInput * const pi);
	virtual bool MDKWork(float *psamples, int numsamples, int const mode);
	virtual bool MDKWorkStereo(float *psamples, int numsamples, int const mode);

	virtual void Command(int const i);

	
	virtual void MDKSave(CMachineDataOutput * const po);


public:
	virtual CMDKMachineInterfaceEx *GetEx() { return &ex; }
	virtual void OutputModeChanged(bool stereo) {}


public:
	miex ex;


public:
	
	
	dword Phase;

	
	gvals gval;

};


mi::mi()
{
	GlobalVals = &gval;
}


mi::~mi()
{

}





#define SINETABLE_BITS	11
#define SINETABLE_SIZE	(1 << SINETABLE_BITS)
static short const *SineTable;

inline short fastsin(dword const phase)
{
	int i0 = (phase >> (32 - SINETABLE_BITS));
	short s0 = SineTable[i0];
	short s1 = SineTable[(i0 + 1) & (SINETABLE_SIZE - 1)];

	double a = (phase & ((1 << (32 - SINETABLE_BITS)) - 1)) * (1.0 / (1 << (32 - SINETABLE_BITS)));

	return s0 + a * (s1 - s0);
}


void mi::MDKInit(CMachineDataInput * const pi)
{



	Phase = 0;
	SineTable = pCB->GetOscillatorTable(OWF_SINE);
}

void mi::MDKSave(CMachineDataOutput * const po)
{

}


void mi::Tick()
{
} 


bool mi::MDKWork(float *psamples, int numsamples, int const mode)
{
	if (!(mode & WM_READ) || !(mode & WM_WRITE))
	{
	
		return false;
	}
	
	do
	{
		double s = *psamples;

		s *= 1.0 - (fastsin(Phase) + 32768) * (0.9 / 65536.0);

		Phase += 65536*4;
		

		*psamples++ = (float)s;

	} while(--numsamples);

	return true;
}

bool mi::MDKWorkStereo(float *psamples, int numsamples, int const mode)
{
	if (!(mode & WM_READ) || !(mode & WM_WRITE))
	{
	
		return false;
	}
	
	do
	{
		double l = psamples[0];
		double r = psamples[1];

		l *= 1.0 - (fastsin(Phase) + 32768) * (0.9 / 65536.0);
		r *= 1.0 - (fastsin(Phase*1.1) + 32768) * (0.9 / 65536.0);

		Phase += 65536*9;
		

		psamples[0] = (float)l;
		psamples[1] = (float)r;
		psamples += 2;

	} while(--numsamples);


	return true;
}




void mi::Command(int const i)
{
}


 

DLL_EXPORTS

