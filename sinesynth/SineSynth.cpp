#include "stdafx.h"
#include "../../buzz/MachineInterface.h"
#include "PatEd.h"

class SmoothParameter
{
public:
	SmoothParameter(double v, double sc)
	{
		target = value = v;
		coeff = sc;
	}

	__forceinline void Update()
	{
		if (fabs(target - value) < 0.0000000001)
			value = target;
		else
			value += coeff * (target - value);
	}

	double value;
	double target;
	double coeff;

};

CMachineParameter const paraDummy = 
{ 
	pt_byte,										// type
	"Dummy",
	"Dummy",							// description
	0,												// MinValue	
	127,											// MaxValue
	255,												// NoValue
	MPF_STATE,										// Flags
	0
};


static CMachineParameter const *pParameters[] = { 
	// track
	&paraDummy,
};

#pragma pack(1)

class gvals
{
public:
	byte dummy;

};

#pragma pack()

CMachineInfo const MacInfo = 
{
	MT_GENERATOR,							// type
	MI_VERSION,
	MIF_PATTERN_EDITOR | MIF_DRAW_PATTERN_BOX,		// flags
	0,											// min tracks
	0,								// max tracks
	1,										// numGlobalParameters
	0,										// numTrackParameters
	pParameters,
	0, 
	NULL,
	"Jeskola Sine Synth",
	"SineSynth",								// short name
	"Oskari Tammelin", 						// author
	NULL
};

class mi;

class miex : public CMachineInterfaceEx
{
public:
	virtual void *CreatePatternEditor(void *parenthwnd);
	virtual void CreatePattern(CPattern *p, int numrows);
	virtual void CreatePatternCopy(CPattern *pnew, CPattern const *pold);
	virtual void DeletePattern(CPattern *p);
	virtual void RenamePattern(CPattern *p, char const *name);
	virtual void SetPatternLength(CPattern *p, int length);
	virtual void PlayPattern(CPattern *p, CSequence *s, int offset);
	virtual void SetEditorPattern(CPattern *p);
	virtual void DrawPatternBox(CDrawPatternBoxContext *ctx);

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
	virtual void Save(CMachineDataOutput * const po);

public:
	miex ex;
	MapPatternToMachinePattern patterns;
	MapStringToMachinePattern loadedPatterns;
	CMachinePattern *pPlayingPattern;
	int patternPos;
	int posInTick;

	double phase;
	SmoothParameter freq;
	SmoothParameter amp;
	CPatEd patEd;

	gvals gval;

};

DLL_EXPORTS

mi::mi()
: freq(0, 0.01), amp(0, 0.01)
{
	ex.pmi = this;
	GlobalVals = &gval;
	TrackVals = NULL;
	AttrVals = NULL;
	pPlayingPattern = NULL;
	phase = 0;
}

mi::~mi()
{

}

void mi::Init(CMachineDataInput * const pi)
{
	pCB->SetMachineInterfaceEx(&ex);
	patEd.pCB = pCB;

	if (pi != NULL)
	{
		byte version;
		pi->Read(version);
		if (version != 1)
		{
			AfxMessageBox("invalid data");
			return;
		}

		int numpat;
		pi->Read(numpat);

		for (int i = 0; i < numpat; i++)
		{
			CString name;

			while(true)
			{
				char ch;
				pi->Read(ch);
				if (ch == 0)
					break;
				name += ch;
			}

			shared_ptr<CMachinePattern> p(new CMachinePattern());
			p->Read(pi);
			loadedPatterns[name] = p;
		}
	}
	

}

void mi::Save(CMachineDataOutput * const po)
{
	byte version = 1;
	po->Write(version);
	po->Write((int)patterns.size());

	for (MapPatternToMachinePattern::iterator i = patterns.begin(); i != patterns.end(); i++)
	{
		char const *name = pCB->GetPatternName((*i).first);
		po->Write(name);
		(*i).second->Write(po);
	}
}

void *miex::CreatePatternEditor(void *parenthwnd)
{
	pmi->patEd.Create(CWnd::FromHandle((HWND)parenthwnd));
	return pmi->patEd.GetSafeHwnd();
}

void miex::CreatePattern(CPattern *p, int numrows)
{
	MapStringToMachinePattern::iterator i = pmi->loadedPatterns.find(pmi->pCB->GetPatternName(p));
	if (i != pmi->loadedPatterns.end())
	{
		(*i).second->pPattern = p;
		pmi->patterns[p] = (*i).second;
		pmi->loadedPatterns.erase(i);
	}
	else
	{
		pmi->patterns[p] = shared_ptr<CMachinePattern>(new CMachinePattern(p, numrows));
	}
}

