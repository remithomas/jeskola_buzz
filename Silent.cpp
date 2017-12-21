// silent driver

#define VC_EXTRALEAN

#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include <process.h>
#include <assert.h>
#include <math.h>
#include "../AudioDriver.h"

#define SAMPLERATE		44100
#define BUFFERSIZE		16384		// size of virtual dma buffer

class ad : public CAudioDriver
{
public:
	ad();
	virtual ~ad();
	virtual void Initialize(dword hwnd, float *(*pcallback)(int &numsamples));
	virtual void Reset();
	virtual bool Enable(bool e);	
	virtual int GetWritePos();
	virtual int GetPlayPos();
	virtual void Configure();

	void DoBlocks();

private:
	bool Start();	
	bool Stop();
	void ReadConfig();
	void WriteConfig();
	void Error(char const *msg);

	int GetTime();

public:
	double PerfFreq;
	double StartTime;
	float *(*pCallback)(int &numsamples);
	int WritePos;
	bool Initialized;
	bool Running;
	bool StopPolling;

};

static void __cdecl PollerThread(void *poo)
{
	ad *pad = (ad *)poo;

	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);

	pad->StopPolling = false;
	
	while(!pad->StopPolling)
	{
		pad->DoBlocks();
		Sleep(20);
	}

	pad->StopPolling = false;

	_endthread();
}

ad::ad()
{
	Initialized = false;
}


ad::~ad()
{
	Reset();
}

void ad::Error(char const *msg)
{
	MessageBox(NULL, msg, "WaveOut driver", MB_OK);
}

bool ad::Start()
{
	if (Running)
		return true;

	LARGE_INTEGER li;
	QueryPerformanceFrequency(&li);
	PerfFreq = (double)li.LowPart + ((double)li.HighPart * (65536.0*65536.0));

	QueryPerformanceCounter(&li);
	StartTime = (double)li.LowPart + ((double)li.HighPart * (65536.0*65536.0));

	WritePos = 0;

	HANDLE h = (HANDLE)_beginthread(PollerThread, 0, this);

	Running = true;
	return true;
}

bool ad::Stop()
{
	if (!Running)
		return true;

	StopPolling = true;
	while(StopPolling)
		Sleep(20);

	
	Running = false;
	return true;
}


void ad::DoBlocks()
{

	int pp = GetTime();
	int count = (dword)(pp - WritePos) & ((1 << 23) - 1);

	WritePos += count;

	while(count > 0)
	{
		int n = count;
		pCallback(n);
		count -= n;
	}

		
} 

void ad::ReadConfig()
{
	SamplesPerSec = SAMPLERATE;
	Flags = ADF_STEREO;
}

void ad::WriteConfig()
{
}


void ad::Initialize(dword hwnd, float *(*pcallback)(int &numsamples))
{
	pCallback = pcallback;
	Running = false;
	ReadConfig();

	Initialized = true;
	Start();
}

void ad::Reset()
{
	Stop();
}

void ad::Configure()
{
}

int ad::GetTime()
{
	LARGE_INTEGER pc;
	QueryPerformanceCounter(&pc);

	double c = (((double)pc.LowPart + ((double)pc.HighPart * (65536.0*65536.0)) - StartTime) / PerfFreq) * SamplesPerSec;

	return (int)fmod(c, (1 << 23));
}


int ad::GetPlayPos()
{
	if (!Running)
		return 0;
 
	return (dword)(GetTime() - BUFFERSIZE) & ((1 << 23) - 1);
}

int ad::GetWritePos()
{
	if (!Running)
		return 0;

	return WritePos & ((1 << 23) - 1);
}

bool ad::Enable(bool e)
{
	if (e)
		return Start();
	else
		return Stop();
}

CAudioDriverInfo info = { 
#ifdef _DEBUG
	"Silent (debug build)"
#else
	"Silent"
#endif
};

extern "C"
{
__declspec(dllexport) CAudioDriver * __cdecl NewAD()
{
	return new ad;
}

__declspec(dllexport) CAudioDriverInfo const * __cdecl GetADInfo()
{
	return &info;
}
}

