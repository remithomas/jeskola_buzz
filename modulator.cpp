
#include "stdafx.h"
#include "MachineInterface.h"

namespace modulator
{

#define INPUT_COUNT		2
#define IN_INPUT		0
#define	IN_MOD			1

#define OUTPUT_COUNT	4
#define OUT_PRODUCT		0
#define OUT_AND			1
#define OUT_OR			2
#define OUT_XOR			3

CMachineParameter const paraLevel = { pt_byte, "Level", "Level", 0, 127, 255, MPF_STATE, 0 };


static CMachineParameter const *pParameters[] = { 
	// track
	&paraLevel,
};

#pragma pack(1)

class gvals
{
public:
	byte level;
};

#pragma pack()

CMachineInfo const MacInfo = 
{
	MT_EFFECT,							// type
	MI_VERSION,
	MIF_MULTI_IO,							// flags
	0,											// min tracks
	0,								// max tracks
	1,										// numGlobalParameters
	0,										// numTrackParameters
	pParameters,
	0, 
	NULL,
	"Jeskola Modulator",
	"Modulator",								// short name
	"Oskari Tammelin", 						// author
	NULL
};

class mi;

class miex : public CMachineInterfaceEx
{
public:
	virtual void MultiWork(float const * const *inputs, float **outputs, int numsamples);
	virtual char const *GetChannelName(bool input, int index)
	{
		if (input)
		{
			switch(index)
			{
			case IN_INPUT: return "Input"; break;
			case IN_MOD: return "Modulator"; break;
			default: return "<invalid>"; break;
			}
		}
		else
		{
			switch(index)
			{
			case OUT_PRODUCT: return "Product"; break;
			case OUT_AND: return "AND"; break;
			case OUT_OR: return "OR"; break;
			case OUT_XOR: return "XOR"; break;
			default: return "<invalid>"; break;
			}
		}
	}

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
	virtual void Save(CMachineDataOutput * const po);

	void MultiWork(float const * const *inputs, float **outputs, int numsamples);

public:
	miex ex;
	gvals gval;

};

void miex::MultiWork(float const * const *inputs, float **outputs, int numsamples) { pmi->MultiWork(inputs, outputs, numsamples); }


mi::mi()
{
	ex.pmi = this;
	GlobalVals = &gval;
	TrackVals = NULL;
	AttrVals = NULL;
}

mi::~mi()
{
}

void mi::Init(CMachineDataInput * const pi)
{
	pCB->SetMachineInterfaceEx(&ex);
	pCB->SetInputChannelCount(INPUT_COUNT);
	pCB->SetOutputChannelCount(OUTPUT_COUNT);
}

void mi::Save(CMachineDataOutput * const po)
{
}

void mi::Tick()
{

}

void mi::MultiWork(float const * const *inputs, float **outputs, int numsamples)
{
	if (inputs[IN_INPUT] == NULL || inputs[IN_MOD] == NULL)
	{
		// setting an output pointer to NULL is equivalent of returning false in Work
		for (int i = 0; i < OUTPUT_COUNT; i++) outputs[i] = NULL;
		return;
	}

	float2 * __restrict pin = (float2 *)inputs[IN_INPUT];
	float2 * __restrict pmod = (float2 *)inputs[IN_MOD];

	if (outputs[OUT_PRODUCT] != NULL)
	{
		float2 * __restrict pout = (float2 *)outputs[OUT_PRODUCT];
		for (int i = 0; i < numsamples; i++) pout[i] = pin[i] * pmod[i] * (1.0 / 32768.0f);
	}

	if (outputs[OUT_AND] != NULL)
	{
		float2 * __restrict pout = (float2 *)outputs[OUT_AND];
		for (int i = 0; i < numsamples; i++) pout[i] = int_and(pin[i], pmod[i]);		// int_and = (float)((int)a & (int)b)
	}

	if (outputs[OUT_OR] != NULL)
	{
		float2 * __restrict pout = (float2 *)outputs[OUT_OR];
		for (int i = 0; i < numsamples; i++) pout[i] = int_or(pin[i], pmod[i]);		
	}

	if (outputs[OUT_XOR] != NULL)
	{
		float2 * __restrict pout = (float2 *)outputs[OUT_XOR];
		for (int i = 0; i < numsamples; i++) pout[i] = int_xor(pin[i], pmod[i]);		
	}

}



} DLL_EXPORTS_NS(modulator, InitModulator)
