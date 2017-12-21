#pragma once

#include "../../buzz/MachineInterface.h"

typedef vector<float> FloatVector;

int const ValuesPerTick = 48;
int const NoteCount = 12 * 4;
int const FirstNote = 60 - 24;

class CMachinePattern
{
public:
	CMachinePattern()
	{
		pPattern = NULL;
	}

	CMachinePattern(CPattern *p, int numrows)
	{
		pPattern = p;
		SetLength(numrows);
	}

	CMachinePattern(CPattern *p, CMachinePattern *pold)
	{
		pPattern = p;
		data.resize(pold->data.size());
		copy(pold->data.begin(), pold->data.end(), data.begin());
	}

	void SetLength(int length)
	{
		data.resize(ValuesPerTick * length);
	}

	void Write(CMachineDataOutput * const po)
	{
		int datasize = (int)data.size();
		po->Write(datasize);
		if (datasize > 0)
			po->Write(&data[0], data.size() * sizeof(float));
	}

	void Read(CMachineDataInput * const pi)
	{
		int datasize;
		pi->Read(datasize);
		data.resize(datasize);
		if (datasize > 0)
			pi->Read(&data[0], datasize * sizeof(float));
	}

	
public:
	CPattern *pPattern;
	FloatVector data;
};

typedef map<CPattern *, shared_ptr<CMachinePattern>> MapPatternToMachinePattern;
typedef map<CString, shared_ptr<CMachinePattern>> MapStringToMachinePattern;