void miex::CreatePatternCopy(CPattern *pnew, CPattern const *pold)
{
	pmi->patterns[pnew] = shared_ptr<CMachinePattern>(new CMachinePattern(pnew, pmi->patterns[(CPattern *)pold].get()));
}

void miex::DeletePattern(CPattern *p)
{
	if (pmi->pPlayingPattern == pmi->patterns[p].get())
		pmi->pPlayingPattern = NULL;

	pmi->patterns.erase(pmi->patterns.find(p));
}

void miex::RenamePattern(CPattern *p, char const *name)
{
	// this is only needed if you want to display the name
}

void miex::SetPatternLength(CPattern *p, int length)
{
	pmi->patterns[p]->SetLength(length);
	pmi->patEd.PatternLengthChanged(pmi->patterns[p].get());
}

void miex::PlayPattern(CPattern *p, CSequence *s, int offset)
{
	pmi->pPlayingPattern = p != NULL ? pmi->patterns[p].get() : NULL;
	pmi->patternPos = offset - 1;
}


void miex::SetEditorPattern(CPattern *p)
{
	if (p == NULL)
		pmi->patEd.SetPattern(NULL);
	else
		pmi->patEd.SetPattern(pmi->patterns.find(p)->second.get());
}

void mi::Tick()
{
	if (pPlayingPattern != NULL)
	{
		posInTick = 0;
		patternPos++;
		if (patternPos >= (int)pPlayingPattern->data.size() / ValuesPerTick)
		{
			patternPos = -1;
			pPlayingPattern = NULL;
		}
	}
}

bool mi::Work(float *psamples, int numsamples, int const)
{
	patEd.SetPlayPos(pPlayingPattern, patternPos * ValuesPerTick + posInTick * ValuesPerTick / pMasterInfo->SamplesPerTick);

	if (pPlayingPattern != NULL && patternPos >= (int)pPlayingPattern->data.size() / ValuesPerTick)
	{
		patternPos = -1;
		pPlayingPattern = NULL;
	}


	bool ret = false;

	for (int i = 0; i < numsamples; i++)
	{
		float data = 0;

		if (pPlayingPattern != NULL && patternPos >= 0)
		{
			int dataindex = patternPos * ValuesPerTick + posInTick * ValuesPerTick / pMasterInfo->SamplesPerTick;
			data = pPlayingPattern->data[dataindex];
		}

		if (data != 0)
		{
			amp.target = 32767.0f;
			freq.target = 2.0 * PI * 440.0 * pow(2.0, (data - 69.0) / 12.0) / pMasterInfo->SamplesPerSec;
		}
		else
		{
			amp.target = 0;
		}

		if (amp.value != 0)
		{
			psamples[i] = (float)(sin(phase) * amp.value);
			ret = true;
		}
		else
		{
			ret = false;
		}

		phase += freq.value;
		freq.Update();
		amp.Update();
		posInTick++;

	}


	return ret;
}

void miex::DrawPatternBox(CDrawPatternBoxContext *ctx)
{
	CMachinePattern *p = pmi->patterns[ctx->Pattern].get();

	COLORREF bgcol = pmi->pCB->GetThemeColor("SE Pattern Box");
	COLORREF col = Blend(bgcol, RGB(0, 0, 0), 0.75f);


	CDC dc;
	dc.Attach((HDC)ctx->HDC);

	float pos = 0;

	for (int y = 0; y < ctx->Height; y++)
	{
		float nextpos = pos + ctx->TicksPerPixel * ValuesPerTick;
		float mini = 999, maxi = -999;

		for (int i = (int)pos; i <= (int)nextpos; i++)
		{
			if (i < (int)p->data.size() && p->data[i] != 0.0f)
			{
				mini = min(mini, p->data[i]);
				maxi = max(maxi, p->data[i]);
			}
		}

		if (maxi >= 0)
		{
			mini = min(max(mini, FirstNote), FirstNote + NoteCount);
			maxi = min(max(maxi, FirstNote), FirstNote + NoteCount);
			int left = ctx->Left + (int)((mini - FirstNote) * (ctx->Width - 1) / NoteCount);
			int right = ctx->Left + (int)((maxi - FirstNote) * (ctx->Width - 1) / NoteCount);
			dc.FillSolidRect(left, y + ctx->Top, right - left + 1, 1, col);
		}

		pos = nextpos;
	}


	dc.Detach();
}
