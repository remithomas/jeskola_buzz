#pragma once

#include "VisualWaveform.h"
#include "MachinePattern.h"
#include "Engine.h"

using namespace System;
using namespace System::Windows::Controls;
using namespace System::Windows::Threading;
using namespace System::Windows::Interop;
using namespace System::Runtime::InteropServices; 
using namespace Sikaloops::GUI;

ref class ManagedGUI : public IGUICallbacks
{
public:
	ManagedGUI(Engine *peng)
	{
		pEngines = peng;
		pCB = pEngines->pCB;
		PlayPos = -1;
		Control = gcnew Sikaloops::GUI::Editor(this);
	}

    virtual void WriteDC(String ^text)
	{
#ifdef _DEBUG
		const char* str2 = (char*)(void*)Marshal::StringToHGlobalAnsi(text);
		pCB->WriteLine(str2);
		Marshal::FreeHGlobal((System::IntPtr)(void*)str2);
#endif
	}

    virtual IWaveform ^GetWaveform(int index)
	{
		assert(index > 0);
		return gcnew VisualWaveform(pCB, index);
	}


    virtual void PlayWave(int track, int index)
	{
		MACHINE_LOCK;
		assert(index > 0);
		pEngines[track].PlayWave(index);
	}

    virtual void StopWave(int track, int index)
	{
		MACHINE_LOCK;
		assert(index > 0);
		pEngines[track].StopWave(index);
	}

    virtual int GetPlayPosition(int track, int index)
	{
		assert(index > 0);
		return pEngines[track].GetPlayPosition(index);
	}

	virtual void PlaySlice(int track, int sliceindex)
	{
		MACHINE_LOCK;
		pEngines[track].PlaySlice(pEditorPattern, track, sliceindex);
	}

	virtual float GetPlayPosition()
	{
		return PlayPos;
	}

    virtual bool GetPlayNotesState()
	{
		return pCB->GetPlayNotesState();
	}

    virtual void SelectWave(int index)
	{
		pCB->SelectWave(index);
	}
   

public:

	void WaveChanged(int index)
	{
		assert(index > 0);
		Control->WaveChanged(index);
	
	}

	void SetEditorPattern(CMachinePattern *p)
	{
		pEditorPattern = p;
		Control->Pattern = p != NULL ? p->MPattern : nullptr;
	}


public:
	HwndSource ^HS;
	Editor ^Control;
	Engine *pEngines;
	CMICallbacks *pCB;
	CMachinePattern *pEditorPattern;
	float PlayPos;

};


class GUI
{
public:
	gcroot<ManagedGUI ^> MGUI;
	HWND Window;
};

extern void CreateGUI(GUI &gui, HWND parent, Engine *peng);
