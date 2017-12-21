

#include "stdafx.h"
#include "MachineInterface.h"


namespace groove1
{

CMachineParameter const paraSwing8th = 
{ 
	pt_byte,										// type
	"Swing 8th",
	"Swing 8th",							// description
	0,												// MinValue	
	127,											// MaxValue
	255,												// NoValue
	MPF_STATE,										// Flags
	0
};


CMachineParameter const paraSwing16th = 
{ 
	pt_byte,										// type
	"Swing 16th",
	"Swing 16th",							// description
	0,												// MinValue	
	127,											// MaxValue
	255,												// NoValue
	MPF_STATE,										// Flags
	0
};

CMachineParameter const paraTrigger = 
{ 
	pt_switch,										// type
	"Trigger",
	"Trigger",							// description
	SWITCH_OFF,												// MinValue	
	SWITCH_ON,											// MaxValue
	SWITCH_NO,												// NoValue
	0,										// Flags
	0
};

static CMachineParameter const *pParameters[] = { 
	// track
	&paraSwing8th,
	&paraSwing16th,
	&paraTrigger
};

#pragma pack(1)

class gvals
{
public:
	byte swing8th;
	byte swing16th;
	byte trigger;

};

#pragma pack()

CMachineInfo const MacInfo = 
{
	MT_GENERATOR,							// type
	MI_VERSION,
	MIF_NO_OUTPUT | MIF_CONTROL_MACHINE | MIF_GROOVE_CONTROL,		// flags
	0,											// min tracks
	0,								// max tracks
	3,										// numGlobalParameters
	0,										// numTrackParameters
	pParameters,
	0, 
	NULL,
	"Jeskola Groove 1",
	"Groove 1",								// short name
	"Oskari Tammelin", 						// author
	NULL
};

class mi;

class mi : public CMachineInterface
{
public:
	mi();
	virtual ~mi();

	virtual void Init(CMachineDataInput * const pi);
	virtual void Tick();
	virtual bool Work(float *psamples, int numsamples, int const mode);

public:
	float swing8th;
	float swing16th;
	gvals gval;

};


mi::mi()
{
	GlobalVals = &gval;
	TrackVals = NULL;
	AttrVals = NULL;
}

mi::~mi()
{

}

void mi::Init(CMachineDataInput * const pi)
{
}


void mi::Tick()
{
	if (gval.swing8th != paraSwing8th.NoValue) swing8th = gval.swing8th;
	if (gval.swing16th != paraSwing16th.NoValue) swing16th = gval.swing16th;

	if (gval.trigger == SWITCH_ON)
	{
		float s8 = 1.0f + swing8th * 2.0f / 127.0f;
		float s16 = 1.0f + swing16th * 2.0f / 127.0f;

		float groove[4] = { s8 * s16, s8, s16, 1 };
		pCB->SetGroovePattern(groove, 4);
	}
	else if (gval.trigger == SWITCH_OFF)
	{
		pCB->SetGroovePattern(NULL, 0);
	}
}


bool mi::Work(float *psamples, int numsamples, int const)
{
	return false;
}

 


}

DLL_EXPORTS_NS(groove1, InitGroove1)
